/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _PROLONGATION_H_
#define _PROLONGATION_H_

#include "Definitions.h"
#include <iostream>

// standard prolongation algorithm.
class Prolongation
{
public:
  Prolongation(const unsigned int nxCoarseGrid, const unsigned int nyCoarseGrid)
      : _nxCoarseGrid(nxCoarseGrid), _nyCoarseGrid(nyCoarseGrid),
        _nxFineGrid(2 * nxCoarseGrid + 1),
        _nyFineGrid(2 * nyCoarseGrid + 1)
  {}

  ~Prolongation() {}

  void interpolation(const FLOAT *const coarseGrid, FLOAT *const fineGrid) const
  {
    const unsigned int fineStride = _nxFineGrid + 2;
    const unsigned int coarseStride = _nxCoarseGrid + 2;

    #pragma omp parallel for schedule(static)
    for (unsigned int y = 1; y < _nyFineGrid + 1; ++y)
    {
      for (unsigned int x = 1; x < _nxFineGrid + 1; ++x)
      {
        const unsigned int fineI = y * fineStride + x;
        const unsigned int coarseI = y / 2 * coarseStride + x / 2;
        
        const FLOAT wy = (y & 1u) ? 0.5f : 1.0f;
        const FLOAT wx = (x & 1u) ? 0.5f : 1.0f;
        
        fineGrid[fineI] +=
            wx * wy * coarseGrid[coarseI] +
            (1.0f - wx) * wy * coarseGrid[coarseI + 1] +
            wx * (1.0f - wy) * coarseGrid[coarseI + coarseStride] +
            (1.0f - wx) * (1.0f - wy) * coarseGrid[coarseI + coarseStride + 1];
      }
    }
  }

private:
  // number of inner coarse grid points
  const unsigned int _nxCoarseGrid;
  const unsigned int _nyCoarseGrid;
  // number of inner fine grid points
  const unsigned int _nxFineGrid;
  const unsigned int _nyFineGrid;
};

#endif // _PROLONGATION_H_
