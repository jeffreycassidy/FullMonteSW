#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include <math.h>
#include "linefile.hpp"
#include "graph.hpp"
#include "SourceDescription.hpp"
#include "newgeom.hpp"
#include "fluencemap.hpp"

#include "io_timos.hpp"

using namespace std;

bool writeTIMOSSource(string fn,const vector<SourceDescription*>& s,long long packetcount)
{
    double w_sum=0.0;
    unsigned long Np;
    long long Np_total=0;
    ofstream os(fn.c_str());

    if (!os.good())
        return false;

    // First line: number of sources
    os << s.size() << endl;

    if (packetcount != 0){
        for(vector<SourceDescription*>::const_iterator it=s.begin(); it != s.end(); ++it)
            w_sum += (*it)->getPower();
    
        for(vector<SourceDescription*>::const_iterator it=s.begin(); it != s.end()-1; ++it)
        {
            Np = rint(((*it)->getPower()/w_sum) * (double)packetcount);
            Np_total += Np;
            os << (*it)->timos_str(Np) << endl;
        }
        os << s.back()->timos_str(packetcount-Np_total);
    }
    else {
        for(vector<SourceDescription*>::const_iterator it=s.begin(); it != s.end(); ++it)
            os << (*it)->timos_str() << endl;
    }

    return os.good();
}

// writes out TIM-OS compatible materials defs
bool writeTIMOSMaterials(string fn,const vector<Material>& mat)
{
    ofstream os(fn.c_str(),ios_base::out);

    if (!os.good())
        return false;

    os << 1 << endl << mat.size()-1 << endl;

    for(vector<Material>::const_iterator it=mat.begin()+1; it != mat.end(); ++it)
    {
        if (it->getg() == 1.0)
            os << "0.001 1 1 " << it->getn() << endl;
        else
            os << it->getMuA() << ' ' << it->getMuS() << ' ' << it->getg() << ' ' << it->getn() << endl;
    }

    if(mat[0].isMatched())
        os << 2 << endl;
    else
        os << 1 << endl << mat[0].getn() << endl;

    return os.good();
}

// Reads a TIM-OS results file (result.dat) - expects surface results first, then volume results
void readTIMOSOutput(string fn,const TetraMesh& mesh,SurfaceFluenceMap& surf,VolumeFluenceMap& vol)
{
    unsigned dtype,Ns,Nd;
    LineFile lf(fn,'%',cerr);

    surf.clear();
    vol.clear();

    while(!lf.eof())
    {
        lf >> dtype >> Nd >> Ns >> LineFile::LF_EOL;
        cout << "Reading data segment of type " << dtype << " with " << Nd << " points and " << Ns << " time steps" << endl;
        if (dtype == 1)
            readTIMOSOutputFlatten(mesh,surf,lf,Nd,Ns);
        else if (dtype == 2)
            readTIMOSOutputFlatten(mesh,vol,lf,Nd,Ns);
        else
            cerr << "Unexpected output format" << endl;
    }
}

// Read and flatten (sum across time steps for each element)
// Template parameter T specifies volume or surface element (either FaceByPointID or TetraByPointID)

// Volume total energy gives (fluence F) F*mu_a
template<class T>double readTIMOSOutputFlatten(const TetraMesh& mesh,FluenceMap<T>& F,LineFile& is,unsigned Nd,unsigned Ns)
{
    T IDps;
    F.clear();

    // read data lines and sum energy across all timesteps
    // IDp IDp IDp A f[Ns]
    double f, f_t, A, E_sum=0.0;
    for(unsigned i=0;i<Nd;++i)
    {
        is >> IDps >> A;
        if (!IDps.isSorted())
            cerr << "ERROR: Unsorted IDs in readTIMOSOutputFlatten" << endl;
        assert(IDps.isSorted());

        f=0;
        for(unsigned t=0;t<Ns;++t)
        {
            is >> f_t;
            f += f_t;
        }
        is >> LineFile::LF_EOL;
        E_sum += f*A;

        if (f != 0)
            F[IDps] = f;
    }
    return E_sum;
}
