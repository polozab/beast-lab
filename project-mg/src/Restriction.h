/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _RESTRICTION_H_
#define _RESTRICTION_H_

#include "Definitions.h"

// carries out restriction by injection; we assume looping over the coarse
// grid and reading from the fine grid
class Restriction
{
public:
  Restriction(const unsigned int nxCoarseGrid, const unsigned int nyCoarseGrid)
      : _nxCoarseGrid(nxCoarseGrid), _nyCoarseGrid(nyCoarseGrid),
        _nxFineGrid(2 * nxCoarseGrid + 1),
        _nyFineGrid(2 * nyCoarseGrid + 1),
        _hxFineGrid2((1.0 / (2.0 * (nxCoarseGrid + 1))) * (1.0 / (2.0 * (nxCoarseGrid + 1)))),
        _hyFineGrid2((1.0 / (2.0 * (nyCoarseGrid + 1))) * (1.0 / (2.0 * (nyCoarseGrid + 1))))
  {}

  ~Restriction() {}

  void injection(const FLOAT *const fineGrid, const FLOAT *const rhsFineGrid, FLOAT *const rhsCoarseGrid, FLOAT *const coarseGridValues1, FLOAT *const coarseGridValues2) const
  {
    const unsigned int fineStride = _nxFineGrid + 2;
    const unsigned int coarseStride = _nxCoarseGrid + 2;

    #pragma omp parallel for if (_nxCoarseGrid * _nyCoarseGrid >= 196608) collapse(2) schedule(static)
    for (unsigned int y = 1; y < _nyCoarseGrid + 1; y++)
    {
      for (unsigned int x = 1; x < _nxCoarseGrid + 1; x++)
      {
          // do injection of error: therefore, compute residual on fine grid
          // and write the value into the coarse grid right hand side
          const unsigned int fineI = y * 2 * fineStride + x * 2;
          const unsigned int coarseI = y * coarseStride + x;

          rhsCoarseGrid[coarseI] = rhsFineGrid[fineI];
          rhsCoarseGrid[coarseI] -= (fineGrid[fineI - 1] - 2.0 * fineGrid[fineI] + fineGrid[fineI + 1]) / _hxFineGrid2;
          rhsCoarseGrid[coarseI] -= (fineGrid[fineI - fineStride] - 2.0 * fineGrid[fineI] + fineGrid[fineI + fineStride]) / _hyFineGrid2;

          coarseGridValues1[coarseI] = 0.0;
          coarseGridValues2[coarseI] = 0.0;
      }
    }
  }

private:
  // inner grid points on coarse grid
  const unsigned int _nxCoarseGrid;
  const unsigned int _nyCoarseGrid;
  // inner grid points on fine grid
  const unsigned int _nxFineGrid;
  const unsigned int _nyFineGrid;
  // squared mesh size of fine grid
  const FLOAT _hxFineGrid2;
  const FLOAT _hyFineGrid2;
};

#endif // _RESTRICTION_H_
