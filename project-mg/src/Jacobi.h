/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _JACOBI_H_
#define _JACOBI_H_

#include "Definitions.h"
#include <algorithm>
#include <algorithm>

class Jacobi
{
public:
  Jacobi(const unsigned int nx, const unsigned int ny)
      : _nx(nx), _ny(ny), _X(getX(nx, ny)), _Y(getY(nx, ny)), _RHS(getRHS(nx, ny)) {}
  ~Jacobi() {}

  template <IterationType IterationType>
  void iterate(const FLOAT *const readField, FLOAT *const writeField, const FLOAT *const rhs) const
  {
    const unsigned int stride = _nx + 2;

    #pragma omp parallel for if (_nx * _ny >= 196608) schedule(dynamic, 64)
    for (unsigned int y = 1; y < _ny + 1; y++)
    {
      for (unsigned int x = 1; x < _nx + 1; x++)
      {
        const unsigned int i = y * stride + x;
        writeField[i] = _RHS * rhs[i];
        writeField[i] += _X * (readField[i - 1] + readField[i + 1]);
        writeField[i] += _Y * (readField[i - stride] + readField[i + stride]);
      }
    }
  }

private:
  FLOAT getX(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return hy * hy / (2.0 * (hx * hx + hy * hy));}
  FLOAT getY(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return hx * hx / (2.0 * (hx * hx + hy * hy));
  }
  FLOAT getRHS(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return -1.0 / (2.0 / hx / hx + 2.0 / hy / hy);
  }

  const unsigned int _nx;
  const unsigned int _ny;
  const FLOAT _X;
  const FLOAT _Y;
  const FLOAT _RHS;
};

#endif // _JACOBI_H_
