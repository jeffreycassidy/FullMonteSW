/*
 * TextFile.cpp
 *
 *  Created on: Oct 7, 2016
 *      Author: jcassidy
 */

#include "TextFile.hpp"

TextFile::TextFile()
{

}

TextFile::~TextFile()
{

}

const TetraMesh* TextFile::mesh() const
{
	return m_mesh;
}

void TextFile::mesh(const TetraMesh* M)
{
	m_mesh=M;
}

void TextFile::mode(Mode m)
{
	m_mode = m;
}

TextFile::Mode TextFile::mode() const
{
	return m_mode;
}

const std::string& TextFile::fileName() const
{
	return m_fileName;
}

void TextFile::fileName(const std::string s)
{
	m_fileName=s;
}
