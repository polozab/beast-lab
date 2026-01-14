/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _SETBOUNDARY_H_
#define _SETBOUNDARY_H_

#include "Definitions.h"

// set boundary conditions for function f(x,y) = x*y defined on unit square.
class SetBoundary
{
public:
  SetBoundary(const unsigned int nx, const unsigned int ny)
      : _nx(nx), _ny(ny), _meshsize2(1.0 / (nx + 1) / (ny + 1)) {}
  ~SetBoundary() {}

  void iterate(FLOAT *const field) const
{
    const unsigned int stride = _nx + 2;
    const unsigned int totalHeight = _ny + 1;
    const unsigned int totalWidth = _nx + 1;
    const FLOAT meshsize2 = _meshsize2;
    
    // Lower boundary
    #pragma omp simd
    for (unsigned int x = 0; x < stride; x++)
    {
        field[x] = 0.0; 
    }
    
    // Upper boundary
    FLOAT *upperRow = field + stride * totalHeight;
    const FLOAT height_factor = totalHeight * meshsize2;
    
    #pragma omp simd
    for (unsigned int x = 0; x < stride; x++)
    {
        upperRow[x] = x * height_factor;
    }
    
    // Left and right boundaries - compute in parallel
    #pragma omp parallel for if (_ny >= 1024)
    for (unsigned int y = 1; y < totalHeight; y++)
    {
        const unsigned int rowStart = y * stride;
        const FLOAT y_factor = y * meshsize2;
        
        field[rowStart] = 0.0;  // Left
        field[rowStart + totalWidth] = totalWidth * y_factor;  // Right
    }
}

private:
  const unsigned int _nx;
  const unsigned int _ny;
  const FLOAT _meshsize2;
};

#endif // _SETBOUNDARY_H_
