/*
 * vtkFullMonteFluenceLineQueryWrapper.h
 *
 *  Created on: Mar 6, 2016
 *      Author: jcassidy
 */

#ifndef VTK_VTKFULLMONTEFLUENCELINEQUERYWRAPPER_H_
#define VTK_VTKFULLMONTEFLUENCELINEQUERYWRAPPER_H_

#include <vtkObject.h>

class FluenceLineQuery;
class vtkPolyData;

class vtkFullMonteFluenceLineQueryWrapper : public vtkObject
{
public:
	virtual ~vtkFullMonteFluenceLineQueryWrapper();

	static vtkFullMonteFluenceLineQueryWrapper* New();

	const FluenceLineQuery* fluenceLineQuery() const;
	void fluenceLineQuery(const FluenceLineQuery* q);

	void update();

	vtkPolyData* getPolyData() const;

protected:
	vtkFullMonteFluenceLineQueryWrapper();

private:
	float 						m_minSegmentLength=1e-5f;
	const FluenceLineQuery* 	m_fluenceLineQuery=nullptr;
	vtkPolyData*				m_vtkPD=nullptr;
};

#endif /* VTK_VTKFULLMONTEFLUENCELINEQUERYWRAPPER_H_ */
