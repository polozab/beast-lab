/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _JACOBI_H_
#define _JACOBI_H_

#include "Definitions.h"
#include <algorithm>

class Jacobi
{
public:
  Jacobi(const unsigned int nx, const unsigned int ny)
      : _nx(nx), _ny(ny), _X(getX(nx, ny)), _Y(getY(nx, ny)), _RHS(getRHS(nx, ny)) {}
  ~Jacobi() {}

  
  void iterate(const FLOAT *const __restrict readField,
             FLOAT *const __restrict writeField,
             const FLOAT *const __restrict rhs) const
{

  const unsigned int TILE_X = 512;
  const unsigned int stride = _nx + 2;

  const unsigned long long work = (unsigned long long)_nx * (unsigned long long)_ny;

  const FLOAT X   = _X;
  const FLOAT Y   = _Y;
  const FLOAT RHS = _RHS;

  
    #pragma omp parallel for if (_nx * _ny >= 196608) schedule(dynamic, 64)
  for (int yi = 1; yi <= (int)_ny; yi += 2)
  {
    const unsigned int y = (unsigned int)yi;
    const bool hasRow2 = (y + 1u <= _ny);
    for (unsigned int xBlock = 1; xBlock <= _nx; xBlock += TILE_X)
    {
      const unsigned int xEnd = std::min(_nx, xBlock + TILE_X - 1);

      {
        const FLOAT *c   = readField   + y * stride + xBlock;
        const FLOAT *s   = c - stride;                 // y-1
        const FLOAT *n   = c + stride;                 // y+1
        const FLOAT *r   = rhs        + y * stride + xBlock;
        FLOAT *out       = writeField + y * stride + xBlock;

        #pragma omp simd
        for (unsigned int x = xBlock; x <= xEnd; ++x)
        {
          *out = RHS * (*r)
               + X * (c[-1] + c[1])
               + Y * (s[0]  + n[0]);
          ++c; ++s; ++n; ++r; ++out;
        }
      }

      if (hasRow2)
      {
        const unsigned int y2 = y + 1u;

        const FLOAT *c   = readField   + y2 * stride + xBlock;
        const FLOAT *s   = c - stride;                // y
        const FLOAT *n   = c + stride;                // y+2
        const FLOAT *r   = rhs        + y2 * stride + xBlock;
        FLOAT *out       = writeField + y2 * stride + xBlock;

        #pragma omp simd
        for (unsigned int x = xBlock; x <= xEnd; ++x)
        {
          *out = RHS * (*r)
               + X * (c[-1] + c[1])
               + Y * (s[0]  + n[0]);
          ++c; ++s; ++n; ++r; ++out;
        }
      }
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
