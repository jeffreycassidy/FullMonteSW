/*
 * VectorTest.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_VECTORTEST_HPP_
#define GEOMETRY_VECTORTEST_HPP_

#define CHECK_VECTOR_CLOSE(lhs,rhs,eps)		BOOST_CHECK_SMALL(norm(lhs-rhs),eps)
#define CHECK_UNIT(v,eps)					BOOST_CHECK_SMALL(norm(v)-1,eps)
#define CHECK_ORTHOGONAL(lhs,rhs,eps)		BOOST_CHECK_SMALL(dot(lhs,rhs),eps)

#define CHECK_ORTHONORMAL(a,b,c,eps);			\
	CHECK_UNIT(a,eps);							\
	CHECK_UNIT(b,eps);							\
	CHECK_UNIT(c,eps);							\
	CHECK_ORTHOGONAL(a,b,eps);					\
	CHECK_ORTHOGONAL(a,c,eps);					\
	CHECK_ORTHOGONAL(b,c,eps);



#endif /* GEOMETRY_VECTORTEST_HPP_ */
