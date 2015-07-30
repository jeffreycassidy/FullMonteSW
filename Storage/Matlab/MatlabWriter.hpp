/*
 * MatlabWriter.hpp
 *
 *  Created on: Jul 16, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_MATLAB_MATLABWRITER_HPP_
#define STORAGE_MATLAB_MATLABWRITER_HPP_

#ifdef SWIG
%module FullMonteMatlabWriter_TCL

%include "std_string.i"
%include "std_vector.i"

%template(DoubleVector) std::vector<double>;

#endif

#include <vector>
#include <fstream>

#ifndef SWIG
#define SWIG_OPENBRACE
#define SWIG_CLOSEBRACE
#else
%{ #include "MatlabWriter.hpp" %}
#endif


class TetraMesh;


/** Writes to Matlab text files.
 * Generally such files use 1-based indexing and ASCII float representation.
 *
 */

class MatlabWriter {

public:

	MatlabWriter();

	void setMesh(const TetraMesh* M){ M_ = M; }

	void setComment(const std::string);

	void setFaceSubset(const std::vector<unsigned>&);
	void setFacesToAll();

	void writeFaces(const std::string fn) const;
	void writeSurfaceFluence(const std::string fn,const std::vector<double>& fluence) const;

	// threshold control (no threshold -> dense representation)
	double threshold() const { return phiMin_; }
	double threshold(double phiMin){ std::swap(phiMin,phiMin_); return phiMin; }
	void removeThreshold(){ phiMin_=-1.0; }

	bool denseOutput() const { return phiMin_==-1.0; }


	// write out original indices when working with subset faces
	bool outputOriginalIndices() const { return outputOriginalIndices_; }
	bool outputOriginalIndices(bool wi){ std::swap(wi,outputOriginalIndices_); return wi; }


private:

	void writeComments_(std::ostream& os) const;

	// point permutation & inverse permutation
	std::vector<unsigned> pointP_,pointQ_;

	// face permutation
	std::vector<unsigned> faceP_;

	std::string comm_;

	double phiMin_=0.0;		// fluence threshold (>= comparison); negative means none/dense output

	bool outputOriginalIndices_=true;

	const TetraMesh *M_=nullptr;
};




#endif /* STORAGE_MATLAB_MATLABWRITER_HPP_ */
