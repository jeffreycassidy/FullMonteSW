/*
 * TIMOS.hpp
 *
 *  Created on: Mar 5, 2015
 *      Author: jcassidy
 */

#ifndef TIMOSREADER_HPP_
#define TIMOSREADER_HPP_

#include "TIMOS.hpp"
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <unordered_map>


class TIMOSReader : public TIMOS
{
public:
	TIMOSReader() {}
	TIMOSReader(std::string pfx) : TIMOS(pfx){}

	virtual ~TIMOSReader(){}

	/// Load the basic mesh (points, tetra connectivity, and region assignments) and create full data structure
	TetraMesh								mesh() const;

	/// Just load the basic mesh
	virtual TetraMeshBase					mesh_base() const;

	virtual Source::Abstract*					sources() const;

	virtual std::vector<SimpleMaterial>		materials_simple() const;
#ifndef SWIG
	virtual std::vector<LegendEntry> 		legend() const;
#endif

	virtual void clear() {};

	void hintFileName(std::string fn);

private:

	std::vector<TetraMesh::FaceHint> readHintFile(const std::string fn) const;

	bool m_useHintFileWhenAvailable=true;

	std::string m_hintFileName;

	virtual Optical 					parse_optical(std::string fn) const=0;
	virtual std::vector<SourceDef>		parse_sources(std::string fn) const=0;
	virtual Mesh						parse_mesh(std::string fn)  const	=0;
	virtual std::vector<LegendEntry> 	parse_legend(std::string fn) const=0;
};



#endif /* TIMOS_HPP_ */

