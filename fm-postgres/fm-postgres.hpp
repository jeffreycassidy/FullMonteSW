#ifndef FMPOSTGRES_INCLUDED
#define FMPOSTGRES_INCLUDED
#include <iostream>
#include <arpa/inet.h>
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/scoped_array.hpp>

#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>

#include <boost/shared_array.hpp>
#include <inttypes.h>

#include <map>
#include <sstream>
#include <string>

#include <tuple/tuple.hpp>

#include "../newgeom.hpp"

#include "blob.hpp"

using namespace std;

boost::program_options::options_description getDBOptions();


namespace globalopts {
    namespace db {
        const string& dbEnvironmentMap(const string& s);
        extern boost::program_options::options_description dbopts;
        extern string blobCachePath;

        extern string user,host,name,pass,port;

        void parseEnvironment(boost::program_options::variables_map&);
    };
};

extern const map<string,string> connprops_default;

template<class H,class T>unsigned   getPGVarLength(const boost::tuples::cons<H,T>& t);
template<class T>unsigned           getPGVarLength(const T&);
unsigned getPGVarLength(const char*);
template<>unsigned getPGVarLength(const string&);
//template<>unsigned getPGVarLength(const char* const&);


template<class T>unsigned getPGTupleLength(const T&);

template<>unsigned getPGTupleLength(const boost::tuples::null_type&);

template<class T>unsigned           getPGTupleLength(const T& t)
{
    return getPGVarLength(t.get_tail()) + getPGVarLength(t.get_head());
}

template<class H,class T>unsigned getPGVarLength(const boost::tuples::cons<H,T>& t)
    { return getPGVarLength(t.get_head()) + getPGVarLength(t.get_tail()); }

template<class T>struct pgTypeInfo {
//    static const Oid        type_oid;
//    static const unsigned   type_length;
//    static const unsigned   type_format;
};

template<>struct pgTypeInfo<unsigned long long> {
    static const Oid        type_oid=20;
    static const unsigned   type_length=8;
    static const unsigned   type_format=1;
    typedef uint64_t pg_representation;
    typedef union { uint64_t pg; unsigned long long native; } pg_representation_union;
};

template<>struct pgTypeInfo<double> {
    static const Oid        type_oid=701;
    static const unsigned   type_length=8;
    static const unsigned   type_format=1;
    typedef uint64_t pg_representation;
    typedef union { uint64_t pg; double native; } pg_representation_union;
};

template<>struct pgTypeInfo<unsigned> {
    static const Oid        type_oid=23;
    static const unsigned   type_length=4;
    static const unsigned   type_format=1;
    typedef uint32_t pg_representation;
    typedef union { pg_representation pg; unsigned native; } pg_representation_union;
};

template<>struct pgTypeInfo<char> {
    static const Oid        type_oid=18;
    static const unsigned   type_length=1;
    static const unsigned   type_format=1;
    typedef uint8_t pg_representation;
    typedef union { pg_representation pg; char native; } pg_representation_union;
};

template<>struct pgTypeInfo<boost::tuples::null_type> {
    static const Oid        type_oid=-1;
    static const unsigned   type_length=0;
    static const unsigned   type_format=1;
};

template<>struct pgTypeInfo<string> {
    static const Oid        type_oid=1043;
    static const unsigned   type_length=0;
    static const unsigned   type_format=0;
};

template<>struct pgTypeInfo<const char*>{
    static const Oid        type_oid=25;
    static const unsigned   type_length=0;
    static const unsigned   type_format=0;
};

template<>struct pgTypeInfo<int> {
    static const Oid        type_oid=23;
    static const unsigned   type_length=4;
    static const unsigned   type_format=1;
    typedef uint32_t pg_representation;
    typedef union { pg_representation pg; int native; } pg_representation_union;
};

template<int D>struct pgTypeInfo<Point<D,double> > {
    static const Oid        type_oid=1022;
    static const unsigned   type_length=(pgTypeInfo<double>::type_length+4)*D + 20;
    static const unsigned   type_format=1;
};

/*
template<int D,class T>struct pgTypeInfo<FixedArray<D,T> >{
    static const Oid        type_oid=pgTypeInfo<Point<D,T> >::type_oid;
    static const unsigned   type_length=pgTypeInfo<Point<D,T> >::type_length;
    static const unsigned   type_format=pgTypeInfo<Point<D,T> >::type_format;
};*/

template<class T>unsigned           getPGVarLength(const T&){ return pgTypeInfo<T>::type_length; }
template<int D,class T>unsigned     getPGVarLength(const FixedArray<D,T>& a){ return 20+D*(pgTypeInfo<T>::type_length+4); }
template<int D,class T>unsigned     getPGVarLength(const Point<D,T>& p){ return 20+D*(pgTypeInfo<T>::type_length+4); }

template<class T>typename pgTypeInfo<T>::pg_representation toNetworkOrder(T);

uint8_t toNetworkOrder(uint8_t);
uint8_t fromNetworkOrder(uint8_t);

uint16_t toNetworkOrder(uint16_t);
uint16_t fromNetworkOrder(uint16_t);

uint32_t toNetworkOrder(uint32_t b);
uint32_t fromNetworkOrder(uint32_t b);

uint64_t toNetworkOrder(uint64_t b);
uint64_t fromNetworkOrder(uint64_t b);

template<class T>typename pgTypeInfo<T>::pg_representation toNetworkOrder(T t)
{
    typename pgTypeInfo<T>::pg_representation_union u;
    u.native=t;
    return toNetworkOrder(u.pg);
}

template<class T>T fromNetworkOrder(const uint8_t* p)
{
    typename pgTypeInfo<T>::pg_representation_union u;
    u.pg = *reinterpret_cast<const typename pgTypeInfo<T>::pg_representation*>(p);
    u.pg = fromNetworkOrder(u.pg);
    return u.native;
}

template<class T>       void unpackPGVariable(const char*,T&);
template<int D,class T> void unpackPGVariable(const char*,FixedArray<D,T>&);
template<>void unpackPGVariable(const char* p,string& s);
template<>void unpackPGVariable(const char* p,boost::tuples::null_type&);

template<class T> void unpackPGVariable(const char* p,T& t)
{
    t=fromNetworkOrder<T>(reinterpret_cast<const uint8_t*>(p));
}

template<class T>void packPGVariable(const T&,uint8_t*);

template<>void packPGVariable(const char* const& s, uint8_t* p);
template<>void packPGVariable(const string& s,uint8_t* p);
template<int D,class T>void packPGVariable(const FixedArray<D,T>&,uint8_t* p);
template<>void packPGVariable(const FaceByPointID& d,uint8_t* p);
template<int D,class T>void packPGVariable(const Point<D,T>& d,uint8_t* p){ packPGVariable((const FixedArray<D,T>&)d,p); }

template<class T>void       packPGVariable(const T& t,uint8_t* p)
{
    *reinterpret_cast<typename pgTypeInfo<T>::pg_representation*>(p) = toNetworkOrder(t); 
}

template<int D,class T>void packPGVariable(const FixedArray<D,T>& a,uint8_t* p)
{
    *reinterpret_cast<uint32_t*>(p)    = toNetworkOrder(uint32_t(1));
    *reinterpret_cast<uint32_t*>(p+4)  = toNetworkOrder(uint32_t(0));
    *reinterpret_cast<uint32_t*>(p+8)  = toNetworkOrder(uint32_t(pgTypeInfo<T>::type_oid));
    *reinterpret_cast<uint32_t*>(p+12) = toNetworkOrder(uint32_t(3));
    *reinterpret_cast<uint32_t*>(p+16) = toNetworkOrder(uint32_t(1));

    p += 20;

    for(unsigned i=0;i<(unsigned)D; ++i)
    {
        *reinterpret_cast<uint32_t*>(p)   = toNetworkOrder<uint32_t>(pgTypeInfo<T>::type_length);
        *reinterpret_cast<uint64_t*>(p+4) = toNetworkOrder<T>(a[i]);
        p += pgTypeInfo<T>::type_length + 4;
    }
}

template<class T>unsigned getTypeOid(const T&){ return pgTypeInfo<T>::type_oid; }

template<class T>void getPGTupleOids(const T& t,Oid* oids)
{
    *oids = getTypeOid(t.get_head());
    getPGTupleOids(t.get_tail(),oids+1);
}
template<>void getPGTupleOids(const boost::tuples::null_type&,Oid*);

void packPGTuple(const boost::tuples::null_type& t,uint8_t* paramData,Oid* paramOids,uint8_t **paramValues,int* paramFormats,int* paramLengths);

template<class H,class T>void packPGTuple(const boost::tuples::cons<const H&,T>& t,uint8_t* paramData,Oid* paramOids,uint8_t** paramValues,int* paramFormats,int* paramLengths)
{
    const H& head=t.get_head();
    packPGVariable(head,paramData);
    *paramFormats=pgTypeInfo<H>::type_format;
    *paramLengths=pgTypeInfo<H>::type_length;
    *paramOids=pgTypeInfo<H>::type_oid;
    *paramValues=paramData;
    packPGTuple(t.get_tail(),paramData+getPGVarLength(head),paramOids+1,paramValues+1,paramFormats+1,paramLengths+1);
}

template<class H,class T>void packPGTuple(const boost::tuples::cons<H,T>& t,uint8_t* paramData,Oid* paramOids,uint8_t** paramValues,int* paramFormats,int* paramLengths)
{
    H head=t.get_head();
    packPGVariable(head,paramData);
    *paramFormats=pgTypeInfo<H>::type_format;
    *paramLengths=pgTypeInfo<H>::type_length;
    *paramOids=pgTypeInfo<H>::type_oid;
    *paramValues=paramData;
    packPGTuple(t.get_tail(),paramData+getPGVarLength(head),paramOids+1,paramValues+1,paramFormats+1,paramLengths+1);
}

/* Used this for debug testing only (unpacking a char* buf)
template<class T>void unpackPGRow(const uint8_t*,T&);
void unpackPGRow(const uint8_t*,boost::tuples::null_type);
template<class T>void unpackPGRow(const uint8_t* p,T& t)
{
    unpackPGVariable((const char*)p,t.get_head());
    unpackPGRow(p+pgTypeInfo<typename T::head_type>::type_length,t.get_tail());
}*/

class PGConnection {
    int server_ver;
    PGconn* conn;

    void connect(const string&);

    public:
    typedef boost::shared_ptr<PGresult> ResultType;

    class PGConnectionException {
        public: 
        string msg;
        PGConnectionException(string msg_) : msg(msg_){};
    };
    PGConnection(const string&);
    PGConnection(const map<string,string>&);
    ~PGConnection();

    enum PGResultFormat { ResultsBinary=1,ResultsText=0 };

    void prepare(const char* stmtName,const char* query,int nParams,const Oid* paramTypes);
    template<class T>void prepare(const char* stmtName,const char* query,const T& params);

    ResultType exec(const char* qrystr);
    ResultType execPrepared(const char* stmtName,int nParams,const char* const* paramValues,const int* paramLengths,const int* paramFormats,enum PGResultFormat=ResultsBinary);
    template<class T>ResultType execPrepared(const char* stmtName,const T& params);
    ResultType execParams(const char* cmd,int nParams,const Oid* paramTypes,const char*const* paramValues,const int* paramLengths,const int*paramFormats,enum PGResultFormat=ResultsBinary);

    template<class T>ResultType execParams(const char* cmd,const T& params); // uses boost::tuple

    Oid createLargeObject(const Blob& b);

    int getLargeObjectSize(Oid lobjid);

    Blob loadLargeObject(Oid lobjid);

    unsigned cleanLargeObjects();

};

template<class T>void unpackPGRow(const PGConnection::ResultType&,T,unsigned);
void unpackPGRow(const PGConnection::ResultType&,boost::tuples::null_type,unsigned,unsigned);

template<class T>void unpackSinglePGRow(const PGConnection::ResultType& res,T t)
{
    int n=PQntuples(res.get());
    if (n != 1)
    {
        stringstream ss; 
        ss << "Expecting exactly one row in result, received " << n << endl;
        throw ss.str();
    }
    unpackPGRow(res,t,0);
}


template<class T>void unpackPGRow(const PGConnection::ResultType& res,T t,unsigned r)
{
    unpackPGRow(res,t,r,0);
//    unpackPGVariable(PQgetvalue(res,r,c),t.get_head());
//    unpackPGRow(res,t.get_tail(),r,c+1);
}

template<class H,class T>void unpackPGRow(const PGConnection::ResultType& res,boost::tuples::cons<H&,T>& t,unsigned r,unsigned c)
{
    unpackPGVariable(PQgetvalue(res.get(),r,c),t.get_head());
    unpackPGRow(res,t.get_tail(),r,c+1);
}

template<>void unpackPGVariable(const char* p,FaceByPointID& d);
template<int D,class T>void unpackPGVariable(const char* p,Point<D,T>& d){ unpackPGVariable(p,(FixedArray<D,T>&)d); }

template<int D,class T>void unpackPGVariable(const char* p,FixedArray<D,T>& d){
    uint32_t    Ndim    = fromNetworkOrder(*(uint32_t*)p);
//    uint32_t    hasnull = fromNetworkOrder(*(uint32_t*)(p+4));
    Oid         typeoid = fromNetworkOrder(*(uint32_t*)(p+8));
    uint32_t    dim0    = fromNetworkOrder(*(uint32_t*)(p+12));
//    uint32_t    dim0lb  = fromNetworkOrder(*(uint32_t*)(p+16));

    if (Ndim != 1)
        cerr << "Invalid Ndim (" << Ndim << ", expecting 1) while unpacking FixedArray" << endl;

    if (typeoid != pgTypeInfo<T>::type_oid)
        cerr << "Invalid Oid (" << typeoid << ", expecting " << pgTypeInfo<T>::type_oid << ')' << endl;

    if (dim0 != D)
        cerr << "Invalid array dimension (" << dim0 << ", expecting " << D << ")" << endl;

    const uint8_t *x = (const uint8_t*)(p+24);

    for(unsigned i=0;i<dim0;++i)
    {
        d[i] = fromNetworkOrder<T>(x);
        x += pgTypeInfo<T>::type_length + 4;
    }
}


template<class T>void PGConnection::prepare(const char* stmtName,const char* query,const T& params)
{
    const unsigned nParam = boost::tuples::length<T>::value;
    Oid argOids[nParam];
    getPGTupleOids(params,argOids);
    PGresult* res = prepare(stmtName,query,nParam,argOids);
    PQclear(res);
}

template<class T>PGConnection::ResultType PGConnection::execPrepared(const char* stmtName,const T& params)
{
    boost::scoped_array<uint8_t> buf(new uint8_t[boost::tuples::length<T>::value*(2*sizeof(int) + sizeof(Oid) + sizeof(char*))+getPGTupleLength(params)]);

    int* paramFormats = (int*)buf.get();
    int* paramLengths = paramFormats + boost::tuples::length<T>::value;
    Oid* paramOids    = (Oid*)(paramLengths + boost::tuples::length<T>::value);
    uint8_t** paramValues = (uint8_t**)(paramOids + boost::tuples::length<T>::value);
    uint8_t*  paramData = (uint8_t*)(paramValues+boost::tuples::length<T>::value);

    packPGTuple(params,paramData,paramOids,paramValues,paramFormats,paramLengths);

    return execPrepared(stmtName,boost::tuples::length<T>::value,(const char* const*)paramValues,paramLengths,paramFormats,ResultsBinary);
}

template<class T>PGConnection::ResultType PGConnection::execParams(const char* cmd,const T& params)
{
    boost::scoped_array<uint8_t> buf(new uint8_t[boost::tuples::length<T>::value*(2*sizeof(int) + sizeof(Oid) + sizeof(char*))+getPGTupleLength(params)]);

    int* paramFormats = (int*)buf.get();
    int* paramLengths = paramFormats + boost::tuples::length<T>::value;
    Oid* paramOids    = (Oid*)(paramLengths + boost::tuples::length<T>::value);
    uint8_t** paramValues = (uint8_t**)(paramOids + boost::tuples::length<T>::value);
    uint8_t*  paramData = (uint8_t*)(paramValues+boost::tuples::length<T>::value);

    packPGTuple(params,paramData,paramOids,paramValues,paramFormats,paramLengths);

    return execParams(cmd,boost::tuples::length<T>::value,paramOids,(const char* const*)paramValues,paramLengths,paramFormats,ResultsBinary);
}

// connects using global variables for options
boost::shared_ptr<PGConnection> PGConnect();

class Source;
class Material;
class TetraMesh;

TetraMesh* exportMesh(PGConnection&,unsigned);
int exportSources(PGConnection&,unsigned,vector<Source*>&,long long Npacket=0);
int exportMaterials(PGConnection&,unsigned,vector<Material>&);
#endif
