#include "fm-postgres/fm-postgres.hpp"
#include "blob.hpp"
#include <boost/smart_ptr/shared_ptr.hpp>

#include <sys/stat.h>

vector<unsigned> getBlobList(PGConnection*);

int main(int argc,char **argv)
{
    bool fix=argc >= 2;
    boost::shared_ptr<PGConnection> conn = PGConnect();

    vector<unsigned> blobList=getBlobList(conn.get());
    cout << "Found " << blobList.size() << " large objects in the database" << endl;

    Oid lobjid;
    string dbsum;

    map<unsigned,string> m;
    boost::shared_ptr<PGresult> res(conn->execParams("SELECT blobid,sha1_160 FROM blobsums;",boost::tuples::null_type()),PQclear);
    cout << "Found " << PQntuples(res.get()) << " sums in the checksum table" << endl;

    for(int i=0;i<PQntuples(res.get()); ++i)
    {
        unpackPGRow(res.get(),boost::tuples::tie(lobjid,dbsum),i);
        m.insert(make_pair(lobjid,dbsum));
        cout << "  " << setw(8) << lobjid << ' ' << setw(7) << conn->getLargeObjectSize(lobjid) << " bytes: " << dbsum << endl;
    }

    cout << endl << endl << "Checking large objects: " << endl;

    struct stat buf;

    bool sum_exists=false,file_exists=false;

    conn.get()->prepare("insertsum","INSERT INTO blobsums(blobid,sha1_160) VALUES($1,$2);",boost::tuples::make_tuple(Oid(1),string()));

    for(vector<unsigned>::const_iterator it=blobList.begin(); it != blobList.end(); ++it)
    {
        stringstream fn;
        fn << "/Users/jcassidy/FullMonte/blobcache/" << *it << ".bin";

        uint8_t filesum[21];
        uint8_t dbsum[21];
        string filesum_string;

        Blob bfile,bdb;

        file_exists = !stat(fn.str().c_str(),&buf);

        map<unsigned,string>::const_iterator mIt=m.find(*it);
        sum_exists = mIt != m.end();

        cout << *it << "  ";

        if (file_exists)        // if file exists, do checksum
        {
            cout << "File exists, ";
            bfile = Blob(fn.str());
            bfile.sha1_160(filesum);
            filesum_string=bfile.sha1_160();
        }
        else
            cout << "No file, ";

        cout << (sum_exists ? "checksum exists" : "no checksum") << ' ';

        if (sum_exists && file_exists && equal(filesum_string.begin(),filesum_string.end(),mIt->second.begin()))
            cout << "- Matches OK" << endl;
        else {
            if (sum_exists && file_exists)
                cout << "- Mismatch";

            if (fix){
                cout << "- Downloading data" << endl;
    
                bdb = conn.get()->loadLargeObject(*it);
                bdb.sha1_160(dbsum);
                dbsum[20]=0;
    
                cout << endl;
    
                // if the file isn't there, write it in
                if (!file_exists)
                {
                    cout << "  Writing file" << endl;
                    bdb.writeFile(fn.str());
                }
                else if(equal(dbsum,dbsum+20,filesum))  // file exists, database must not - check for equality
                    cout << "  File sum agrees with database" << endl;
    
                // if the checksum isn't in the DB, write it there
                if (!sum_exists)
                {
                    cout << "  Adding to database" << endl;
                    try {
                        conn.get()->execPrepared("insertsum",boost::tuples::make_tuple(Oid(*it),bdb.sha1_160()));
                    }
                    catch(PGConnection::PGConnectionException& e)
                    {
                        cerr << "Failed to insert sum, error: " << e.msg << endl;
                    }
                }
                else {
                    cout << "  Updating checksum in database" << endl;
                    conn.get()->execParams("UPDATE blobsums SET sha1_160=$2 WHERE blobid=$1;",boost::tuples::make_tuple(Oid(*it),bdb.sha1_160()));
                }
            }
            cout << endl;
        }
    }
}

vector<unsigned> getBlobList(PGConnection* conn)
{
    Oid lobjid;
    vector<unsigned> v;

    // query for large objects
    boost::shared_ptr<PGresult> res(conn->execParams("SELECT oid FROM pg_largeobject_metadata",boost::tuples::null_type()),PQclear);

    int Nr=PQntuples(res.get());

    if (Nr < 0)
        return v;

    v.reserve(PQntuples(res.get()));

    for(unsigned i=0;i<PQntuples(res.get()); ++i)
    {
        unpackPGRow(res.get(),boost::tuples::tie(lobjid),i);
        v.push_back(lobjid);
    }

    return v;
}
