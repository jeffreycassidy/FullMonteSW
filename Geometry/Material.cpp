#include "Material.hpp"
#include  <iostream>

/** Prints simple output format mu_a=XXX mu_s=XXX g=XXX n=XXX */
std::ostream& operator<<(std::ostream& os,const Material& mat)
{
    return os << "mu_a=" << mat.mu_a << " mu_s=" << mat.mu_s << " g=" << mat.hgparams.g << " n=" << mat.n <<
    		(mat.isMatched() ? " (matched boundary)" : " (normal)");
}
