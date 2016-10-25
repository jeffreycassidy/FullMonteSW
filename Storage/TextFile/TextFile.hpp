/*
 * TextFile.hpp
 *
 *  Created on: Nov 12, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_TEXTFILE_TEXTFILE_HPP_
#define STORAGE_TEXTFILE_TEXTFILE_HPP_

#include <string>

class TetraMesh;

class TextFile
{
public:
	enum Mode { Surface, Volume };

	/// Get/set attached mesh
	void 					mesh(const TetraMesh*);
	const TetraMesh*		mesh() 					const;

	/// Select surface/volume mode
	void 					mode(Mode m);
	Mode					mode()					const;

	/// Get/set file name
	const std::string&		fileName() const;
	void 					fileName(std::string);

protected:
	TextFile();
	virtual ~TextFile();

private:
	std::string			m_fileName;
	const TetraMesh*	m_mesh=nullptr;
	Mode				m_mode;
};


#endif /* STORAGE_TEXTFILE_TEXTFILE_HPP_ */
