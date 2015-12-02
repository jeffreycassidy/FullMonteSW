/*
 * PacketTrace.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_PACKETPOSITIONTRACE_HPP_
#define OUTPUTTYPES_PACKETPOSITIONTRACE_HPP_

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

class PacketPositionTrace
{

public:

	PacketPositionTrace(){}

	PacketPositionTrace(std::vector<TraceStep>&& tr) : trace_(std::move(tr)){}

	static std::vector<PacketPositionTrace> loadTextFile(const std::string fn)
	{
		std::vector<PacketPositionTrace> 	v;
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

		std::cout << "  Has " << Ntr << " traces" << std::endl;

		for(unsigned tr=0;tr < Ntr && !is.eof(); ++tr)
		{
			is >> Nst;

			std::cout << "    " << Nst << " steps" << std::endl;

			traceSteps.resize(Nst);

			for(unsigned i=0;i<Nst && !is.eof(); ++i)
				is >> traceSteps[i].pos[0] >> traceSteps[i].pos[1] >> traceSteps[i].pos[2] >> traceSteps[i].w;
			v[tr] = PacketPositionTrace(std::move(traceSteps));
		}

		return v;
	}

	std::vector<TraceStep>::const_iterator begin() const { return trace_.begin(); }
	std::vector<TraceStep>::const_iterator end() const { return trace_.end(); }

	const std::vector<TraceStep>& steps() const { return trace_; }

private:
	std::vector<TraceStep> trace_;

};



#endif /* OUTPUTTYPES_PACKETPOSITIONTRACE_HPP_ */
