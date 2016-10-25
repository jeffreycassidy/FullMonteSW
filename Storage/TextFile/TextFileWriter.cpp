/*
 * TextFileWriter.cpp
 *
 *  Created on: Mar 6, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Queries/FluenceLineQuery.hpp>
#include <FullMonteSW/Storage/TextFile/TextFileWriter.hpp>

#include <vector>

#include <fstream>
#include <iostream>
#include <iomanip>


using namespace std;

//void TextFileWriter::doVisit(FluenceLineQuery* lfq)
void TextFileWriter::doVisit(OutputData* lfq)
{
//	ofstream os(m_fnPfx+".line.txt");
//
//	os << "# Text file output from FullMonte: fluence line query" << endl;
//	os << "# " << endl;
//	os << "# File format:" << endl;
//	os << "# <Nt>" << endl;
//	os << "#     {<d> <tetID> <phi>}Nt" << endl;
//	os << "# <Nifc>" << endl;
//	os << "#     {<d> <matID>}Nifc" << endl;
//	os << "#" << endl;
//	os << "#" << endl;
//	os << "# Nt     Number of tetras sampled along ray" << endl;
//	os << "# d      Depth from surface where tetra starts" << endl;
//	os << "# tetID  ID number of the tetra" << endl;
//	os << "# phi    Average fluence J/cm2 within volume element" << endl;
//	os << "# Nifc   Number of material interfaces crossed by ray" << endl;
//	os << "# matID  Material ID" << endl;
//
//	unsigned Nt = boost::size(lfq->result());
//
//	os << Nt << endl;
//
//	os << setprecision(3) << fixed;
//
//	vector<pair<float,unsigned>> matBoundary;
//
//	const auto R = lfq->result();
//	unsigned matID_last=(*begin(R)).matID;
//
//	matBoundary.emplace_back(0.0f,matID_last);
//
//	for(const auto seg : R)
//	{
//		os << "  " << setw(8) << setprecision(3) << seg.dToOrigin << ' ' << setw(7) << seg.IDt << ' ' <<
//				setprecision(4) << setw(8) << seg.phi << endl;
//		if (matID_last != seg.matID)
//			matBoundary.emplace_back(seg.dToOrigin,seg.matID);
//		matID_last = seg.matID;
//	}
//
//	os << matBoundary.size() << endl;
//
//	for(const auto b : matBoundary)
//		os << fixed << setw(8) << b.first << ' ' << setw(3) << b.second << endl;
}
