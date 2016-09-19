/*
 * vtkFullMonteFieldAdaptor.h
 *
 *  Created on: Aug 12, 2016
 *      Author: jcassidy
 */

#ifndef VTK_VTKFULLMONTEARRAYADAPTOR_H_
#define VTK_VTKFULLMONTEARRAYADAPTOR_H_

#include <vtkObject.h>

#include <FullMonteSW/OutputTypes/OutputData.hpp>

class vtkAbstractArray;

class vtkFullMonteArrayAdaptor : public vtkObject
{
public:
	vtkTypeMacro(vtkFullMonteArrayAdaptor,vtkObject)

	static vtkFullMonteArrayAdaptor* New();

	/// Change the source map
	void source(const char* mptr);
	void source(const VolumeAbsorbedEnergyDensityMap* E);
	void source(const VolumeFluenceMap* phi);
	void source(const SurfaceFluenceMap* phi);

	/// Update the values from the source map
	void update();

	/// Collect the result (note: this is a single copy owned by this class; callers should not modify the returned pointer)
	vtkAbstractArray* result();

protected:
	vtkFullMonteArrayAdaptor();
	~vtkFullMonteArrayAdaptor();

private:
	enum FieldType { SurfaceFluence, Fluence, Energy };

	FieldType	m_type=Fluence;
	const VolumeFluenceMap* m_fullMonteFluence=nullptr;
	const VolumeAbsorbedEnergyDensityMap* m_fullMonteArray=nullptr;
	const SurfaceFluenceMap* m_fullMonteFluenceS=nullptr;
	vtkAbstractArray* m_vtkArray=nullptr;
};


#endif /* VTK_VTKFULLMONTEARRAYADAPTOR_H_ */
