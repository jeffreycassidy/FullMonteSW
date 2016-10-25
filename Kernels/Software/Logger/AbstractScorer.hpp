/*
 * AbstractScorer.hpp
 *
 *  Created on: Sep 29, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_ABSTRACTSCORER_HPP_
#define KERNELS_SOFTWARE_LOGGER_ABSTRACTSCORER_HPP_

#include <list>

#include "tuple_for_each.hpp"

/** Scorer Concept
 *
 * A Scorer manages the scoring process, including the following responsibilities:
 *
 * 	1) Acting as a Logger factory when the Kernel starts up
 * 	2) Providing get/set methods for options for creation of the Logger
 * 	3) Conversion of the Logger information into a finalized OutputData*
 *
 *
 *
 *
 * Concept requirements
 *
 * 		Default constructible
 *
 * 		virtual std::list<OutputData*> results() const			Returns 0 or more results holding a copy of the current state
 * 		virtual void clear()									Resets all scores to a default state
 *
 * 		typedef Logger											The type of logger created
 * 		Logger createLogger();									Logger factory
 */

class OutputData;

#include <FullMonteSW/OutputTypes/OutputData.hpp>

class AbstractScorer
{
public:
	virtual ~AbstractScorer();

	virtual void clear()=0;

	/// Return a list of 0 or more OutputData elements holding the current results
	virtual std::list<OutputData*> results() const=0;

protected:
	AbstractScorer();

private:
};

inline std::list<OutputData*> results(const AbstractScorer& S)
{
	//std::list<OutputData*> r = S.results();
	//for(const auto p : r)
		//std::cout << "  Result: " << p << std::endl;//p->typeString() << std::endl;
	std::cout << "Ping!" << std::endl;
	return std::list<OutputData*>();
}




/** Functor class that appends the results from an AbstractScorer descendent into a list */

struct AppendResults
{
	AppendResults(){}
	AppendResults operator()(const AbstractScorer& S){ results.splice(results.end(), S.results()); return *this; }

	std::list<OutputData*> results;
};

template<typename... Scorers>std::list<OutputData*> results(const std::tuple<Scorers...>& Ss)
{
	return tuple_for_each(Ss,AppendResults()).results;
}

#endif /* KERNELS_SOFTWARE_LOGGER_ABSTRACTSCORER_HPP_ */
