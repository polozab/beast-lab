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
    const unsigned int fineStride   = _nxFineGrid + 2;
    const unsigned int coarseStride = _nxCoarseGrid + 2;

    #pragma omp parallel for collapse(2) schedule(static)
    for (unsigned int yf = 1; yf <= _nyFineGrid; ++yf)
    {

      for (unsigned int xf = 1; xf <= _nxFineGrid; ++xf)
      {

        FLOAT* fRow = fineGrid + yf * fineStride;

        const unsigned int yc0 = yf >> 1;
        const unsigned int yc1 = yc0 + 1;
        const unsigned int yOdd = yf & 1u;

        const FLOAT wy0 = yOdd ? (FLOAT)0.5 : (FLOAT)1.0;
        const FLOAT wy1 = yOdd ? (FLOAT)0.5 : (FLOAT)0.0;

        const FLOAT* cRow0 = coarseGrid + yc0 * coarseStride;
        const FLOAT* cRow1 = coarseGrid + yc1 * coarseStride;

        const unsigned int xc0 = xf >> 1;
        const unsigned int xc1 = xc0 + 1;
        const unsigned int xOdd = xf & 1u;

        const FLOAT wx0 = xOdd ? (FLOAT)0.5 : (FLOAT)1.0;
        const FLOAT wx1 = xOdd ? (FLOAT)0.5 : (FLOAT)0.0;

        const FLOAT c00 = cRow0[xc0];
        const FLOAT c10 = cRow0[xc1];
        const FLOAT c01 = cRow1[xc0];
        const FLOAT c11 = cRow1[xc1];

        const FLOAT t0 = wx0 * c00 + wx1 * c10;
        const FLOAT t1 = wx0 * c01 + wx1 * c11;
        const FLOAT val = wy0 * t0 + wy1 * t1;

        fRow[xf] += val;
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
