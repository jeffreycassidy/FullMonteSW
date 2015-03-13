%module FullMonteTIMOS_TCL

%include "std_string.i"
%apply std::string { const std::string* }

%typemap(out) std::array<float,3> {
	stringstream ss;
	std::array<float,3> a=$1;
	for(unsigned i=0;i<3;++i)
		ss << a[i] << ' ';
	Tcl_AppendResult(interp,ss.str().c_str());
}


%{
	#include "TIMOSReader.hpp"
	std::string getLabel(const LegendEntry& l);
%}


%include "std_vector.i"
namespace std{
	%template(vectorlegend) vector<LegendEntry>;
	%template(sourcevector) vector<SourceDescription*>;
};

struct LegendEntry {
	std::string label;
	std::array<float,3> colour;
};

class TIMOSReader {
	public:
	TIMOSReader();
	TIMOSReader(std::string pfx);
	
	void setMeshFileName(std::string);
	void setOpticalFileName(std::string);
	void setSourceFileName(std::string);
	void setLegendFileName(std::string);
	
	virtual TetraMesh						mesh() const;
	virtual std::vector<Material>			materials() const;
	virtual std::vector<SourceDescription*>	sources() const;

	virtual std::vector<LegendEntry> legend() const;

	virtual void clear(){}
};
