#ifndef BINFILE_HPP_INCLUDED_
#define BINFILE_HPP_INCLUDED_

#ifndef SWIG

#include <string>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>

#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/FullMonte/Kernels/Software/Material.hpp>
#include <FullMonteSW/Geometry/SourceDescription.hpp>

#include "../TIMOS/TIMOS.hpp"

#endif


class BinFileReader : public Reader {
public:
	BinFileReader() {}
	BinFileReader(std::string pfx) : sourceFn_(pfx+".source.bin"),optFn_(pfx+".opt.bin"),meshFn_(pfx+".mesh.bin"),legendFn_(pfx+".legend.bin"){}

	virtual ~BinFileReader(){}

	void setMeshFileName(std::string fn){ meshFn_=fn; }
	void setOpticalFileName(std::string fn){ optFn_=fn; }
	void setSourceFileName(std::string fn){ sourceFn_=fn; }
	void setLegendFileName(std::string fn){ legendFn_=fn; }

	virtual TetraMesh						mesh() const;
	virtual std::vector<SourceDescription*>	sources() const;

	virtual std::vector<SimpleMaterial>		materials_simple() const;

	virtual std::vector<LegendEntry> 		legend() const;

	virtual void clear(){}

private:
	std::string sourceFn_,optFn_,meshFn_,legendFn_;
};



class BinFileWriter : public Writer {

	std:: string sourceFn_, optFn_, meshFn_;

	static void writeUserComments(std::ostream&,std::string);

public:
	BinFileWriter(std::string pfx) : Writer(),sourceFn_(pfx+".source.bin"),optFn_(pfx+".opt.bin"),meshFn_(pfx+".mesh.bin"){}

	virtual ~BinFileWriter(){}

	virtual void write(const TetraMesh&) const;
	virtual void write(const std::vector<SourceDescription*>&) const;

	virtual void writeSurfFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
	virtual void writeVolFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
};

#endif /* TIMOS_HPP_ */
