/*
 * TIMOS.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#include <FullMonte/Geometry/Sources/PointSource.hpp>
#include <FullMonte/Geometry/Sources/Volume.hpp>
#include <FullMonte/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonte/Geometry/Sources/PencilBeam.hpp>
#include "TIMOS.hpp"

Source::Base*	TIMOS::convertToSource(const TIMOS::SourceDef tSrc)
{
	Source::Base* src=nullptr;

	switch(tSrc.type)
	{
	case TIMOS::SourceDef::Types::Volume:
		src = new Source::Volume(tSrc.w,tSrc.details.vol.tetID);
		break;

	case TIMOS::SourceDef::Types::PencilBeam:
		src = new Source::PencilBeam(
					tSrc.w,
					tSrc.details.pencilbeam.pos,
					tSrc.details.pencilbeam.dir,
					tSrc.details.pencilbeam.tetID);
		break;

	case TIMOS::SourceDef::Types::Face:
		src = new Source::SurfaceTri(
				tSrc.w,
				tSrc.details.face.IDps);
		break;

	case TIMOS::SourceDef::Types::Point:
		src = new Source::PointSource(
				tSrc.w,
				tSrc.details.point.pos);
		break;

	default:
		throw std::logic_error("TIMOSReader::convertToSource - invalid source type");
	}

	return src;
}

TIMOS::SourceDef TIMOS::convertFromSource(const Source::Base* src)
{
	TIMOS::SourceDef tSrc;

	tSrc.w=src->power();

	if (const Source::PencilBeam *pb = dynamic_cast<const Source::PencilBeam*>(src))
	{
		tSrc.type=TIMOS::SourceDef::Types::PencilBeam;
		tSrc.details.pencilbeam.pos = pb->position();
		tSrc.details.pencilbeam.dir = pb->direction();
	}
	else if (const Source::SurfaceTri *face = dynamic_cast<const Source::SurfaceTri*>(src))
	{
		tSrc.type=TIMOS::SourceDef::Types::Face;
		tSrc.details.face.IDps = face->triPointIDs();
	}
	else if (const Source::Volume *vol = dynamic_cast<const Source::Volume*>(src))
	{
		tSrc.type=TIMOS::SourceDef::Types::Volume;
		tSrc.details.vol.tetID = vol->elementID();
	}
	else if (const Source::PointSource *pt = dynamic_cast<const Source::PointSource*>(src))
	{
		tSrc.type=TIMOS::SourceDef::Types::Point;
		tSrc.details.point.pos = pt->position();
	}
	else
		throw std::logic_error("TIMOSReader::convertFromSource - invalid source type");
	return tSrc;
}



