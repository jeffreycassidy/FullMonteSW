#include "fm-postgres.hpp"
#include <string>
#include <iomanip>
#include <cstring>

#include <sys/stat.h>

using namespace std;

namespace globalopts {
    // default values for database connection
    namespace db {
        string user="postgres";
        string host="betzgrp-pcjeff.eecg.utoronto.ca";
        string name="FMDB";
        string pass="database";
        string port="5432";

        string blobCachePath="";

        bool verboseBlobCache=true;
        bool validateBlobCacheSum=true;
    };
};

namespace globalopts {
    namespace db {

pair<string,string> dbenv[] = {
    make_pair("FULLMONTE_DBHOST","dbhost"),
    make_pair("FULLMONTE_DBUSER","dbuser"),
    make_pair("FULLMONTE_DBPORT","dbport"),
    make_pair("FULLMONTE_DBNAME","dbname"),
    make_pair("FULLMONTE_DBBLOBCACHE","bcache")
};
const map<string,string> envmapper(dbenv,dbenv+5);

const string empty_str("");

const string& dbEnvironmentMap(const string& s)
{
    map<string,string>::const_iterator it=envmapper.find(s);
    if (it == envmapper.end())
        return empty_str;
    else
        return it->second;
}

boost::program_options::options_description getDBOptions()
{
    boost::program_options::options_description dbopts("Database options");
    dbopts.add_options()
        ("bcache,B",boost::program_options::value<string>(&globalopts::db::blobCachePath),"Blob cache directory")
        ("dbuser,U",boost::program_options::value<string>(&globalopts::db::user),"Database user name")
        ("dbhost,H",boost::program_options::value<string>(&globalopts::db::host),"Database host")
        ("dbname,D",boost::program_options::value<string>(&globalopts::db::name),"Database schema name")
        ("dbport,P",boost::program_options::value<string>(&globalopts::db::port),"Database connection port")
        ;
    return dbopts;
}

boost::program_options::options_description dbopts = getDBOptions();
    }
}

template<>unsigned getPGTupleLength(const boost::tuples::null_type&){ return 0; }

// deal with boost tuples
// bloody obscure bug!! when passing a temporary char* from boost::scoped_array::get(), can't return a char* const&
// result is it was calling the default getPGVarLength which was saying zero
//template<>unsigned getPGVarLength(const char* const& t)             { return strlen(t)+1; }
unsigned getPGVarLength(const char* t)                              { return strlen(t)+1; }
template<>unsigned getPGVarLength(const string& t)                  { return t.length()+1; }

uint8_t fromNetworkOrder(uint8_t b){ return b; }
uint8_t toNetworkOrder(uint8_t b){ return b; }

uint16_t fromNetworkOrder(uint16_t b){ return ntohs(b); }
uint16_t toNetworkOrder(uint16_t b){ return htons(b); }

uint32_t toNetworkOrder(uint32_t b)     { return htonl(b); }
uint32_t fromNetworkOrder(uint32_t b)   { return ntohl(b); }

uint64_t toNetworkOrder(uint64_t b)     { return htonl(b >> 32) | ((uint64_t)htonl(b & 0xFFFFFFFF) << 32); }
uint64_t fromNetworkOrder(uint64_t b)   { return ntohl((uint64_t)(b >> 32)) | ((uint64_t)ntohl(b&0xFFFFFFFF) << 32);   }
uint64_t fromNetworkOrder_blah(uint64_t b)   { return ntohl((uint64_t)(b >> 32)) | ((uint64_t)ntohl(b&0xFFFFFFFF) << 32);   }

// not sure why this works
// 1-D array format is:

//  Name        Size        Offset  Description
//  ----        ----        ------  -----------
//  Ndim        4           0       Number of dimensions
//  hasnull     4           4       Has null (=0)
//  typeoid     4           8       Oid of element type
//  dim0        4           12      Number of elements

//              typlen+2    24                  First element   -- INFERRED FROM DOUBLE ARRAY; NOT SURE IF IT GENERALIZES
//              typlen+2    24+(typlen+2)*i     Element i

unsigned from_hex_digit(char i)
{
	if (i >= '0' && i <= '9')
		return i-'0';
	else if (i >= 'A' && i <= 'F')
		return i-'A'+10;
	else if (i >= 'a' && i <= 'a')
		return i-'a'+10;
	else
		cerr << "Invalid character '" << i << "' (" << (unsigned) i << ") in from_hex_digit" << endl;
	return 0;
}

template<>void unpackPGVariable(const char* p,string& s){ s.assign(p); }
template<>void unpackPGVariable(const char* p,boost::tuples::null_type&){}
template<>void unpackPGVariable(const char* p,FaceByPointID& d){ unpackPGVariable(p,(array<unsigned,3>&)d); }
template<>void unpackPGVariable(const char* p,Point<3,double>& P){ unpackPGVariable(p,(array<double,3>&)P); }
template<>void unpackPGVariable(const char* p,SHA1_160_SUM& s){
	string::iterator it=s.begin();
	for(unsigned i=0;i<40;i+=2)
		*(it++) = (from_hex_digit(p[i])<<4) + from_hex_digit(p[i+1]);
}

void unpackPGRow(const PGConnection::ResultType&,boost::tuples::null_type,unsigned,unsigned){}
void unpackPGRow(const uint8_t* p,boost::tuples::null_type){}

template<>void packPGVariable(const char* const& s, uint8_t* p){ copy(s,s+strlen(s)+1,p); }
template<>void packPGVariable(const string& s,uint8_t* p){ copy(s.begin(),s.end()+1,p); }
template<>void packPGVariable(const SHA1_160_SUM& s,uint8_t* p){ string h=s.as_hex(); copy(h.begin(),h.end(),p); p[40]='\0'; }
template<>void packPGVariable(const FaceByPointID& d,uint8_t* p){ packPGVariable((const array<unsigned,3>&)d,p); }

PGConnection::PGConnection(const string& connstr) : conn(NULL)
{
    connect(connstr);
}

void PGConnection::connect(const string& connstr)
{
    if (conn)
        PQfinish(conn);

    conn = PQconnectdb(connstr.c_str());

    if(!conn)
        throw PGConnectionException("Serious failure - return from PQconnectDB is null");

    ConnStatusType connstatus=PQstatus(conn);

    switch(connstatus){
        case CONNECTION_OK:
        cout << "Connected OK at " << endl;
        break;
        default:
        throw PGConnectionException("Failed to connect with connection string \"" + connstr + "\"");
        break;
    }
    server_ver = PQserverVersion(conn);
}

PGConnection::PGConnection(const map<string,string>& props) : server_ver(0),conn(NULL)
{
    string connstr;

    for(map<string,string>::const_iterator it=props.begin(); it != props.end(); ++it)
        connstr += it->first + " = " + it->second + " ";

    connect(connstr);
}

PGConnection::~PGConnection()
{
    if (conn)
        PQfinish(conn);
}

Oid PGConnection::createLargeObject(const string& s)
{
    const char* p=s.data();
    unsigned Nb=s.size();

    Oid lobjid;
    int lobj_fd;

    if (!conn)
        throw PGConnectionException("Database not open at ");


    // begin transaction
    PQexec(conn,"BEGIN");

    // create large object & check
    lobjid = lo_creat(conn, INV_READ | INV_WRITE);
    if (lobjid <= 0)
    {
        cerr << "Failed to create large object" << endl;
        return -1;
    }

    // open fd for large object & check
    lobj_fd = lo_open(conn,lobjid,INV_WRITE | INV_READ);
    if (lobj_fd == -1)
    {
        cerr << "Failed to open large object" << endl;
        return -1;
    }

    // write
    int Nb_written = lo_write(conn,lobj_fd,(const char*)p,Nb);

    if (Nb_written < 0)
        cerr << "Error writing - Nb<0" << endl;
    else if ((unsigned)Nb_written != Nb){
        cerr << "Error while writing" << endl;
        return -1;
    }

    lo_close(conn,lobj_fd);
    string sum = SHA1_160_SUM(s).as_hex();
    cout << "Writing checksum (size " << sum.size() << "): " << sum << endl;
    execParams("INSERT INTO blobsums(blobid,sha1_160) VALUES ($1,$2);",boost::tuples::make_tuple(lobjid,sum));

    PQexec(conn,"COMMIT");

    return lobjid;
}

int PGConnection::getLargeObjectSize(Oid lobjid)
{
    if(!conn)
        throw PGConnectionException("Database is not open at ");// ##__FILE__ ":" ##__LINE__ );

    PQexec(conn,"BEGIN");

    // open and check
    int lobj_fd = lo_open(conn,lobjid,INV_READ);
    if (lobj_fd == -1)
    {
        cerr << "Failed to open large object " << lobjid << " for reading" << endl;
        PQexec(conn,"end");
        return -1;
    }

    // get file size and reserve storage
    lo_lseek(conn,lobj_fd,0,SEEK_END);
    int Nb_total = lo_tell(conn,lobj_fd);
    lo_lseek(conn,lobj_fd,0,SEEK_SET);
    PQexec(conn,"COMMIT");

    return Nb_total;
}

string PGConnection::loadLargeObject(Oid lobjid)
{
    int lobj_fd; 
    stringstream fn;

    if(!conn)
        throw PGConnectionException("Database is not open at ");// ##__FILE__ ":" ##__LINE__ );

    if (globalopts::db::blobCachePath != "")    // if caching is enabled
    {
        struct stat buf;
        fn << globalopts::db::blobCachePath << '/' << lobjid << ".bin";
        if(!stat(fn.str().c_str(),&buf)){            // file exists
        	string b_file = readBinary(fn.str());

            if (!globalopts::db::validateBlobCacheSum)  // just return data if not checking checksums
            {
                if (globalopts::db::verboseBlobCache)
                    cout << "Fetching large object ID=" << lobjid << " from cache, no checksum validation" << endl;
                return b_file;
            }

            SHA1_160_SUM dbsum;
            SHA1_160_SUM filesum(b_file);
            PGConnection::ResultType res = execParams("SELECT sha1_160 FROM blobsums WHERE blobid=$1;",boost::tuples::make_tuple(lobjid));

            if (PQntuples(res.get()) == 0){ // couldn't find a checksum
                if (globalopts::db::verboseBlobCache)
                    cout << "No checksum found for large object ID=" << lobjid << endl;
            }
            else
            {
                unpackSinglePGRow(res,boost::tuples::tie(dbsum));
                if(dbsum==filesum)
                {
                    if(globalopts::db::verboseBlobCache)
                        cout << "Loaded large object ID=" << lobjid << " from cache, checksum OK" << endl;
                    return b_file;
                }
            }
        }
    }

    // we only get here if checksum is invalid or caching is disabled

    // begin transaction
    PQexec(conn,"BEGIN");

    // open and check
    lobj_fd = lo_open(conn,lobjid,INV_READ);
    if (lobj_fd == -1)
    {
        cerr << "Failed to open large object " << lobjid << " for reading" << endl;
        PQexec(conn,"end");
        return string();
    }

    // get file size and reserve storage
    lo_lseek(conn,lobj_fd,0,SEEK_END);
    int Nb_total = lo_tell(conn,lobj_fd);
    lo_lseek(conn,lobj_fd,0,SEEK_SET);

    string s(Nb_total,'\0');

    // do the read & check for errors
    int Nb_read=0,c;
    char *q=(char*)s.data();			// cast away constness for reading

    for(Nb_read=0; Nb_read < Nb_total; Nb_read += c,q += c)
    {
        c = lo_read(conn,lobj_fd,q,min(Nb_total-Nb_read,4096));
        if (c < 0 || c != min(Nb_total-Nb_read,4096))
            cerr << "Read failed" << endl;
#ifndef POSIX_TIMER
        if (Nb_read % (10*4096) == 0)
            cout << "\rRead " << setw(8) << Nb_read << " of " << setw(8) << Nb_total << " bytes (" << fixed << setprecision(2) << setw(5) << (double)Nb_read/(double)(Nb_total)*100.0 << "%)" << flush;
#endif
    }
    cout << "\rRead " << setw(8) << Nb_read << " of " << setw(8) << Nb_total << " bytes (100.00%)" << endl << flush;

    PQexec(conn,"COMMIT");

    if (globalopts::db::blobCachePath != "")
    {
        if(globalopts::db::verboseBlobCache)
            cout << "Overwriting database checksum value and cache file for large object ID=" << lobjid << endl;
        execParams("DELETE FROM blobsums WHERE blobid=$1;",boost::tuples::make_tuple(lobjid));
        execParams("INSERT INTO blobsums(blobid,sha1_160) VALUES ($1,$2);",boost::tuples::make_tuple(lobjid,SHA1_160_SUM(s)));
        writeBinary(fn.str(),s);
    }

    return s;
}

void PGConnection::prepare(const char* stmtName,const char* query,int nParams,const Oid* paramTypes)
{
    PGresult* res;

    if(!conn)
        throw PGConnectionException("Not connected to database");
    
    res = PQprepare(conn,stmtName,query,nParams,paramTypes);

    if (!res)
        throw PGConnectionException("Serious failure, PQprepare returned NULL");

    ExecStatusType status = PQresultStatus(res);

    stringstream errmsg;
    switch(status){
        case PGRES_COMMAND_OK:
        break;
        default:
        errmsg << "Prepare statement failed; status " << PQresultErrorMessage(res) << endl;
        throw PGConnectionException(errmsg.str());// __FILE__ ":" __LINE__);
        break;
    }
    PQclear(res);
}

PGConnection::ResultType PGConnection::execPrepared(const char* stmtName,int nParams,const char* const* paramValues,const int* paramLengths,const int* paramFormats,enum PGResultFormat resultFormat)
{
    PGresult* res;
    ExecStatusType status;

    if(!conn)
        throw PGConnectionException("Error: DB not connected");

    res = PQexecPrepared(conn,stmtName,nParams,paramValues,paramLengths,paramFormats,(int)resultFormat);

    stringstream errmsg;

    if (res==NULL)
        throw PGConnectionException("Serious failure at __FILE__:__LINE__");
    else {
        status=PQresultStatus(res);
        if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        {
            errmsg << "Failed to execute prepared statement; message: " << PQresultErrorMessage(res) << endl;
            errmsg << "  Statement name: " << stmtName << endl;
            throw PGConnectionException(errmsg.str());//__FILE__ ":" __LINE__);
        }
    }

    return ResultType(res,PQclear);
}

PGConnection::ResultType PGConnection::execParams(const char* cmd,int nParams,const Oid* paramTypes,const char*const* paramValues,const int* paramLengths,const int*paramFormats,enum PGResultFormat resultFormat)
{
    PGresult* res;
    ExecStatusType status;

    if(!conn)
        throw PGConnectionException("Error: DB not connected");

    res = PQexecParams(conn,cmd,nParams,paramTypes,paramValues,paramLengths,paramFormats,(int)resultFormat);

    stringstream errmsg;

    if (res==NULL)
        throw PGConnectionException("Serious failure at __FILE__:__LINE__");
    else {
        status=PQresultStatus(res);
        if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        {
            errmsg << "Failed to execute prepared statement; message: " << PQresultErrorMessage(res) << endl;
            errmsg << "  Query string: '" << cmd << '\'' << endl;
            throw PGConnectionException(errmsg.str());
        }
    }
    return ResultType(res,PQclear);
}


PGConnection::ResultType PGConnection::exec(const char* qrystr)
{
    PGresult* res;
    ExecStatusType status;

    if(!conn)
        throw PGConnectionException("Error: DB not connected");

    res = PQexec(conn,qrystr);

    stringstream errmsg;

    if (res==NULL)
        throw PGConnectionException("Serious failure at __FILE__:__LINE__");
    else {
        status=PQresultStatus(res);
        if(status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK)
        {
            errmsg << "Failed to execute prepared statement; message: " << PQresultErrorMessage(res) << endl;
            throw PGConnectionException(errmsg.str());
        }
    }

    return ResultType(res,PQclear);
}

unsigned PGConnection::cleanLargeObjects()
{
    if (!conn)
        throw PGConnectionException("Not connected");

    PGresult* res = PQexec(conn,"SELECT oid FROM large_objects_unused");

    if (!res)
        throw PGConnectionException("Result returned is null");

    ExecStatusType status = PQresultStatus(res);

    if (status != PGRES_TUPLES_OK)
        throw PGConnectionException("Failed to return results");

    unsigned N = PQntuples(res);
    Oid lobjid;

    for(unsigned i=0;i<N;++i)
    {
        lobjid = atol(PQgetvalue(res,i,0));
        cout << "INFO: Deleting unused large object Oid=" << lobjid << endl;
        lo_unlink(conn,lobjid);
    }
    PQclear(res);

    return N;
}

template<>void getPGTupleOids(const boost::tuples::null_type&,Oid*){}

void packPGTuple(const boost::tuples::null_type& t,uint8_t* paramData,Oid* paramOids,uint8_t **paramValues,int* paramFormats,int* paramLengths)
{
}

boost::shared_ptr<PGConnection> PGConnect()
{
    stringstream connstr;

    connstr << "host = " << globalopts::db::host
        << " user = " << globalopts::db::user
        << " port = " << globalopts::db::port
        << " dbname = " << globalopts::db::name;

    return boost::shared_ptr<PGConnection>(new PGConnection(connstr.str()));
}
