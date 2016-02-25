#ifndef GEOMETRY_TETRAMESHBASE_INCLUDED_
#define GEOMETRY_TETRAMESHBASE_INCLUDED_

#include <vector>

#include "newgeom.hpp"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>

#include <boost/range/algorithm.hpp>


/** TetraMeshBase contains the bare essentials of a tetrahedral mesh: points, tetras (by 4 point indices) and a region code for
 * each tetra. Generally tetra 0 and point 0 are dummy entries, to facilitate conversion between 0-based and 1-based indexing.
 * Likewise region 0 is a special code to indicate the exterior of the mesh.
 *
 */

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

	const std::vector<Point<3,double>>& 	points() const 	 	{ return m_points; }
	const std::vector<TetraByPointID>& 	tetrasByID() const 	{ return m_tetraPoints; }

	void remapMaterial(unsigned from,unsigned to);
	std::vector<unsigned> tetraMaterialCount() const;

	unsigned getNp() const { return m_points.size()-1; };
	unsigned getNt() const { return m_tetraPoints.size()-1; };

	// Accessors for various point/face constructs
	const Point<3,double>&  getPoint(unsigned id)           const { return m_points[id]; }
	const TetraByPointID&   getTetraPointIDs(unsigned id)  	const { return m_tetraPoints[id]; }
    Point<3,double>         getTetraPoint(unsigned IDt,unsigned i) const { return m_points[m_tetraPoints[IDt][i]]; }

    double                  getTetraVolume(TetraByPointID IDps) const
        { return std::abs(scalartriple(m_points[IDps[0]],m_points[IDps[1]],m_points[IDps[2]],m_points[IDps[3]])/6); }

    double                  getTetraVolume(unsigned IDt) const { return getTetraVolume(m_tetraPoints[IDt]); }

    const std::vector<Point<3,double> >& getPoints() const { return m_points; }
    const std::vector<TetraByPointID>& getTetrasByPointID() const { return m_tetraPoints; }

    unsigned getNumberOfRegions() const
    {
    	return *boost::max_element(m_tetraMaterials);
    }


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

protected:
	std::vector<Point<3,double> >    	m_points;          	///< Point vector
	std::vector<TetraByPointID>      	m_tetraPoints;    	///< Tetra -> 4 point IDs
	std::vector<unsigned>				m_tetraMaterials;	///< Tetra -> material

private:
	float m_cmPerLengthUnit=1.0f;			///< Scale parameter (1.0 -> cm, 0.1 -> mm)

	template<class Archive>void serialize(Archive& ar,const unsigned int version)
		{	ar & BOOST_SERIALIZATION_NVP(m_points) & BOOST_SERIALIZATION_NVP(m_tetraPoints) & BOOST_SERIALIZATION_NVP(m_tetraMaterials); }

	friend class boost::serialization::access;
};

#endif
