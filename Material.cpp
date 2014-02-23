#include "Material.hpp"

/*Material& Material::operator=(const Material& m)
{
	mu_s = m.mu_s;
	mu_a = m.mu_a;
	mu_p = m.mu_p;
	mu_t = m.mu_t;
	n = m.n;
	albedo = m.albedo;
	absfrac = m.absfrac;
	matchedboundary = m.matchedboundary;
	isscattering = m.isscattering;
	setG(m.hgparams.g);
	s_prop = m.s_prop;
	s_init = m.s_init;
	return *this;
}*/

ostream& operator<<(ostream& os,const Material& mat)
{
    return os << "mu_a=" << mat.mu_a << " mu_s=" << mat.mu_s << " g=" << mat.hgparams.g << " n=" << mat.n;
}
