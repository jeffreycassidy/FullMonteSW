/*
 * Visitor.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_VISITOR_HPP_
#define GEOMETRY_SOURCES_VISITOR_HPP_

namespace Source
{

class PointSource;
class Ball;
class Line;
class Volume;
class Composite;
class Surface;
class SurfaceTri;
class Base;
class PencilBeam;

class Visitor
{
public:
	virtual void visit(PointSource* p)		=0;
	virtual void visit(Ball* b)			=0;
	virtual void visit(Line* l)			=0;
	virtual void visit(Volume* v)		=0;
	virtual void visit(Composite* c)	=0;
	virtual void visit(Surface* s)		=0;
	virtual void visit(SurfaceTri* st)	=0;
	virtual void visit(Base* b)			=0;
	virtual void visit(PencilBeam* b)	=0;
};

};




#endif /* GEOMETRY_SOURCES_VISITOR_HPP_ */
