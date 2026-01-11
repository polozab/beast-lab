/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _JACOBI_H_
#define _JACOBI_H_

#include "Definitions.h"
#include <algorithm>

// carries out a jacobi step, reading from val and writing to _writeGrid.
class Jacobi
{
public:
  Jacobi(const unsigned int nx, const unsigned int ny)
      : _nx(nx), _ny(ny), _X(getX(nx, ny)), _Y(getY(nx, ny)), _RHS(getRHS(nx, ny)) {}
  ~Jacobi() {}

  void iterate(const FLOAT *const readField, FLOAT *const writeField, const FLOAT *const rhs) const
  {
    const unsigned int stride = _nx + 2;
    const int blockSize = 1800;

    #pragma omp parallel for if (_nx * _ny >= 196608) collapse(2) schedule(static)
    for (unsigned int yb = 1; yb < _ny + 1; yb += blockSize)
    {
      for (unsigned int xb = 1; xb < _nx + 1; xb += blockSize)
      {
        const unsigned int y_end = std::min(yb + blockSize, _ny + 1);
        const unsigned int x_end = std::min(xb + blockSize, _nx + 1);

        for (unsigned int y = yb; y < y_end; y++)
        {
          for (unsigned int x = xb; x < x_end; x++)
          {
            const unsigned int i = y * stride + x;
            writeField[i] = _RHS * rhs[i];
            writeField[i] += _X * (readField[i - 1] + readField[i + 1]);
            writeField[i] += _Y * (readField[i - stride] + readField[i + stride]);
          }
        }
      }
    }
  }

private:
  // returns the prefactor for the Jacobi stencil in x-direction
  FLOAT getX(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return hy * hy / (2.0 * (hx * hx + hy * hy));
  }
  // returns the prefactor for the Jacobi stencil in y-direction
  FLOAT getY(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return hx * hx / (2.0 * (hx * hx + hy * hy));
  }
  // returns the prefactor for the right hand side in Jacobi computation
  FLOAT getRHS(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return -1.0 / (2.0 / hx / hx + 2.0 / hy / hy);
  }

  // number of inner grid points
  const unsigned int _nx;
  const unsigned int _ny;
  // prefactors in Jacobi computations
  const FLOAT _X;
  const FLOAT _Y;
  const FLOAT _RHS;
};

#endif // _JACOBI_H_
