/*
 * MatlabWriter.cpp
 *
 *  Created on: Jul 16, 2015
 *      Author: jcassidy
 */

#include "MatlabText.hpp"

#include <boost/range/counting_range.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <tuple>

#include "take_drop.hpp"

using namespace std;

MatlabWriter::MatlabWriter(){}

/** Selects the subset of faces which are exported
 *
 */

void MatlabWriter::setFaceSubset(const std::vector<unsigned>& s)
{
	assert(M_ || !"MatlabWriter needs a mesh set before creating a subset");

	// face permutation
	facePerm_ = Permutation<unsigned>::buildFromForward(M_->getNf()+1,s);

	// point permutation
	std::vector<unsigned> q(M_->getNp()+1,0);

	// mark points which have a reference
	for(const auto IDps : s | boost::adaptors::transformed([this](unsigned i){ return M_->getFacePointIDs(i); }))
		for(const unsigned IDp : IDps)
			q.at(IDp) = 1;

	pointPerm_ = Permutation<unsigned>::buildStableFromBoolRange(q);
}

/** Clears the face subset vectors so all faces are exported (generally not advised since it's much larger)
 *
 */

void MatlabWriter::setFacesToAll()
{
	pointPerm_.clear();
	facePerm_.clear();
}

void MatlabWriter::setComment(const std::string comm)
{
	comm_ = comm;
}

void MatlabWriter::writeFaces(const std::string fn) const
{
	ofstream os(fn.c_str());

	os << "% Surface mesh; point and face indices start at 1" << endl;

	if (pointPerm_.empty())
		os << "% Constructed using the original point numbering (may contain a large number of unused points)" << endl;
	else
		os << "% Constructed from a subset of the original mesh, with points renumbered" << endl;

	writeComments_(os);

	os << "% format below is <Np> <Nd=3> { <IDp_original>? <x> <y> <z> }Np <Nf> <Ns=3> { <IDf_original>? <p0> <p1> <p2> }Nf" << endl;
	os << "% Np_original/Nf_original refer to the original indices of the point/face (given only if remapping enabled & output requested)" << endl;

	// write points
	os << (pointPerm_.empty() ? M_->getNp() : pointPerm_.forward().size()) << " 3"  << endl;

	os << fixed << setprecision(6);

	if (pointPerm_.empty())
		for(const auto& p : M_->points() | drop(1))
		{
			for(unsigned j=0;j<3;++j)						// Drop dummy point 0; Matlab starts index at 1 so will line up
				os << setw(10) << p[j] << ' ';
			os << endl;
		}
	else
		for(const auto i : pointPerm_.forward() | boost::adaptors::indexed(0U))
		{
			assert(i.value() != -1U);
			if (outputOriginalIndices_)
				os << setw(6) << i.value() << ' ';
			for(unsigned j=0;j<3;++j)
				os << setw(10) << M_->getPoint(i.value())[j] << ' ';
			os << endl;
		}



	if (facePerm_.empty())
	{
		os << M_->getNf() << " 3" << endl;
		for(unsigned i=1;i<=M_->getNf();++i)					// Again drop dummy face 0 to align with Matlab numbering
		{
			for(unsigned j=0;j<3;++j)
				os << setw(5) << M_->getFacePointIDs(i)[j] << ' ';
			os << endl;
		}
	}
	else
	{
		os << facePerm_.forward().size() << " 3" << endl;
		for(const auto i : facePerm_.forward())
		{
			if (outputOriginalIndices_)
				os << setw(6) << i << ' ';
			std::array<unsigned,3> y = remap(M_->getFacePointIDs(i),pointPerm_.inverse());
			for(unsigned j=0;j<3;++j)
				os << setw(5) << y[j]+1 << ' ';
			os << endl;
		}
	}
}

void MatlabWriter::writeComments_(std::ostream& os) const
{
	if (comm_.size()>0)
	{
		os << "% ---------- USER COMMENTS" << endl;
		stringstream ss(comm_);
		string l;
		while(!ss.eof())
		{
			getline(ss,l);
			os << "%     " << l << endl;;
		}
		os << "% ----------" << endl;
	}
}


namespace detail {

/** Helper class for formatted output of (index,fluence) or just fluence to ostream.
 *
 */

class FluenceWriter {
public:
	FluenceWriter(ostream& os) : os_(os){}

	// convenience methods for dealing with (index,value) pairs -> print both index and value
	void operator()(const boost::range::index_value<const double&,std::ptrdiff_t> p){ operator()(p.index(),p.value()); };
	void operator()(const boost::range::index_value<double,std::ptrdiff_t> p){ operator()(p.index(),p.value()); };
	void operator()(const std::pair<unsigned,double> p) const { operator()(p.first,p.second); }

	// just a double argument: write only the fluence
	void operator()(double phi) const { os_ << setw(phiW_) << setprecision(phiP_) << phi << endl; }

	// a pair: write index and fluence
	void operator()(unsigned i,double phi) const { os_ << setw(idxW_) << i << ' ' << setprecision(phiP_) << setw(phiW_) << phi << endl; }

	ostream& os_;
	unsigned idxW_=6,phiW_=8,phiP_=6;
};

};



/**
 * Writes either all faces or a subset (depending on what subset is active in the writer) to a text file suitable for Matlab import.
 * File comments occur on lines starting with '%'
 * File format is:
 *
 *
 * % Comments....
 * <Nf> <Nfsub> <Nnz>
 * { <idx>? <phi> } Nnz
 *
 *
 * ===== Description =====
 *
 * Nf: number of faces in mesh
 * Nfsub: number of faces in subset
 * 		 Nfsub == Nf => no subset, idx refers to mesh face
 * 		 Nfsub <  Nf => subset, idx refers to index within subset
 * Nnz: number of nonzero elements included
 * 		Nnz == Nfsub => dense representation, idx is omitted
 * 		Nnz <  Nfsub => sparse representation, idx defined as above
 *
 *	===== Arguments =====
 *
 * fn			The filename
 * phi			The fluence vector
 */

void MatlabWriter::writeSurfaceFluence(const std::string fn,const std::vector<double>& phi) const
{
	ofstream os(fn.c_str());

	bool sparse = !denseOutput();
	bool remapped = !facePerm_.empty();

	unsigned Nf=0,Nfsub=0,Nnz=0;

	// determine number of output values
	assert(M_ || !"MatlabWriter needs a mesh set before writing");
	Nf = M_->getNf();

	// +1 because of dummy element at index 0
	assert(Nf+1 == phi.size() && "Number of fluence elements does not match number of faces");

	if (remapped)
	{
		Nfsub = facePerm_.forward().size();
		if (sparse)
			for(unsigned i=0;i<facePerm_.forward().size();++i)
				Nnz += phi[facePerm_.forward()[i]] > phiMin_;
		else
			Nnz = Nfsub;
	}
	else
	{
		Nfsub = Nf;
		if (sparse)
			for(unsigned i=1;i<=Nf; ++i)
				Nnz += phi[i];
		else
			Nnz=Nf;
	}

	if (sparse)
		os << "% Sparse surface fluence with face indices starting at 1, threshold is phi > " << phiMin_ << endl;
	else
		os << "% Dense surface fluence starting at face index 1" << endl;

	if (remapped)
		os << "% Constructed from a subset of the original mesh" << endl;
	else
		os << "% Constructed using the original face numbering (may contain a large number of unused faces)" << endl;

	writeComments_(os);

	os << "% format below is <Nf> <Nsub> <Nnz> followed by either: " << endl;
	os << "%    { <IDf> <phi> } if Nnz < Nsub (sparse representation)" << endl;
	os << "%    { <phi> } else (dense)" << endl;

	os << Nf << ' ' << Nfsub << ' ' << Nnz << endl;

	os << fixed;

	detail::FluenceWriter w(os);

	if (remapped)
	{
		auto phiR = facePerm_.forward() | boost::adaptors::transformed([&phi](unsigned j){ return phi[j]; });
		if (!sparse)							// Fluence for faces phi[faceP_[j]] for j in [1,Nfsub]
			boost::for_each(phiR, w);
		else									// Fluence (j,phi[faceP_[j]]) for j in [1,Nfsub]
			boost::for_each(phiR | boost::adaptors::indexed(1U)
					| boost::adaptors::filtered([this](const boost::range::index_value<double,std::ptrdiff_t> p){ return p.value()>phiMin_; }),
				w);
	}
	else
	{
		if (!sparse)							// Fluence for faces phi[i] for i in [1..Nf]
			boost::for_each(phi | drop(1), w);
		else									// sparse output: index + fluence for all elements of subset exceeding threshold
			boost::for_each(phi
				| drop(1)
				| boost::adaptors::indexed(1U)
				| boost::adaptors::filtered([this](const boost::range::index_value<const double&,std::ptrdiff_t> p){ return p.value() > phiMin_; }),
					w);

	}
}
