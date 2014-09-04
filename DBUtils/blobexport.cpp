#include "blob.hpp"
#include "fm-postgres/fm-postgres.hpp"

#include <boost/program_options.hpp>

int main(int argc,char **argv)
{
    unsigned blobid;
    string ofn;
    boost::program_options::options_description cmdline;
    boost::program_options::variables_map vm;

    cmdline.add_options()
        ("blobid,b",boost::program_options::value<unsigned>(&blobid),"Blob ID to extract")
        ("output,o",boost::program_options::value<string>(&ofn),"Output file name")
        ;

    cmdline.add(globalopts::db::dbopts);

    boost::program_options::store(parse_command_line(argc,argv,cmdline),vm);
    boost::program_options::store(parse_environment(cmdline,globalopts::db::dbEnvironmentMap),vm);
    boost::program_options::notify(vm);

    boost::shared_ptr<PGConnection> conn = PGConnect();

    string s = conn.get()->loadLargeObject(atoi(argv[1]));
    writeBinary(argv[2],s);
}
