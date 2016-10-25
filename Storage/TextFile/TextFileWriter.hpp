/*
 * TextFileWriter.hpp
 *
 *  Created on: Mar 6, 2016
 *      Author: jcassidy
 */

#ifndef STORAGE_TEXTFILE_TEXTFILEWRITER_HPP_
#define STORAGE_TEXTFILE_TEXTFILEWRITER_HPP_

#include <string>

class TextFileWriter : public OutputData::Visitor
{
public:
	virtual ~TextFileWriter(){}
	TextFileWriter(std::string pfx="output") : m_fnPfx(pfx){}

	void fileNamePrefix(std::string pfx){ m_fnPfx=pfx; }


private:
//	virtual void doVisit(FluenceLineQuery* lq) override;
	virtual void doVisit(OutputData* lq) override;
	std::string m_fnPfx="output";

};


#endif /* STORAGE_TEXTFILE_TEXTFILEWRITER_HPP_ */
