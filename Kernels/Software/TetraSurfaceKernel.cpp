/*
 * TetraSurfaceKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Kernels/Software/TetraSurfaceKernel.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerTuple.hpp>
#include "TetraMCKernelThread.hpp"

#include <iostream>
using namespace std;

void TetraSurfaceKernel::prestart()
{
}

void TetraSurfaceKernel::postfinish()
{
	cout << "TetraSurfaceKernel is finished" << endl;

	cout << dec << fixed << setprecision(5);

	cout << "  Energy launched: " << conservationScorer().state().w_launch << endl;

	cout << "  SurfaceExitScorer stats" << endl;
	cout << "     Accumulations: " << surfaceScorer().accumulator().accumulationCount() << endl;
	cout << "       Expected exit count: " << eventScorer().state().Nexit << endl;
	cout << "     Retries:       " << surfaceScorer().accumulator().retryCount() << endl;

	unsigned Nf = mesh()->getNf()+1;

	cout << "     Total energy:  " << surfaceScorer().total() << endl;
	cout << "       Expected:    " << conservationScorer().state().w_exit << endl;

	cout << "       Face 0: " << surfaceScorer().accumulator()[Nf] << endl;
}

TetraSurfaceKernel::TetraSurfaceKernel(const TetraMesh* mesh) :
	TetraMCKernel<RNG,TetraSurfaceScorer>(mesh)
{
}
