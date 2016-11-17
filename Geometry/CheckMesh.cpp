/*
 * CheckMesh.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: jcassidy
 */

#include "TetraMesh.hpp"

#include <FullMonteSW/Storage/VTK/VTKLegacyReader.hpp>

#include <string>
#include <iostream>

#include "MeshChecker.hpp"

#include "Modifiers/AffineTransform.hpp"

using namespace std;

typedef AffineTransform<float,3> AffineTransform3f;

int main(int argc,char **argv)
{
	std::string fn = argv[1];

	VTKLegacyReader R;
	R.setFileName(fn);

	cout << "Reading from file " << fn << endl;

	TetraMeshBase MB = R.mesh();

	AffineTransform3f T = AffineTransform3f::scale(
			std::pow(1.3f,0.3333f),
			std::array<float,3>{{53.7f, -59.7f, 135.3f }});

	MB.apply(T);

	TetraMesh M = MB;

	MeshChecker C(&M);

	for(unsigned i=1;i<=M.getNt();++i)
	{
		C.dihedrals(i);
		C.pointHeights(i);
	}
	return 0;
}


