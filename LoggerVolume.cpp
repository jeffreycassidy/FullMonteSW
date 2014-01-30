#include "LoggerVolume.hpp"

typedef double FluenceCountType;

template<class T>ostream& operator<<(ostream& os,const LoggerVolume<T>&){ cout << "Volume logger report" << endl; return os; }

template<>void VolumeArray<double>::fluenceMap(VolumeFluenceMap& F,const vector<Material>& mat,bool per_volume)
{
    F.clear();
    map<unsigned,double>::const_iterator m_it=F.begin();
    unsigned i=1;

    for(vector<FluenceCountType>::const_iterator v_it=v.begin()+1; v_it != v.end(); ++v_it,++i)
    {
        if (*v_it != 0)
        {
            if(per_volume)
                F[i] = *v_it/mesh.getTetraVolume(i)/mat[mesh.getMaterial(i)].getMuA();
            else
                F[i] = *v_it;
        }
    }
}

template<>void VolumeArray<double>::hitMap(map<unsigned,unsigned long long>& m)
{
    m.clear();
    map<unsigned,unsigned long long>::iterator m_it=m.begin();
    unsigned IDt=1;
    if (v.size()<2)
        return;
    for(vector<FluenceCountType>::const_iterator it=v.begin()+1; it != v.end(); ++it,++IDt)
        if (*it != 0)
            m_it = m.insert(m_it,make_pair(IDt,*it));
}
