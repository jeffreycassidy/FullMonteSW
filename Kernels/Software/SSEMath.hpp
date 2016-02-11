#ifndef SSEMATH_INCLUDED
#define SSEMATH_INCLUDED

#include <array>
#include <limits>

#include <smmintrin.h>

#include <cassert>
#include <boost/static_assert.hpp>
#include <boost/range/algorithm.hpp>

#include <cmath>

namespace SSE
{

class SSEBase
{
protected:
	SSEBase(){}
	SSEBase(__m128 v) : m_v(v){}

public:
	__m128 m_v;

	SSEBase abs() const
	{
		__m128i sign = _mm_set1_epi32(0x7fffffff);
		return _mm_and_ps(__m128(sign),m_v);
	}


	explicit operator __m128() const { return m_v; }

	static SSEBase undef()	{ return SSEBase(_mm_set1_ps(std::numeric_limits<float>::quiet_NaN())); 	}
	static SSEBase zero() 	{ return SSEBase(_mm_setzero_ps()); 										}
	static SSEBase one()	{ return SSEBase(_mm_set_ss(1.0f)); }
	static SSEBase ones()	{ return SSEBase(_mm_set1_ps(1.0f)); }
};

class Scalar : public SSEBase
{
public:
	explicit Scalar(__m128 s) 	: SSEBase(s){}
	Scalar() : SSEBase(_mm_setzero_ps()){}

	/// Fill a Scalar using the bottom element (useful when extending the output of _ss functions)
	static Scalar fill(__m128 s)
	{
		return Scalar(_mm_shuffle_ps(s,s,_MM_SHUFFLE(0,0,0,0)));
	}

	explicit Scalar(float s) 	: SSEBase(_mm_set1_ps(s)){}

	explicit operator float() const
	{
		return _mm_cvtss_f32(m_v);
	}

	static Scalar sqrt(Scalar s)
	{
		return Scalar(_mm_sqrt_ps(s.m_v));
	}

protected:
	Scalar(SSEBase s) : SSEBase(s){}
};

template<std::size_t D>class Vector : public SSEBase
{

public:
	Vector(){}
	explicit Vector(__m128 v) : SSEBase(v){}

	template<typename T>explicit Vector(std::array<T,D> a,
			typename std::enable_if< std::is_floating_point<T>::value, int>::type=0)
	{
		float f[4];
		unsigned i;
		for(i=0;i<D;++i)
			f[i]=a[i];
		for(;i<4;++i)
			f[i]=0.0f;
		m_v = _mm_load_ps(f);
	}

	/** Basic arithmetic */

	Vector operator-(const Vector rhs) const
	{
		return Vector(_mm_sub_ps(m_v,rhs.m_v));
	}

	Vector operator+(const Vector rhs) const
	{
		return Vector(_mm_add_ps(m_v,rhs.m_v));
	}

	Vector operator*(const Scalar rhs) const
	{
		return Vector(_mm_mul_ps(m_v,rhs.m_v));
	}

	Vector operator/(const Scalar rhs) const
	{
		return Vector(_mm_div_ps(m_v,rhs.m_v));
	}

	static Scalar dot(Vector lhs,Vector rhs)
	{
		// for i [0,4)
		// 	mask[i+4] == 1 -> include lhs[i]*rhs[i]
		// 	mask[i]   == 1 -> o[i] = dot(lhs,rhs)
		return Scalar(_mm_dp_ps(lhs.m_v,rhs.m_v,(mask<<4) | 0x7));
	}

	std::array<float,D> array() const
	{
		float f[4];
		_mm_store_ps(f,m_v);
		return *reinterpret_cast<std::array<float,3>*>(f);
	}

	/// Returns the index of the smallest element in terms of absolute value
	unsigned indexOfSmallestElement() const
	{
		const std::array<float,D> d = array();
		std::array<float,D> du;

		boost::transform(d, du.begin(), [](float i){ return std::abs(i); });
		return boost::min_element(d)-d.begin();
	}

	/// Return i'th component
	template<unsigned i>Scalar component()
	{
		BOOST_STATIC_ASSERT(i < D);
		return Scalar(_mm_shuffle_ps(m_v,m_v,_MM_SHUFFLE(i,i,i,i)));
	}


	static Vector undef(){ return SSEBase::undef(); }
	static Vector zero() { return SSEBase::zero(); 	}

protected:
	explicit Vector(SSEBase v) : SSEBase(v){}
	static constexpr int 	mask = (1<<D)-1;			// Mask indicating which elements are occupied

	friend Vector<3> cross(Vector<3>,Vector<3>);
};



using Vector3 = Vector<3>;
using Vector2 = Vector<2>;

// u = (a,b,c)   v = (d,e,f)
// cross product is (bf-ce, cd-af, ae-bd)

inline Vector3 cross(Vector3 u,Vector3 v)
{
    __m128 bca = _mm_shuffle_ps(u.m_v,u.m_v,_MM_SHUFFLE(3,0,2,1));
    __m128 fde = _mm_shuffle_ps(v.m_v,v.m_v,_MM_SHUFFLE(3,1,0,2));

    __m128 bf_cd_ae = _mm_mul_ps(bca,fde);

    __m128 cab = _mm_shuffle_ps(u.m_v,u.m_v,_MM_SHUFFLE(3,1,0,2));
    __m128 efd = _mm_shuffle_ps(v.m_v,v.m_v,_MM_SHUFFLE(3,0,2,1));

    __m128 ec_af_bd = _mm_mul_ps(efd,cab);

    return Vector3(_mm_sub_ps(bf_cd_ae,ec_af_bd));
}

template<std::size_t D>inline Scalar norm2(Vector<D> v)
{
	return dot(v,v);
}

template<std::size_t D>inline Scalar norm(Vector<D> v)
{
	return sqrt(dot(v,v));
}

template<std::size_t D>inline Scalar dot(Vector<D> lhs,Vector<D> rhs)
{
	return Vector<D>::dot(lhs,rhs);
}


enum Checking { NoCheck, Assert, Except, Normalize };

template<std::size_t D>class UnitVector : public Vector<D>
{
public:

	template<typename FT>UnitVector(std::array<FT,D> a,Checking c) :
		Vector<D>(a)
		{
			check(c);
		}

	explicit UnitVector() : Vector<D>(SSEBase::undef()){}

	UnitVector(Vector<D> v,Checking c) : Vector<D>(v)
	{
		check(c);
	}

	using Vector<D>::operator+;
	using Vector<D>::operator-;
	using Vector<D>::operator*;
	using Vector<D>::operator/;

	using Vector<D>::dot;

	static UnitVector normalize(Vector<D> v)
	{
		Scalar norm2 = dot(v,v);

		__m128 k = _mm_sqrt_ss(norm2.m_v);

		return _mm_div_ps(v.m_v,_mm_shuffle_ps(k,k,_MM_SHUFFLE(0,0,0,0)));
	}

	static UnitVector normalize_approx(Vector<D> v)
	{
		Scalar norm2 = dot(v,v);

		__m128 k = _mm_rsqrt_ss(norm2.m_v);

		return _mm_mul_ps(_mm_shuffle_ps(k,k,_MM_SHUFFLE(0,0,0,0)),v.m_v);
	}

	bool check(Checking c,float eps=1e-5)
	{
		float e;
		switch(c)
		{
		case Assert:
		case Except:
			e = std::abs(float(Scalar::sqrt(dot(*this,*this)))-1.0f);
			if (c==Except && e > eps)
				throw std::logic_error("Non-unit vector in UnitVector::check()");
			else if (c==Assert)
				assert(e < eps);
			return e<eps;

		case NoCheck:
			return true;

		case Normalize:
			*this = normalize(*this);
			return true;
		}
		return true;
	}


	/** Return the i'th basis vector (v[i]=1.0, v[j != i]=0.0) */
	template<unsigned i>static UnitVector basis()
	{
		__m128 one = _mm_set_ss(1.0f);

		BOOST_STATIC_ASSERT(i < D);

		return _mm_shuffle_ps(one,one,_MM_SHUFFLE(i!=3, i!=2, i!=1, i!=0));
	}

	static UnitVector basis(unsigned i)
	{
		float f[4]{0.0,0.0,0.0,0.0};
		f[i]=1.0f;
		assert(i < D);
		return _mm_load_ps(f);
	}

protected:
	UnitVector(__m128 v) : Vector<D>(v){}
	UnitVector(Vector<D> v) : Vector<D>(v){}		// protect because this waives checking or normalization
};

using UnitVector3 = UnitVector<3>;
using UnitVector2 = UnitVector<2>;

using Point3 = Vector<3>;
using Point2 = Vector<2>;



/** Returns a pair of vectors orthonormal to eachother and to v */

std::pair<UnitVector3,UnitVector3> normalsTo(UnitVector3 v);

template<std::size_t D>UnitVector<D> normalize(Vector<D> v)
{
	return UnitVector<D>::normalize(v);
}

};

#endif
