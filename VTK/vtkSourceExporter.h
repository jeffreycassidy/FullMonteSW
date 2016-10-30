/*
 * vtkSourceExporter.h
 *
 *  Created on: Oct 28, 2016
 *      Author: jcassidy
 */

#ifndef VTK_VTKSOURCEEXPORTER_H_
#define VTK_VTKSOURCEEXPORTER_H_

class TetraMesh;
class vtkUnstructuredGrid;

#include <vtkObject.h>
#include <FullMonteSW/Geometry/Sources/Abstract.hpp>

/** Converts one or more source descriptions into a VTK visualization. The resulting unstructured grid will have a
 * cell data field "Source number" that specifies the order in which the elements were added 0..(N-1).
 *
 * Composite: add all elements, with increasing ID number
 * Ball: uses vtkSphereSource based on (centre,radius), _not_ the actual tetras
 * Line, Point, Volume, Surface, SurfaceTri: as usual
 */

class vtkSourceExporter : public vtkObject
{
public:

	vtkTypeMacro(vtkSourceExporter,vtkObject)

	vtkSourceExporter();
	virtual ~vtkSourceExporter();

	static vtkSourceExporter* New();

	void source(Source::Abstract* S);

	/// NOTE: Takes Abstract or Composite at the moment, due to the need for manual casting
	/// TODO: Remove need for manual type checking
	void source(const char* s);
	Source::Abstract* source() const;

	vtkUnstructuredGrid* output() const;

	void mesh(const char* s);
	void mesh(const TetraMesh* M);

private:
	const TetraMesh* 	m_mesh=nullptr;
	Source::Abstract*	m_source=nullptr;
};




#endif /* VTK_VTKSOURCEEXPORTER_H_ */
