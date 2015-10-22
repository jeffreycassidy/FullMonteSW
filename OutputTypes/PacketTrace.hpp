/*
 * PacketTrace.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_PACKETTRACE_HPP_
#define OUTPUTTYPES_PACKETTRACE_HPP_

#include <iostream>
#include <fstream>
#include <array>
#include <vector>

struct TraceStep
	{
		std::array<float,3>		pos;
		float					w;
	};


/** Holds the trace of one packet, as a series of (pos,w) steps.
 *
 */

class PacketTrace
{

public:

	PacketTrace(){}

	static std::vector<PacketTrace> loadTextFile(const std::string fn)
	{
		std::vector<PacketTrace> 	v;
		std::vector<TraceStep>   	traceSteps;

		std::size_t Ntr,Nst;

		std::ifstream is (fn.c_str());

		std::string s;

		if (!is.good())
			throw std::logic_error("Failed to open file");

		// strip leading comments

		char c;

		while((c=is.peek()), !is.eof() && (c == '%' || c == '#'))
		{
			std::getline(is,s);
			std::cout << "INFO: comment line '" << s << "'" << std::endl;
		}

		is >> Ntr;

		v.resize(Ntr);

		TraceStep tmp;

		std::cout << "  Has " << Ntr << " traces" << std::endl;

		for(unsigned tr=0;tr < Ntr && !is.eof(); ++tr)
		{
			is >> Nst;

			std::cout << "    " << Nst << " steps" << std::endl;

			traceSteps.resize(Nst);

			for(unsigned i=0;i<Nst && !is.eof(); ++i)
				is >> traceSteps[i].pos[0] >> traceSteps[i].pos[1] >> traceSteps[i].pos[2] >> traceSteps[i].w;
			v[tr] = PacketTrace(std::move(traceSteps));
		}

		return v;
	}

	const std::vector<TraceStep>& steps() const { return trace_; }

private:
	std::vector<TraceStep> trace_;

	PacketTrace(std::vector<TraceStep>&& tr) : trace_(tr){}
};


/** Converts a Range of PacketTrace to a vtkPolyData
 *
 */

#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyDataWriter.h>

template<class Range>vtkPolyData* tracesToPolyLines(const Range& r)
{
	vtkPoints *pts = vtkPoints::New();
	vtkCellArray *ca = vtkCellArray::New();

	for(const auto trace : r)
	{
		ca->InsertNextCell(trace.steps().size());
		for(TraceStep ts : trace.steps())
		{
			vtkIdType IDp = pts->InsertNextPoint(ts.pos.data());
			ca->InsertCellPoint(IDp);
		}
	}

	vtkPolyData* pd = vtkPolyData::New();
	pd->SetPoints(pts);
	pd->SetLines(ca);

	return pd;
}

#ifdef SWIG
%template(tracesToPolyLinesV) tracesToPolyLines<std::vector<PacketTrace>>;
#endif

void convertTraces(const std::string ifn,const std::string ofn)
{
	std::vector<PacketTrace> v = PacketTrace::loadTextFile(ifn);

	std::cout << "Loaded " << v.size() << " traces from " << ifn << std::endl;

	vtkPolyData* pd = tracesToPolyLines(v);

	vtkPolyDataWriter *W = vtkPolyDataWriter::New();
	W->SetInputData(pd);
	W->SetFileName(ofn.c_str());
	W->Update();
	W->Delete();

	pd->Delete();
}



#endif /* OUTPUTTYPES_PACKETTRACE_HPP_ */
