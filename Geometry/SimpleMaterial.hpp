/*
 * SimpleMaterial.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SIMPLEMATERIAL_HPP_
#define GEOMETRY_SIMPLEMATERIAL_HPP_

struct SimpleMaterial {
	float mu_a,mu_s,g,n;

	// albedo in terms of mu_s' (!= 1-absfrac)
	float albedo() const { float msp=mu_s_prime(); return (msp > 0 || mu_a > 0) ? msp/(msp+mu_a) : 0; }

	// reduced scattering coefficient
	float mu_s_prime() const { return mu_s*(1.0-g); }

	// fraction of energy absorbed at each interaction
	float absfrac() const { return (mu_s > 0 || mu_a > 0) ? 1-mu_s/(mu_s+mu_a) : 0; }
};

#endif /* GEOMETRY_SIMPLEMATERIAL_HPP_ */
