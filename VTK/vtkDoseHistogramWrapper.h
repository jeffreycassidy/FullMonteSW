/*
 * vtkFullMonteFieldAdaptor.h
 *
 *  Created on: Aug 12, 2016
 *      Author: jcassidy
 */

#ifndef VTK_vtkDoseHistogramWrapper_H_
#define VTK_vtkDoseHistogramWrapper_H_

#include <vtkObject.h>

#include <FullMonteSW/OutputTypes/OutputData.hpp>

class DoseHistogram;
class vtkFloatArray;
class vtkTable;

class vtkAbstractArray;

/** Converts an EmpiricalCDF<float,float> to
 *
 * Output ranges x [0,dose_max], y [0,1.0]
 *
 * EmpiricalCDF uses the statistical convention of F(x) = Pr(X<=x).
 * This class converts to the dose-histogram convention of plotting the proportion of tissue that receives a dose of
 * _at least_ x, by taking 1-F(x).
 *
 */

class vtkDoseHistogramWrapper : public vtkObject
{
public:
	vtkTypeMacro(vtkDoseHistogramWrapper,vtkObject)

	static vtkDoseHistogramWrapper* New();

	/// Change the source map
	void source(const char* mptr);
	void source(const OutputData* D);

	/// Update the values from the source map
	void update();

	/// Collect the result (note: this is a single copy owned by this class; callers should not modify the returned pointer)
	vtkTable* table() const;

protected:
	vtkDoseHistogramWrapper();
	~vtkDoseHistogramWrapper();

private:
	const DoseHistogram*	m_histogram=nullptr;
	vtkTable*				m_vtkTable;
};


#endif /* VTK_vtkDoseHistogramWrapper_H_ */
