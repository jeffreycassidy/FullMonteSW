/*
 * SimpleMaterial.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SIMPLEMATERIAL_HPP_
#define GEOMETRY_SIMPLEMATERIAL_HPP_

/** Simple material description mu_s, mu_a, g, n
 */

class SimpleMaterial
{
public:
	SimpleMaterial(){}
	SimpleMaterial(float muS,float muA,float g,float n) :
		mu_a(muA),
		mu_s(muS),
		m_g(g),
		m_n(n){}

	float muS()	const	{ return mu_s; 				}
	float muA() const	{ return mu_a; 				}
	float muT() const 	{ return mu_a+mu_s; 		}
	float n() 	const	{ return m_n; 				}
	float g()	const	{ return m_g;				}

	/// Albedo in terms of mu_s' (!= 1-absfrac unless g==0.0)
	float albedo() const { float msp=mu_s_prime(); return (msp > 0 || mu_a > 0) ? msp/(msp+mu_a) : 1.0f; }

	/// Reduced scattering coefficient
	float mu_s_prime() const { return mu_s*(1.0-m_g); }

	/// Fraction of energy absorbed at each interaction
	float absfrac() const { return (mu_s > 0 || mu_a > 0) ? 1-mu_s/(mu_s+mu_a) : 0; }

private:
	/// Default material is air
	float mu_a=0.0f,mu_s=0.0f,m_g=1.0f,m_n=1.0f;
};

#endif /* GEOMETRY_SIMPLEMATERIAL_HPP_ */
