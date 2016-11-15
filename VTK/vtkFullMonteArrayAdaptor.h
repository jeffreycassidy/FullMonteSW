/*
x * vtkFullMonteFieldAdaptor.h
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
	void source(const OutputData* D);

	/// Update the values from the source map
	void update();

	/// Collect the result (note: this is a single copy owned by this class; callers should not modify the returned pointer)
	vtkAbstractArray* array();

protected:
	vtkFullMonteArrayAdaptor();
	~vtkFullMonteArrayAdaptor();

private:
	const OutputData* m_fullMonteArray=nullptr;
	vtkAbstractArray* m_vtkArray=nullptr;
};


#endif /* VTK_VTKFULLMONTEARRAYADAPTOR_H_ */
