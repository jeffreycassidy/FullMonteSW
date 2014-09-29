#include "DVH.hpp"
#include "graph.hpp"
#include "Sequence.hpp"
#include "Parallelepiped.hpp"
#include "fm-postgres/fm-postgres.hpp"
#include "io_timos.hpp"
#include <boost/range/adaptor/indexed.hpp>

#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

#include "fm-postgres/fm-postgres.hpp"
#include "fm-postgres/fmdbexportcase.hpp"

DVH makeDVH(const vector<double>& energy,const TetraMesh& M,const vector<Material>& mats,const vector<unsigned>& regions);

template<typename T>vector<T> loadTextVector(string fn)
{
	ifstream is(fn);
	vector<T> v;
	T t;
	for(is >> t; !is.eof() && !is.fail(); is >> t)
		v.push_back(t);
	return v;
}

namespace po=boost::program_options;

int main(int argc,char **argv)
{
	unsigned IDtarget=20;

	po::options_description cmdline;

	vector<string> targetfns;
	string meshfn;
	string prefix;
	string fluencefn;
	string materialsfn;
	string bboxstr;
	Sequence<unsigned> runIDs;

	cmdline.add_options()
			("target,t",po::value<vector<string>>(&targetfns),"Target file names and IDs (<ID#>:filename)")
			("materialfile,M",po::value<string>(&materialsfn),"Materials file")
			("meshfile,m",po::value<string>(&meshfn),"Mesh filename")
			("prefix,p",po::value<string>(&prefix),"File prefix to process")
			("fluence,f",po::value<string>(&fluencefn),"Fluence file to process")
			("bbox,b",po::value<string>(&bboxstr),"Bounding box (<dx0> <dy0> <dx0> <l0> <u0> <dx1> <dy1> <dz1> <l1> <u1> <l2> <u2>")
			("runs,r",po::value<Sequence<unsigned>>(&runIDs),"Run ID(s) to process from database")
			;

	boost::program_options::variables_map vm;

    cmdline.add(globalopts::db::dbopts);

    // parse options (if an option is already set, subsequent store() calls will not overwrite
    try {
        //po::store(po::command_line_parser(argc,argv).options(cmdline).positional(pos).run(),vm);
    	po::store(po::command_line_parser(argc,argv).options(cmdline).run(),vm);
        po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
        po::notify(vm);
    }
    catch (po::error& e){
        cerr << "Caught an exception in options processing" << endl;
        cout << cmdline << endl;
        return -1;
    }
	TetraMesh* M=NULL;
	boost::shared_ptr<PGConnection> conn=NULL;

	vector<Material> mats;
	vector<unsigned> runs_v;

	if(vm.count("meshfile") && vm.count("materialfile") && (vm.count("fluence") || vm.count("prefix")))
	{
		M = new TetraMesh(meshfn,TetraMesh::MatlabTP);
		if(vm.count("runs"))
		{
			cerr << "ERROR: Invalid arguments; can't specify runs from the database and files" << endl;
			return -1;
		}
		mats = readTIMOSMaterials(materialsfn);
	}
	else if(vm.count("runs"))
	{
		conn=PGConnect();
		runs_v = runIDs.as_vector();

		if (runs_v.size()==0)
		{
			cerr << "ERROR: No runs specified" << endl;
			return -1;
		}


		RunInfo info = getRunInfo(conn.get(),runs_v[0]);
		M = exportMesh(*conn.get(),info.IDmesh);
		mats = exportMaterials(*conn.get(),info.IDcase);
	}
	else
	{
		cerr << "ERROR: No run(s) specified for computation, or missing materials/mesh" << endl;
		return -1;
	}


	// handle target specification if provided
	vector<unsigned> regions = M->getMaterialMap();
	if (vm.count("target"))
	{
		for(const string& s : targetfns)
		{
			string targetFn;
			stringstream ss(s);
			ss >> IDtarget;
			ss.ignore(1,':');
			ss >> targetFn;
			cout << "Loading target definition for region " << IDtarget << " from file '" << targetFn << '\'' << endl;

			ifstream is(targetFn.c_str());
			vector<unsigned> target;
			std::copy(std::istream_iterator<unsigned>(is),std::istream_iterator<unsigned>(),std::back_inserter(target));

			for(unsigned IDt : target)
				regions[IDt] = IDtarget;

			cout << "  Loaded " << target.size() << " elements" << endl;
		}
	}

	if(vm.count("bbox"))
	{
		Parallelepiped dvhbox = readParallelepiped(bboxstr);

		cout << "DVH clipped to bounding parallelepiped: " << dvhbox << endl;

		// if tetra has no points inside the bounding box then set its region to zero
		// yes, it's <= because there's a zero element prepended so the vector size is +1
		double interiorV=0.0,clippedV=0.0;
		unsigned clippedN=0,interiorN=0;
		for(unsigned i=0;i<=M->getNt();++i)
		{
			bool inside=false;
			for(unsigned j=0;j<4;++j)
				inside |= dvhbox.pointWithin(M->getTetraPoint(i,j));

			if(!inside)
			{
				regions[i]=0;
				clippedV += M->getTetraVolume(i);
				++clippedN;
			}
			else
			{
				interiorV += M->getTetraVolume(i);
				++interiorN;
			}
		}
		cout << "Clipped volume: " << clippedV << " (" << clippedN << ") interior volume: " << interiorV << " (" << interiorN
				<< "); total " << clippedV+interiorV << " (" << clippedN+interiorN << ')' << endl;
	}

	stringstream ss;
	ss << "dvh.regions.out";
	ofstream os(ss.str().c_str());
	os << "% Region map for DVH calculation" << endl;
	os << "% Each line is <region ID> <vol>" << endl;
	os << regions.size() << endl;
	for(auto IDreg : regions | boost::adaptors::indexed(0))
		os << IDreg.value() << ' ' << M->getTetraVolume(IDreg.index()) << endl;
	os.close();

	if(vm.count("fluence"))				//
	{
		// load the fluence
		vector<double> energy = loadTextVector<double>(fluencefn);

		DVH dvh = makeDVH(energy,*M,mats,regions);

		dvh.writeASCII("dvh.out");
	}
	else if (vm.count("runs"))			// get it from database
	{
		for(unsigned IDr : runs_v)
		{
			// get the fluence vector
			vector<double> energy = exportResultVector(conn.get(),IDr,2);

			cout << "Run " << IDr << endl;
			DVH dvh = makeDVH(energy,*M,mats,regions);

			// write it out to a text file
			stringstream ss;
			ss << "dvh.run" << IDr << ".txt";

			dvh.writeASCII(ss.str());
		}
	}
}


DVH makeDVH(const vector<double>& energy,const TetraMesh& M,const vector<Material>& mats,const vector<unsigned>& regions)
{
	// get the energy result
	double sumE=0.0,sumV=0.0;
	for(double E : energy)
		sumE += E;
	cout << "  Total energy: " << sumE << ", total volume: " << sumV << " in " << energy.size() << " elements" << endl;

	sumE=sumV=0.0;
	unsigned i;
	for(i=0; i<regions.size();++i)
		if (regions[i] != 0)
		{
			sumE += energy[i];
			sumV += M.getTetraVolume(i);
		}
	cout << "  In nonzero regions: total energy " << sumE << ", total volume: " << sumV << " in " << i << " elements" << endl;

	vector<double> fluence = volumeEnergyToFluence(energy,M,mats);

	// do the DVH
	return DVH(M,fluence,regions);
}
