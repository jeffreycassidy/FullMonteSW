#include "GeompackFile.hpp"

#define BOOST_RESULT_OF_USE_DECLTYPE

#include <boost/iterator/function_input_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/archive/iterators/istream_iterator.hpp>
#include <boost/tuple/tuple.hpp>

#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>

using namespace std;

template<class T,size_t D>istream& operator>>(istream& is,array<T,D>& a)
{
	for(unsigned i=0;i<D;++i)
		is >> a[i];
	return is;
}

template<class T,size_t D>ostream& operator<<(ostream& os,const array<T,D>& a)
{
	for(unsigned i=0;i<D;++i)
	{
		os << a[i];
		if (i < D-1)
			os << ' ';
	}
	return os;
}

void GeompackMesh::load(string fn,bool verbose)
{
	ifstream is(fn.c_str());

	unsigned long Np,Np_info;

	is >> Np;

	// Load vertices

	P.resize(Np);
	P_info.resize(Np);

	for(unsigned long i=0; i<Np; ++i)
		is >> P[i] >> P_info[i];

	if (verbose)
		cout << "  Loaded " << Np << " vertices"<< endl;


	// Load information records

	is >> Np_info;

	info.resize(Np_info);

	for(unsigned long i=0;i<Np_info; ++i)
		is >> info[i].nodecode >> info[i].icsur >> info[i].ucsur >> info[i].vcsur;

	if (verbose)
			cout << "  Loaded " << Np_info << " information records"<< endl;

	// Load node elements

	unsigned long nodelem,nelem;
	is >> nodelem >> nelem;

	if (nodelem != 4)
		throw GeompackMesh::BadElementType(4,nodelem);

	tetinfo.resize(nelem);

	for(unsigned long i=0; i<nelem; ++i)
		is >> tetinfo[i].v;

	if (verbose)
		cout << "  Loaded " << nelem << " tetrahedral records"<< endl;

	for(unsigned long i=0; i<nelem; ++i)
		is >> tetinfo[i].regcode >> tetinfo[i].facinfo;

	if (verbose)
		cout << "  Loaded " << nelem << " tetrahedral info records"<< endl;


	// Load constrained edges

	unsigned long ncedg;
	is >> ncedg;

	ce.resize(ncedg);

	for(unsigned long i=0; i<ncedg; ++i)
		is >> ce[i].edgv0 >> ce[i].edgv1 >> ce[i].edginfo;

	if (verbose)
		cout << "  Loaded " << ncedg << " constrained edges"<< endl;

	if (!checkIntegrity(verbose))
		cout << "Integrity check failed" << endl;
	else
		cout << "Integrity check OK" << endl;
}

/** Checks integrity of the mesh.
 * Tests:
 *   Tetra vertex indices within range (0..Np)
 *
 * @param printStderr Prints errors to stderr if true (optional, default=false)
 * @return True if OK, else false
 *
 */

bool GeompackMesh::checkIntegrity(bool printStderr) const
{
	unsigned errct=0,maxErr=100;

	unsigned i=0;
	for(const Tetra& t : tetinfo)
	{
		unsigned v_min=-1,v_max=0;
		for(const unsigned& v : t.v)
		{
			v_min=min(v_min,v);
			v_max=max(v_max,v);
		}

		if (v_max > P.size())
		{
			errct++;
			if (printStderr && errct < maxErr)
			{
				cerr << "ERROR: Vertex index exceeds size of vertex array (" << P.size() << ')' << endl;
				cerr << "  tetra " << i << ": " << t.v << endl;
			}
		}
		++i;
	}

	return errct==0;
}

string GeompackMesh::Exception::message() const { return "Unknown GeompackMesh::Exception"; }

string GeompackMesh::BadElementType::message() const {
	stringstream ss;
	ss << "Unexpected geometry element type; expected " << expected << ", received " << received;
	return ss.str();
}
