/*
 * Disk.hpp
 *
 *  Created on: Aug 18, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_DISK_HPP_
#define GEOMETRY_SOURCES_DISK_HPP_

class Disk
{
public:

	void diameter(float d);
	void radius(float r);

	float diameter() const;
	float radius() const;

private:
	float m_radius=0.0f;
};



#endif /* GEOMETRY_SOURCES_DISK_HPP_ */
