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

#endif

#include <vector>
#include <fstream>

#ifndef SWIG
#define SWIG_OPENBRACE
#define SWIG_CLOSEBRACE
#else
%{ #include "MatlabWriter.hpp" %}
#endif

SWIG_OPENBRACE
SWIG_CLOSEBRACE

class TetraMesh;


/** Writes to Matlab text files.
 * Generally such files use 1-based indexing and ASCII float representation.
 *
 */

// surface file: points & triangles
// fluence:			<fluence>				for all elements sequentially
// sparsefluence:	<faceidx> <fluence>		for all elements with nonzero fluence

class MatlabWriter {

public:

	MatlabWriter();

	void setMesh(const TetraMesh* M){ M_ = M; }

	void setComment(const std::string);

	void setFaceSubset(const std::vector<unsigned>&);
	void setFacesToAll();

	void writeFaces(const std::string fn) const;
	void writeSparseSurfaceFluence(const std::vector<double>& fluence,double threshold=-1.0) const;

private:
	// point permutation & inverse permutation
	std::vector<unsigned> pointP_,pointQ_;

	// face permutation
	std::vector<unsigned> faceP_;

	std::string comm_;

	const TetraMesh *M_=nullptr;
};




#endif /* STORAGE_MATLAB_MATLABWRITER_HPP_ */
