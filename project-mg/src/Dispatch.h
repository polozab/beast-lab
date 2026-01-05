
#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include "Multigrid.h"
#include "ComputeError.h"

template <unsigned int Level>
void dispatch(
    const unsigned int nx,
    const unsigned int preSmoothing,
    const unsigned int postSmoothing,
    const unsigned int jacobiStepsCoarsestLevel,
    const unsigned int levels,
    const unsigned int multigridCycles,
    FLOAT *field1,
    FLOAT *field2,
    FLOAT *rhs)
{

  Multigrid<Level> multigrid(nx, nx, levels);
  FLOAT *currentSolution = field1;

  ComputeError computeError(nx, nx);
  const VTKPlotter vtkPlotter;

  for (unsigned int i = 0; i < multigridCycles; i++)
  {

    char msg[64];
    std::snprintf(msg, sizeof(msg), "Iteration %d", i);
    // NVTX_RANGE_START(i_range, 0xFF51555C, msg);

    // always work on the correct field (either field1 or field2);
    // this switch is required due to the Jacobi method
    if (currentSolution == field1)
    {
      currentSolution = multigrid.solve(
          preSmoothing, postSmoothing, jacobiStepsCoarsestLevel,
          field1, field2, rhs);
    }
    else
    {
      currentSolution = multigrid.solve(
          preSmoothing, postSmoothing, jacobiStepsCoarsestLevel,
          field2, field1, rhs);
    }

    computeError.computePointwiseError(currentSolution);
    std::cout << "Iteration " << i << ", Max-error: " << computeError.getMaxError() << std::endl;

    // NVTX_RANGE_END(i_range);
  }

  // commented out: result not interesting here
  // computeError.plotPointwiseError();
  // vtkPlotter.writeFieldData(currentSolution,nx,nx,"result.vtk");
}

#endif _DISPATCH_H_ // _DISPATCH_H_
