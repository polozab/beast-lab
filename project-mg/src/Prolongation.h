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
    const unsigned int lastFineX = (_nxCoarseGrid << 1) + 1;
    const unsigned int lastFineY = (_nyCoarseGrid << 1) + 1;

    #pragma omp parallel
    {
      #pragma omp for
      for (unsigned int y = 1; y < _nyCoarseGrid + 1; y++)
      {
        const unsigned int fineRow = (y << 1) * fineStride;
        const unsigned int coarseRow = y * coarseStride;
        for (unsigned int x = 1; x < _nxCoarseGrid + 1; x++)
        {
          fineGrid[fineRow + (x << 1)] += coarseGrid[coarseRow + x];
        }
      }

      #pragma omp for
      for (unsigned int y = 1; y < _nyCoarseGrid + 1; y++)
      {
        const unsigned int fineRow = (y << 1) * fineStride;
        const unsigned int coarseRow = y * coarseStride;
        fineGrid[fineRow + 1] += 0.5 * coarseGrid[coarseRow + 1];
        for (unsigned int x = 1; x < _nxCoarseGrid; x++)
        {
          const unsigned int coarseI = coarseRow + x;
          fineGrid[fineRow + (x << 1) + 1] += 0.5 * (coarseGrid[coarseI] + coarseGrid[coarseI + 1]);
        }
        fineGrid[fineRow + lastFineX] += 0.5 * coarseGrid[coarseRow + _nxCoarseGrid];
      }

      #pragma omp for
      for (unsigned int y = 1; y < _nyCoarseGrid; y++)
      {
        const unsigned int fineRow = ((y << 1) + 1) * fineStride;
        const unsigned int coarseRow = y * coarseStride;
        const unsigned int coarseRowN = (y + 1) * coarseStride;
        for (unsigned int x = 1; x < _nxCoarseGrid + 1; x++)
        {
          const unsigned int coarseI = coarseRow + x;
          fineGrid[fineRow + (x << 1)] += 0.5 * (coarseGrid[coarseI] + coarseGrid[coarseRowN + x]);
        }
      }

      #pragma omp for
      for (unsigned int x = 1; x < _nxCoarseGrid + 1; x++)
      {
        const unsigned int fineX = x << 1;
        fineGrid[1 * fineStride + fineX] += 0.5 * coarseGrid[1 * coarseStride + x];
        fineGrid[lastFineY * fineStride + fineX] += 0.5 * coarseGrid[_nyCoarseGrid * coarseStride + x];
      }

      #pragma omp for
      for (unsigned int y = 1; y < _nyCoarseGrid; y++)
      {
        const unsigned int fineRow = ((y << 1) + 1) * fineStride;
        const unsigned int coarseRow = y * coarseStride;
        const unsigned int coarseRowN = (y + 1) * coarseStride;
        fineGrid[fineRow + 1] += 0.25 * (coarseGrid[coarseRow + 1] + coarseGrid[coarseRowN + 1]);
        for (unsigned int x = 1; x < _nxCoarseGrid; x++)
        {
          const unsigned int coarseI = coarseRow + x;
          fineGrid[fineRow + (x << 1) + 1] += 0.25 * (coarseGrid[coarseI] + coarseGrid[coarseI + 1] +
                                                      coarseGrid[coarseRowN + x] + coarseGrid[coarseRowN + x + 1]);
        }
        fineGrid[fineRow + lastFineX] += 0.25 * (coarseGrid[coarseRow + _nxCoarseGrid] +
                                                 coarseGrid[coarseRowN + _nxCoarseGrid]);
      }

      #pragma omp for
      for (unsigned int x = 1; x < _nxCoarseGrid; x++)
      {
        const unsigned int fineX = (x << 1) + 1;
        fineGrid[1 * fineStride + fineX] += 0.25 * (coarseGrid[1 * coarseStride + x] +
                                                    coarseGrid[1 * coarseStride + x + 1]);
        fineGrid[lastFineY * fineStride + fineX] += 0.25 * (coarseGrid[_nyCoarseGrid * coarseStride + x] +
                                                            coarseGrid[_nyCoarseGrid * coarseStride + x + 1]);
      }

      #pragma omp for
      for (unsigned int edge = 0; edge < 2; edge++)
      {
        const unsigned int fineRow = edge == 0 ? 1 * fineStride : lastFineY * fineStride;
        const unsigned int coarseRow = edge == 0 ? 1 * coarseStride : _nyCoarseGrid * coarseStride;
        fineGrid[fineRow + 1] += 0.25 * coarseGrid[coarseRow + 1];
        fineGrid[fineRow + lastFineX] += 0.25 * coarseGrid[coarseRow + _nxCoarseGrid];
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
