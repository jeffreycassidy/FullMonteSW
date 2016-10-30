#ifndef GEOMETRY_TETRAMESHBASE_INCLUDED_
#define GEOMETRY_TETRAMESHBASE_INCLUDED_

#include <vector>

#include "newgeom.hpp"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>

#include <boost/range/counting_range.hpp>

#include <boost/range/algorithm.hpp>

#ifndef SWIG
typedef struct {} points_tag;
constexpr points_tag points;

typedef struct {} point_coords_tag;
constexpr point_coords_tag point_coords;

typedef struct {} volume_tag;
constexpr volume_tag volume;

template<typename I>class WrappedInteger
{
public:
	WrappedInteger(){}
	explicit WrappedInteger(I v) : m_value(v){}

	explicit operator I() const { return m_value; }

	bool operator==(const WrappedInteger<I>& rhs) const { return rhs.m_value==m_value; }
	bool operator!=(const WrappedInteger<I>& rhs) const { return rhs.m_value!=m_value; }

	WrappedInteger& operator++(){ m_value++; return *this; }

	WrappedInteger& operator+=(std::ptrdiff_t rhs)					{ m_value += rhs; return *this; }
	WrappedInteger  operator+ (const WrappedInteger<I> rhs)	const 	{ return WrappedInteger(rhs.m_value + m_value); }
	std::ptrdiff_t	operator- (const WrappedInteger<I> rhs)			{ return m_value-rhs.m_value;}

	WrappedInteger& operator=(const WrappedInteger<I>&) = default;

	I value() const { return m_value; }

protected:
	I m_value=I();
};

#endif

/** TetraMeshBase contains the bare essentials of a tetrahedral mesh: points, tetras (by 4 point indices) and a region code for
 * each tetra. Generally tetra 0 and point 0 are dummy entries, to facilitate conversion between 0-based and 1-based indexing.
 * Likewise region 0 is a special code to indicate the exterior of the mesh.
 *
 */

template<typename FT,std::size_t D>class AffineTransform;

class TetraMeshBase
{
public:
	TetraMeshBase(){};

	TetraMeshBase(TetraMeshBase&& M) : m_points(std::move(M.m_points)),m_tetraPoints(std::move(M.m_tetraPoints)),m_tetraMaterials(std::move(M.m_tetraMaterials)){}
	TetraMeshBase(const TetraMeshBase& M) = default;
	TetraMeshBase& operator=(TetraMeshBase&&) = default;
	TetraMeshBase& operator=(const TetraMeshBase&) = default;

	TetraMeshBase(const std::vector<Point<3,double> >& P_,const std::vector<TetraByPointID>& T_p_,const std::vector<unsigned>& T_m_=std::vector<unsigned>())
		: m_points(P_),m_tetraPoints(T_p_),m_tetraMaterials(T_m_)
	{
		if(m_tetraMaterials.size() != m_tetraPoints.size())
			m_tetraMaterials.resize(m_tetraPoints.size(),0);
	}

	virtual void Delete(){ delete this; }
	virtual ~TetraMeshBase(){}

	const std::vector<TetraByPointID>& 	tetrasByID() const 	{ return m_tetraPoints; }

	void remapMaterial(unsigned from,unsigned to);
	std::vector<unsigned> tetraMaterialCount() const;

	unsigned getNp() const { return m_points.size()-1; };
	unsigned getNt() const { return m_tetraPoints.size()-1; };

	// Accessors for various point/face constructs
	const Point<3,double>&  getPoint(unsigned id)           const { return m_points[id]; }
	const TetraByPointID&   getTetraPointIDs(unsigned id)  	const { return m_tetraPoints[id]; }
    Point<3,double>         getTetraPoint(unsigned IDt,unsigned i) const { return m_points[m_tetraPoints[IDt][i]]; }

    const std::vector<Point<3,double> >& getPoints() const { return m_points; }
    const std::vector<TetraByPointID>& getTetrasByPointID() const { return m_tetraPoints; }

    unsigned getNumberOfRegions() const
    {
    	return *boost::max_element(m_tetraMaterials);
    }

    void apply(const AffineTransform<float,3>& T);




    unsigned                		getMaterial(unsigned IDt) const { return m_tetraMaterials[IDt]; }
    const std::vector<unsigned>& 	getMaterials() const { return m_tetraMaterials; }

    // adjust units
    float			cmPerLengthUnit() 			const 	{ return m_cmPerLengthUnit; }
    void			cmPerLengthUnit(float l)			{ m_cmPerLengthUnit=l;		}

    std::array<Point<3,double>,4> tetraPoints(unsigned IDt) const
    {
    	std::array<Point<3,double>,4> p;
    	for(unsigned i=0;i<4;++i)
    		p[i]=m_points[m_tetraPoints[IDt][i]];
    	return p;
    }

    class PointDescriptor;

    typedef boost::counting_iterator<
    		PointDescriptor,
			std::random_access_iterator_tag,
			std::ptrdiff_t>	PointIterator;

    typedef boost::iterator_range<PointIterator> 			PointRange;



    class TetraDescriptor;

    typedef boost::counting_iterator<
        			TetraDescriptor,
        			std::random_access_iterator_tag,
        			std::ptrdiff_t> TetraIterator;

    typedef  boost::iterator_range<TetraIterator> TetraRange;

#ifndef SWIG
    PointRange points() const;
#endif

protected:
	std::vector<Point<3,double> >    	m_points;          	///< Point vector
	std::vector<TetraByPointID>      	m_tetraPoints;    	///< Tetra -> 4 point IDs
	std::vector<unsigned>				m_tetraMaterials;	///< Tetra -> material

private:
	float m_cmPerLengthUnit=1.0f;			///< Scale parameter (1.0 -> cm, 0.1 -> mm)

	template<class Archive>void serialize(Archive& ar,const unsigned int version)
		{	ar & BOOST_SERIALIZATION_NVP(m_points) & BOOST_SERIALIZATION_NVP(m_tetraPoints) & BOOST_SERIALIZATION_NVP(m_tetraMaterials); }

	friend class boost::serialization::access;


    friend Point<3,double> 	get(point_coords_tag,	const TetraMeshBase&,PointDescriptor);
    friend TetraByPointID 	get(points_tag,			const TetraMeshBase&,TetraDescriptor);
};

class TetraMeshBase::PointDescriptor : public WrappedInteger<unsigned>
{
public:
	explicit PointDescriptor(unsigned i=0) : WrappedInteger<unsigned>(i){}

};

class TetraMeshBase::TetraDescriptor : public WrappedInteger<unsigned>
{
public:
	explicit TetraDescriptor(unsigned i=0) : WrappedInteger<unsigned>(i){}
};

#ifndef SWIG

inline TetraByPointID 	get(points_tag,			const TetraMeshBase& M,TetraMeshBase::TetraDescriptor IDt)	{ return M.m_tetraPoints[IDt.value()]; 	}
inline Point<3,double> 	get(point_coords_tag,	const TetraMeshBase& M,TetraMeshBase::PointDescriptor IDt)	{ return M.m_points[IDt.value()]; 		}

double get(volume_tag,const TetraMeshBase& M,TetraMeshBase::TetraDescriptor T);

/** Write instance for getting a property for all points of a tetra */

//template<typename Prop,typename Mesh>
//	std::array<decltype(get(std::declval<Prop>(), std::declval<const Mesh&>(), TetraMeshBase::PointDescriptor())),4>
//	get(Prop p,const Mesh& M,TetraByPointID IDps)
//	{
//		typedef decltype(get(p, M, TetraMeshBase::PointDescriptor())) Result;
//		std::array<Result,4> res;
//
//		for(unsigned i=0;i<4;++i)
//			res[i] = get(p,M,TetraMeshBase::PointDescriptor(IDps[i]));
//
//		return res;
//	}

template<typename Prop>std::array<decltype(get(std::declval<Prop>(),std::declval<const TetraMeshBase&>(),TetraMeshBase::PointDescriptor())),4>
	get(Prop p,const TetraMeshBase& M,TetraMeshBase::TetraDescriptor IDt)
	{
	typedef decltype(get(p, M, TetraMeshBase::PointDescriptor())) Result;
	TetraByPointID IDps = get(points,M,IDt);
	std::array<Result,4> res;

	for(unsigned i=0;i<4;++i)
		res[i] = get(p,M,TetraMeshBase::PointDescriptor(IDps[i]));

	return res;
	}

#endif

#endif
