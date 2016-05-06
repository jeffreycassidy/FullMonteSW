/*
 * TetraKernelBase.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_TETRAKERNELBASE_HPP_
#define KERNELS_TETRAKERNELBASE_HPP_

#include <immintrin.h>
#include <iostream>

class SimpleMaterial;
class TetraMesh;

class TetraKernelBase
{
public:
	TetraKernelBase(const TetraMesh* mesh=nullptr) :
		m_mesh(mesh)
	{}

	class Material;

	void				mesh(const TetraMesh* M)			{ m_mesh=M;			}
	const TetraMesh*	mesh()						const	{ return m_mesh;	}

protected:
	const TetraMesh*	m_mesh=nullptr;
};


/** Material representation for the tetra kernel.
 *
 */

#ifndef SWIG

class TetraKernelBase::Material
{
public:
	Material();
	Material(const SimpleMaterial& mat);
	Material(const Material&)=default;

	__m128 m_prop=_mm_setzero_ps();		///< [0] physical step, [1] dimensionless step, [2] time, [3] X
	__m128 m_init=_mm_setzero_ps();		///< Initial value of propagation vector

	float absorbedFraction() 	const 	{ return m_absfrac; 	}
	bool scatters() 			const 	{ return m_scatters; 	}

	float n() 					const 	{ return m_n; 			}
	float muT()					const 	{ return m_muT; 		}

private:
	static constexpr float c0 = 299.792458;		///< Speed of light in mm/ns
	float m_absfrac=0.0f;	///< Fraction absorbed at each interaction
	bool m_scatters=false;	///< Flag to indicate if the material scatters light or not

	float m_n=1.0f;
	float m_muT=0.0f;

	friend std::ostream& operator<<(std::ostream&,const Material& mat);
};

#endif

#endif /* KERNELS_TETRAKERNELBASE_HPP_ */
