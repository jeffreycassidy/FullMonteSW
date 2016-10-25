/*
 * TextPositionTraceWriter.cpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */


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



