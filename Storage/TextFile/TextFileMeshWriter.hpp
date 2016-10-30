/*
 * TextFileMeshWriter.hpp
 *
 *  Created on: Oct 7, 2016
 *      Author: jcassidy
 */

#ifndef STORAGE_TEXTFILE_TEXTFILEMESHWRITER_HPP_
#define STORAGE_TEXTFILE_TEXTFILEMESHWRITER_HPP_

#include "TextFile.hpp"

template<typename T>class FilterBase;

class TextFileMeshWriter : public TextFile
{
public:
	TextFileMeshWriter();
	virtual ~TextFileMeshWriter();

	/// Write the file
	void write() const;

	void writeHintFile() const;

	void 					faceFilter(const FilterBase<int>* F);
	const FilterBase<int>* 	faceFilter() 							const;

private:
	void writePoints(std::ostream& os) 	const;
	void writeTetras(std::ostream& os) 	const;
	void writeFaces(std::ostream& os) 	const;

	unsigned m_coordinatePrecision = 4;
	unsigned m_coordinateWidth = 7;

	unsigned m_pointIDWidth=6;
	unsigned m_tetraIDWidth=6;
	unsigned m_faceIDWidth=7;

	const FilterBase<int>*		m_faceFilter=nullptr;
	const FilterBase<unsigned>* m_tetraFilter=nullptr;
	bool m_showTetraID=true;

	bool m_showFaceID=true;

	bool m_showPoints=true;
	bool m_showFaces=true;
	bool m_showTetras=true;
	bool m_showMaterial=true;
};



#endif /* STORAGE_TEXTFILE_TEXTFILEMESHWRITER_HPP_ */
