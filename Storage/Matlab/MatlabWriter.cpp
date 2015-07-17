/*
 * MatlabWriter.cpp
 *
 *  Created on: Jul 16, 2015
 *      Author: jcassidy
 */

#include "MatlabWriter.hpp"

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <tuple>

using namespace std;

MatlabWriter::MatlabWriter(){}

/** Selects the subset of faces which are exported
 *
 */

void MatlabWriter::setFaceSubset(const std::vector<unsigned>& s)
{
	faceP_ = s;
	tie(pointP_,pointQ_) = make_point_perm(s | boost::adaptors::transformed([this](unsigned i){ return M_->getFacePointIDs(i); }),
			M_->getNp()+1);
}

/** Clears the face subset vectors so all faces are exported (generally not advised since it's much larger)
 *
 */

void MatlabWriter::setFacesToAll()
{
	pointP_.clear();
	pointQ_.clear();
	faceP_.clear();
}

void MatlabWriter::setComment(const std::string comm)
{
	comm_ = comm;
}

void MatlabWriter::writeFaces(const std::string fn) const
{
	ofstream os(fn.c_str());

	os << "% Surface mesh; point and face indices start at 1" << endl;

	if (pointQ_.empty())
		os << "% Constructed using the original point numbering (may contain a large number of unused points)" << endl;
	else
		os << "% Constructed from a subset of the original mesh, with points renumbered" << endl;

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

	os << "% format below is <Np> <Nd=3> { <x> <y> <z> }Np <Nf> <Ns=3> { <p0> <p1> <p2> }Nf" << endl;

	// write points
	os << (pointP_.empty() ? M_->getNp() : pointP_.size()) << " 3"  << endl;

	os << fixed << setprecision(6);

	if (pointP_.empty())
		for(unsigned i=1;i<=M_->getNp();++i)					// Drop dummy point 0; Matlab starts index at 1 so will line up
		{
			for(unsigned j=0;j<3;++j)
				os << setw(10) << M_->getPoint(i)[j] << ' ';
			os << endl;
		}
	else
		for(unsigned i=0;i<pointP_.size();++i)
		{
			assert(pointP_[i] != 0);							// Check for correct remapping
			assert(pointP_[i] != -1U);
			for(unsigned j=0;j<3;++j)
				os << setw(10) << M_->getPoint(pointP_[i])[j] << ' ';
			os << endl;
		}



	if (faceP_.empty())
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
		os << faceP_.size() << " 3" << endl;
		for(unsigned i=0;i<faceP_.size();++i)
		{
			std::array<unsigned,3> y = remap(M_->getFacePointIDs(faceP_[i]),pointQ_);
			for(unsigned j=0;j<3;++j)
				os << setw(5) << y[j]+1 << ' ';
			os << endl;
		}
	}
}

void MatlabWriter::writeSparseSurfaceFluence(const std::vector<double>&,double) const
{
}
