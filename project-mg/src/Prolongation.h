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

    #pragma omp parallel for
    for (unsigned int y = 1; y < _nyCoarseGrid + 1; y++)
    {
      
      for (unsigned int x = 1; x < _nxCoarseGrid + 1; x++)
      {
        // the points on the "diagonal" contribute to 0.25 to the error
        // interpolation (since 4 values are used for bilinear interpolation of the mid-point).
        // for the central point (fineI), we directly use the corresponding value.
        // for the other points, the bi-linear interpolation reduces to averaging
        // the two closest values (e.g. South- and North-values)
        
        const unsigned int fineRow = y * 2 * fineStride;
        const unsigned int coarseRow = y * coarseStride;

        const unsigned int fineI = fineRow + x * 2;
        const unsigned int coarseI = coarseRow + x;

        const FLOAT coarseVal = coarseGrid[coarseI];

        fineGrid[fineI - fineStride - 1] += 0.25 * coarseVal;
        fineGrid[fineI - fineStride] += 0.5 * coarseVal;
        fineGrid[fineI - fineStride + 1] += 0.25 * coarseVal;
        fineGrid[fineI - 1] += 0.5 * coarseVal;
        fineGrid[fineI] += coarseVal;
        fineGrid[fineI + 1] += 0.5 * coarseVal;
        fineGrid[fineI + fineStride - 1] += 0.25 * coarseVal;
        fineGrid[fineI + fineStride] += 0.5 * coarseVal;
        fineGrid[fineI + fineStride + 1] += 0.25 * coarseVal;
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
