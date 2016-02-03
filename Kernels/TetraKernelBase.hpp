/*
 * TetraKernelBase.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_TETRAKERNELBASE_HPP_
#define KERNELS_TETRAKERNELBASE_HPP_

class TetraMesh;

class TetraKernelBase
{
public:
	TetraKernelBase(const TetraMesh* mesh=nullptr) :
		m_mesh(mesh)
	{}

	void				mesh(const TetraMesh* M)			{ m_mesh=M;			}
	const TetraMesh*	mesh()						const	{ return m_mesh;	}

protected:
	const TetraMesh*	m_mesh=nullptr;
};


#endif /* KERNELS_TETRAKERNELBASE_HPP_ */
