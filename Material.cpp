#include "Material.hpp"

ostream& operator<<(ostream& os,const Material& mat)
{
    return os << "mu_a=" << mat.mu_a << " mu_s=" << mat.mu_s << " g=" << mat.hgparams.g << " n=" << mat.n;
}
