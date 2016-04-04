/*
 * MeshChecker.cpp
 *
 *  Created on: Apr 4, 2016
 *      Author: jcassidy
 */

#include "TetraMesh.hpp"
#include "Tetra.hpp"

#include "MeshChecker.hpp"

#include <array>
#include <iostream>

#include "Basis.hpp"

using namespace std;

bool MeshChecker::dihedrals(unsigned IDt) const
{
	Tetra T = m_mesh->getTetra(IDt);
	bool tetOK=true;
//
//	for(unsigned i=0;i<4;++i)
//		for(unsigned j=i+1;j<4;++j)
//		{
//			bool normal_ij_correct = dot(T.face_normal(i),T.face_normal(j)) >= 0;
//			tetOK &= normal_ij_correct;
//			if (!normal_ij_correct)
//				cout << "Error in tetra ID " << IDt << ": normals of faces " << i << " (ID " << T.IDfs[i] << ") and " << j << " (ID " << T.IDfs[j] << ") " <<
//					" have positive dot product, meaning tetra is unbounded" << endl;
//		}

	return tetOK;
}

bool MeshChecker::pointHeights(unsigned IDt) const
{
	Tetra T = m_mesh->getTetra(IDt);
	TetraByPointID IDps = m_mesh->getTetraPointIDs(IDt);

	array<array<float,3>,4> tetraPointCoords;

	for(unsigned i=0;i<4;++i)
		tetraPointCoords[i] = m_mesh->points()[IDps[i]].as_array_of<float>();

	bool tet_ok=true;
	for(int f=0;f<4;++f)
	{
		// negative height -> inside tetra
		array<float,4> h = T.heights(tetraPointCoords[f]);

		for(unsigned i=0;i<4;++i)
			if (h[i] > m_pointHeightTolerance)
			{
				tet_ok=false;
				cout << "Error: incorrect height of " << f << "'th tetra point (ID " << IDps[f] << ") over " << i << "'th face (ID" << T.IDfs[i] << "): " << h[i] << endl;
			}

		if (!tet_ok)
		{
			cout << "tet " << IDt << " face " << f << " (ID " << T.IDfs[f] << ") opposite-corner heights: ";
			for(unsigned i=0;i<4;++i)
				cout << h[i] << ' ';
			cout << endl;
		}
	}

	return tet_ok;
}


