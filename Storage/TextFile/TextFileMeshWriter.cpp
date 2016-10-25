/*
 * TextFileMeshWriter.cpp
 *
 *  Created on: Oct 7, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <tuple>

#include "TextFileMeshWriter.hpp"

#include <iomanip>

#include <fstream>

using namespace std;

TextFileMeshWriter::TextFileMeshWriter()
{
}

TextFileMeshWriter::~TextFileMeshWriter()
{
}

void TextFileMeshWriter::faceFilter(const FilterBase<int>* F)
{
	m_faceFilter=F;
}

const FilterBase<int>* TextFileMeshWriter::faceFilter() const
{
	return m_faceFilter;
}

//
///** Selects the subset of faces which are exported
// *
// */
//
//void MatlabWriter::faceSubset(const std::vector<unsigned>& s)
//{
//	assert(M_ || !"MatlabWriter needs a mesh set before creating a subset");
//
//	// face permutation
//	facePerm_ = Permutation<unsigned>::buildFromForward(M_->getNf()+1,s);
//
//	// point permutation
//	std::vector<unsigned> q(M_->getNp()+1,0);
//
//	// mark points which have a reference
//	for(const auto IDps : s | boost::adaptors::transformed([this](unsigned i){ return M_->getFacePointIDs(i); }))
//		for(const unsigned IDp : IDps)
//			q.at(IDp) = 1;
//
//	pointPerm_ = Permutation<unsigned>::buildStableFromBoolRange(q);
//}
//
///** Clears the face subset vectors so all faces are exported (generally not advised since it's much larger)
// *
// */
//
//void MatlabWriter::setFacesToAll()
//{
//	pointPerm_.clear();
//	facePerm_.clear();
//}
//
//void MatlabWriter::comment(const std::string comm)
//{
//	comm_ = comm;
//}

void TextFileMeshWriter::writePoints(std::ostream& os) const
{
	if (!mesh())
	{
		std::cout << "TextFileMeshWriter::writePoints(os) called but no mesh assigned" << endl;
		return;
	}

	if (mesh()->getNp()==0)
	{
		std::cout << "TextFileMeshWriter::writePoints(os) called but no points in the mesh" << endl;
		return;
	}

	os << "# <Np = # of points> 3" << endl;

	os << mesh()->getNp() << ' ' << 3 << endl;

	const auto prec = os.precision(m_coordinatePrecision);
	const auto fmt = os.flags();

	os << fixed;

	for(unsigned i=1; i<=mesh()->getNp(); ++i)
	{
		Point<3,double> P = mesh()->getPoint(i);
		for(unsigned j=0;j<3;++j)
			os << setw(m_coordinateWidth) << P[j] << ' ';

		// TODO: additional per-point output here?

		os << endl;
	}

	os.flags(fmt);
	os.precision(prec);
}

void TextFileMeshWriter::writeFaces(std::ostream& os) const
{
	unsigned nnz=0,Nf=mesh()->getNf();

	if (m_faceFilter)
		for(int IDf=1; IDf <= Nf; ++IDf)
			nnz += (*m_faceFilter)(IDf);
	else
		nnz = Nf;

	os << endl;

	if (nnz != Nf)
	{
		os << "# <nnz>/<Nf> <D>" << endl;
		os << "# nnz is the number of faces displayed" << endl;
	}
	else
	{
		os << "# <Nf> <D>" << endl;
	}

	os << "# Nf is the total number of faces in the mesh" << endl;
	os << "# D is the number of quantities per line (1 for face ID, 3 for point IDs of face" << endl;

	os << "# Each line is ";

	if (m_showFaceID)
		os << "<IDf> ";
	os << "<p0> <p1> <p2> ";
	os << endl;

	if(m_showFaceID)
		os << "#     <IDf> = Face ID" << endl;

	os << "#     <p0..2> = point indices for the face (point IDs start at 1)" << endl;

	if (nnz != Nf)
		os << nnz << '/';

	os << Nf << ' ' << (m_showFaceID + 3) << endl;

	for(int IDf=1; IDf <= Nf; ++IDf)
	{
		if (!m_faceFilter || (*m_faceFilter)(IDf))
		{
			FaceByPointID IDps = mesh()->getFacePointIDs(IDf);
			if (m_showFaceID)
				os << setw(m_faceIDWidth) << IDf;

			for(unsigned i=0;i<3;++i)
				os << setw(m_pointIDWidth) << IDps[i] << ' ';

			os << endl;
		}
	}
	os << endl;
}

void TextFileMeshWriter::writeTetras(std::ostream& os) const
{
	unsigned nnz=0, Nt = mesh()->getNt();

	if (m_tetraFilter)
		for(unsigned IDt=1; IDt <= Nt; ++IDt)
			nnz += (*m_tetraFilter)(IDt);
	else
		nnz = Nt;

	os << endl;

	if (nnz != Nt)
	{
		os << "# <nnz>/<Nt> <D>" << endl;
		os << "# nnz is the number of tetras displayed" << endl;
	}
	else
	{
		os << "# <Nt> <D>" << endl;
	}

	os << "# Nt is the total number of tetras in the mesh" << endl;
	os << "# D is the number of quantities per line (1 for face ID, 4 for point IDs of tetra)" << endl;

	os << "# Each line is ";

	if (m_showTetraID)
		os << "<IDt> ";
	os << "<p0> <p1> <p2> <p3>";
	os << endl;

	if(m_showTetraID)
		os << "#     <IDt> = Tetra ID" << endl;

	os << "#     <p0..3> = point indices for the tetra (point IDs start at 1)" << endl;

	if (nnz != Nt)
		os << nnz << '/';

	os << Nt << ' ' << (m_showTetraID + 4) << endl;

	for(unsigned IDt=1; IDt <= Nt; ++IDt)
	{
		if (!m_tetraFilter || (*m_tetraFilter)(IDt))
		{
			TetraByPointID IDps = mesh()->getTetraPointIDs(IDt);
			if (m_showTetraID)
				os << setw(m_tetraIDWidth) << IDt;

			for(unsigned i=0;i<4;++i)
				os << setw(m_pointIDWidth) << IDps[i] << ' ';

			os << endl;
		}
	}
	os << endl;
}

//void MatlabWriter::writeComments_(std::ostream& os) const
//{
//	if (comm_.size()>0)
//	{
//		os << "% ---------- USER COMMENTS" << endl;
//		stringstream ss(comm_);
//		string l;
//		while(!ss.eof())
//		{
//			getline(ss,l);
//			os << "%     " << l << endl;;
//		}
//		os << "% ----------" << endl;
//	}
//}

//
//namespace detail {
//
///** Helper class for formatted output of (index,fluence) or just fluence to ostream.
// *
// */
//
//class FluenceWriter {
//public:
//	FluenceWriter(ostream& os) : os_(os){}
//
//	// convenience methods for dealing with (index,value) pairs -> print both index and value
//	void operator()(const boost::range::index_value<const double&,std::ptrdiff_t> p){ operator()(p.index(),p.value()); };
//	void operator()(const boost::range::index_value<double,std::ptrdiff_t> p){ operator()(p.index(),p.value()); };
//	void operator()(const std::pair<unsigned,double> p) const { operator()(p.first,p.second); }
//
//	// just a double argument: write only the fluence
//	void operator()(double phi) const { os_ << setw(phiW_) << setprecision(phiP_) << phi << endl; }
//
//	// a pair: write index and fluence
//	void operator()(unsigned i,double phi) const { os_ << setw(idxW_) << i << ' ' << setprecision(phiP_) << setw(phiW_) << phi << endl; }
//
//	ostream& os_;
//	unsigned idxW_=6,phiW_=8,phiP_=6;
//};
//
//};
//


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
//
//void MatlabWriter::writeSurfaceFluence(const std::string fn,const std::vector<double>& phi) const
//{
//	ofstream os(fn.c_str());
//
//	bool sparse = !denseOutput();
//	bool remapped = !facePerm_.empty();
//
//	unsigned Nf=0,Nfsub=0,Nnz=0;
//
//	// determine number of output values
//	assert(M_ || !"MatlabWriter needs a mesh set before writing");
//	Nf = M_->getNf();
//
//	// +1 because of dummy element at index 0
//	assert(Nf+1 == phi.size() && "Number of fluence elements does not match number of faces");
//
//	if (remapped)
//	{
//		Nfsub = facePerm_.forward().size();
//		if (sparse)
//			for(unsigned i=0;i<facePerm_.forward().size();++i)
//				Nnz += phi[facePerm_.forward()[i]] > phiMin_;
//		else
//			Nnz = Nfsub;
//	}
//	else
//	{
//		Nfsub = Nf;
//		if (sparse)
//			for(unsigned i=1;i<=Nf; ++i)
//				Nnz += phi[i];
//		else
//			Nnz=Nf;
//	}
//
//	if (sparse)
//		os << "% Sparse surface fluence with face indices starting at 1, threshold is phi > " << phiMin_ << endl;
//	else
//		os << "% Dense surface fluence starting at face index 1" << endl;
//
//	if (remapped)
//		os << "% Constructed from a subset of the original mesh" << endl;
//	else
//		os << "% Constructed using the original face numbering (may contain a large number of unused faces)" << endl;
//
//	writeComments_(os);
//
//	os << "% format below is <Nf> <Nsub> <Nnz> followed by either: " << endl;
//	os << "%    { <IDf> <phi> } if Nnz < Nsub (sparse representation)" << endl;
//	os << "%    { <phi> } else (dense)" << endl;
//
//	os << Nf << ' ' << Nfsub << ' ' << Nnz << endl;
//
//	os << fixed;
//
//	detail::FluenceWriter w(os);
//
//	if (remapped)
//	{
//		auto phiR = facePerm_.forward() | boost::adaptors::transformed([&phi](unsigned j){ return phi[j]; });
//		if (!sparse)							// Fluence for faces phi[faceP_[j]] for j in [1,Nfsub]
//			boost::for_each(phiR, w);
//		else									// Fluence (j,phi[faceP_[j]]) for j in [1,Nfsub]
//			boost::for_each(phiR | boost::adaptors::indexed(1U)
//					| boost::adaptors::filtered([this](const boost::range::index_value<double,std::ptrdiff_t> p){ return p.value()>phiMin_; }),
//				w);
//	}
//	else
//	{
//		if (!sparse)							// Fluence for faces phi[i] for i in [1..Nf]
//			boost::for_each(phi | drop(1), w);
//		else									// sparse output: index + fluence for all elements of subset exceeding threshold
//			boost::for_each(phi
//				| drop(1)
//				| boost::adaptors::indexed(1U)
//				| boost::adaptors::filtered([this](const boost::range::index_value<const double&,std::ptrdiff_t> p){ return p.value() > phiMin_; }),
//					w);
//
//	}
//}

void TextFileMeshWriter::write() const
{
	std::ofstream os(fileName().c_str());

	if (m_showPoints)
		writePoints(os);

	if (m_showFaces)
		writeFaces(os);

	if (m_showTetras)
		writeTetras(os);
}
