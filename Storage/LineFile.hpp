/*
 * LineFile.hpp
 *
 *  Created on: Nov 7, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_HEXFILE_LINEFILE_HPP_
#define STORAGE_HEXFILE_LINEFILE_HPP_


#include <boost/iterator/iterator_facade.hpp>

#include <boost/range/iterator_range.hpp>

/** Produce a stream of lines as strings.
 *
 * Can be configured with options below in LineStreamConfig
 *
 */

struct LineStreamConfig
{
	bool skipEmptyLastLine=true;			// skip the line immediately before EOF if it's empty
	bool skipEmptyLines=false;				// skip all empty lines
	char singleLineCommentChar='%';			// strip lines starting with this character
};

class LineStreamInputIterator : public boost::iterator_facade<
	LineStreamInputIterator,
	std::string,
	std::input_iterator_tag,
	const std::string&,
	std::ptrdiff_t>
{
public:
	LineStreamInputIterator() : m_is(*(std::istream*)nullptr),m_lineNumber(-1U){}
	LineStreamInputIterator(std::istream& is,LineStreamConfig& cfg,std::ptrdiff_t lineNumber=0) :
		m_is(is),
		m_cfg(cfg),
		m_lineNumber(lineNumber)
	{
		// read first line so dereference() returns a valid value
		increment();
	}

	LineStreamInputIterator(const LineStreamInputIterator&) = default;

	std::ptrdiff_t lineNumber() const { return m_lineNumber; }



private:
	void increment()
	{
		do
		{
			if (m_is.eof())
			{
				m_lineNumber=-1U;
				m_s.clear();
				return;
			}

			++m_lineNumber;
			std::getline(m_is,m_s);
		}
		while(skipLine(m_s));

		if (m_cfg.skipEmptyLastLine && m_is.eof() && m_s.empty())
			m_lineNumber=-1U;
	}

	const std::string& dereference() const
	{
		return m_s;
	}

	// equal if line numbers are equal, or if rhs is end-of-file
	bool equal(const LineStreamInputIterator& rhs) const { return rhs.m_lineNumber == m_lineNumber; }

	std::istream&		m_is;
	std::string			m_s;
	LineStreamConfig	m_cfg;
	std::ptrdiff_t		m_lineNumber=1;




	bool skipLine(const std::string l) const
	{
		if (m_cfg.skipEmptyLines && l.empty())							// empty line
			return true;

		if (l.size() > 0 && l[0] == m_cfg.singleLineCommentChar)		// comment line
			return true;

		return false;
	}

	friend boost::iterator_core_access;
};




class LineStream
{

public:
	LineStream(std::istream& is) : m_is(is){}

	void skipEmptyLastLine(bool s){ m_cfg.skipEmptyLastLine=s; }
	void singleLineCommentChar(char c){ m_cfg.singleLineCommentChar = c; }
	void skipEmptyLines(bool s){ m_cfg.skipEmptyLines=s; }

	boost::iterator_range<LineStreamInputIterator> lines()
	{
		return boost::iterator_range<LineStreamInputIterator>(
			LineStreamInputIterator(m_is,m_cfg,1ULL),
			LineStreamInputIterator());
	}

private:
	LineStreamConfig	m_cfg;
	std::istream& 		m_is;
};


#endif /* STORAGE_HEXFILE_LINEFILE_HPP_ */
