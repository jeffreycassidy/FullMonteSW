#include "LoggerSurface.hpp"

void writeFileBin(string,const vector<Point<3,double> >&,const map<FaceByPointID,double>&);

/*void LoggerSurface::collect()
{
    vector<FaceByPointID>::const_iterator Fp=mesh.F_p.begin();

    for(vector<pair<unsigned,double> >::iterator it=counts.begin(); it != counts.end(); ++it,++Fp)
    {
        assert(Fp != mesh.F_p.end());

        if (it->first != 0)

        {
            const Point<3,double> &A = mesh.P[(*Fp)[0]], &B = mesh.P[(*Fp)[1]], &C = mesh.P[(*Fp)[2]];
            Vector<3,double> AB(A,B), AC(A,C);
            double area=cross(AB,AC).norm_l2();
            assert(area != 0.0);
            it->second /= area;
        }
    }
}*/
/*
void LoggerSurface::writeFileASCII(string fn,bool printIDs)
{
	unsigned i=0;
    FaceByPointID fID;
    
	ofstream os(fn.c_str());

//    collect();

	for(vector<pair<unsigned,double> >::const_iterator it=counts.begin(); it != counts.end(); ++it,++i){
        {
            if (printIDs && it->first > 0)
            {
                fID = mesh.getFacePointIDs(i);
                os << fID[0] << ' ' << fID[1] << ' ' << fID[2] << ' ';
            }
		    if (it->first > 0)
			    os << i << ' ' << it->first << ' ' << it->second << endl;
        }
	}
}*/

/*void LoggerSurface::writeFileASCII(string fn,const map<FaceByPointID,double>& m,bool printIDs)
{
	unsigned i=0;
	ofstream os(fn.c_str());

    if (!os.good())
    {
        cerr << "LoggerSurface::writeFileASCII() - Failed to open " << fn << " for writing" << endl;
        return;
    }
    else
        cout << "INFO: Writing fluence results to " << fn << endl;

    for(map<FaceByPointID,double>::const_iterator it=m.begin(); it != m.end(); ++it,++i)
    {
        if (printIDs && it->second > 0)
            os << it->first[0] << ' ' << it->first[1] << ' ' << it->first[2] << ' ';
        if (it->second > 0)
			    os << i << ' ' << 0 << ' ' << it->second << endl;
	}
    os.close();
}*/

void SurfaceArray::fluenceMap(SurfaceFluenceMap& F)
{
    F.clear();

    // copy from vector to map, eliminating zero entries and dividing by face area
    for(TetraMesh::boundary_f_const_iterator it=mesh.boundaryFaceBegin(); it != mesh.boundaryFaceEnd(); ++it)
    {
        unsigned IDf = it->first;
        if (getValue(counts[IDf]) != 0)
            F[IDf] = getValue(counts[IDf])/mesh.getFaceArea(IDf);
    }
}

void SurfaceArray::hitMap(map<unsigned,unsigned long long>& m)
{
    m.clear();
    map<unsigned,unsigned long long>::iterator m_it=m.begin();
    unsigned i=1;
    if(counts.size()<2)
        return;
    for(vector<FluenceCountType>::const_iterator it=counts.begin()+1; it != counts.end(); ++it,++i)
        if (getHits(*it) != 0)
            m_it = m.insert(m_it,make_pair(i,getHits(*it)));
}

void SurfaceArray::resultMap(map<FaceByPointID,double>& m,bool per_area)
{
    m.clear();

    // copy from vector to map
    for(TetraMesh::boundary_f_const_iterator it=mesh.boundaryFaceBegin(); it != mesh.boundaryFaceEnd(); ++it)
    {
        if (getValue(counts[it->first]) != 0)
            m.insert(make_pair(mesh.getFacePointIDs(it->first),getValue(counts[it->first])));
    }

    for(map<FaceByPointID,double>::iterator it=m.begin(); per_area && it != m.end(); ++it)
    {
        const Point<3,double> &A = mesh.getPoint(it->first[0]),
            &B = mesh.getPoint(it->first[1]),
            &C = mesh.getPoint(it->first[2]);
        Vector<3,double> AB(A,B), AC(A,C);
        double area=cross(AB,AC).norm_l2()/2;
        assert(area != 0.0);
        it->second /= area;
    }
}

// Writes out a VTK datafile (v3.0)
/*void LoggerSurface::writeFileVTK(string fn)
{
    map<FaceByPointID,double> m;

    for(TetraMesh::boundary_f_const_iterator it=mesh.boundaryFaceBegin(); it != mesh.boundaryFaceEnd(); ++it)
    {
        FaceByPointID f = *it;

        const Point<3,double> &A = mesh.P[mesh.F_p[it->first][0]], &B = mesh.P[mesh.F_p[it->first][1]], &C = mesh.P[mesh.F_p[it->first][2]];
        Vector<3,double> AB(A,B), AC(A,C);
        double area=cross(AB,AC).norm_l2();
        assert(area != 0.0);
        if(area != 0.0)
            m.insert(make_pair(mesh.F_p[it->first],counts[it->first].second/area));
        else
            m.insert(make_pair(*it,0));
    }

    ::writeFileVTK(fn,mesh.P,m);
}*/
