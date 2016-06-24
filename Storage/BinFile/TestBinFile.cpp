/*
 * TestBinFile.cpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#include "../TIMOS/TIMOSReader.hpp"
#include "BinFile.hpp"

int main(int argc,char **argv)
{
	string ipfx("../../../data/mouse");
	string opfx("mouse");

	// load the mouse
	TIMOSReader tr(ipfx);
	TetraMesh M;
	M = tr.mesh();

	// write to binary file
	BinFileWriter w(opfx);
	w.write(M);

	// read back
	cout << "Reading back from " << opfx << ".bin" << endl;
	BinFileReader br(opfx);
	TetraMesh Mi = br.mesh();

	return 0;
}

