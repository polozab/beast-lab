/********************************************************************
 * Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 *
 * With explicit permission to use in the BEAST lab WS 20/21
 *******************************************************************/

#include "Multigrid.h"
#include "ComputeError.h"
#include "Dispatch.h"

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    std::cout << "ERROR main: Please call program by"
              << std::endl
              << " ./multigrid preSmoothingSteps postSmoothingSteps numberGridLevels numberMultigridIterations"
              << std::endl;
    return -1;
  }

  const unsigned int preSmoothing = (unsigned int)atoi(argv[1]);
  const unsigned int postSmoothing = (unsigned int)atoi(argv[2]);
  const unsigned int jacobiStepsCoarsestLevel = 1;
  const unsigned int levels = (unsigned int)atoi(argv[3]);
  const unsigned int multigridCycles = (unsigned int)atoi(argv[4]);

  // determine resolution
  const unsigned int nx = ((unsigned int)(1 << levels)) - 1;

  // initialise fields and set boundary values
  FLOAT *field1 = new FLOAT[(nx + 2) * (nx + 2)];
  if (field1 == NULL)
  {
    std::cout << "ERROR field1==NULL!" << std::endl;
    return -1;
  }
  FLOAT *field2 = new FLOAT[(nx + 2) * (nx + 2)];
  if (field2 == NULL)
  {
    std::cout << "ERROR field2==NULL!" << std::endl;
    return -1;
  }
  FLOAT *rhs = new FLOAT[(nx + 2) * (nx + 2)];
  if (rhs == NULL)
  {
    std::cout << "ERROR rhs   ==NULL!" << std::endl;
    return -1;
  }

  const SetBoundary setBoundary(nx, nx);
  for (unsigned int i = 0; i < (nx + 2) * (nx + 2); i++)
  {
    field1[i] = 0.0;
    field2[i] = 0.0;
    rhs[i] = 0.0;
  }
  setBoundary.iterate(field1);
  setBoundary.iterate(field2);

  // Template dispatch so we can distinguish different calls

  switch (levels) {
    case 0: { dispatch<0>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 1: { dispatch<1>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 2: { dispatch<2>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 3: { dispatch<3>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 4: { dispatch<4>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 5: { dispatch<5>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 6: { dispatch<6>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 7: { dispatch<7>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 8: { dispatch<8>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 9: { dispatch<9>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 10:{ dispatch<10>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 11:{ dispatch<11>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 12:{ dispatch<12>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 13:{ dispatch<13>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 14:{ dispatch<14>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 15:{ dispatch<15>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    case 16:{ dispatch<16>(preSmoothing, postSmoothing, jacobiStepsCoarsestLevel, levels, multigridCycles, field1, field2, rhs); }
    default: {
      std::cout << "ERROR unhandled level";
      exit(EXIT_FAILURE);
    }
  }

  return 0;
}
