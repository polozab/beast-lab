/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

// define type of floating point number to be used
#define FLOAT double

// define nvtx macros for creating and closing ranges
#include <nvtx3/nvToolsExt.h>
#define NVTX_RANGE_START(_rangeIdVar, _argbColor, _msgCstr)                 \
    nvtxRangeId_t _rangeIdVar = 0;                                          \
    do {                                                                    \
        nvtxEventAttributes_t _nvtx_evt = {0};                              \
        _nvtx_evt.version = NVTX_VERSION;                                   \
        _nvtx_evt.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;                     \
        _nvtx_evt.colorType = NVTX_COLOR_ARGB;                              \
        _nvtx_evt.color = (uint32_t)(_argbColor);                           \
        _nvtx_evt.messageType = NVTX_MESSAGE_TYPE_ASCII;                    \
        _nvtx_evt.message.ascii = (_msgCstr);                               \
        (_rangeIdVar) = nvtxRangeStartEx(&_nvtx_evt);                       \
    } while (0)

#define NVTX_RANGE_END(_rangeIdVar)                                         \
    do {                                                                    \
        nvtxRangeEnd((_rangeIdVar));                                        \
    } while (0)

#endif // _DEFINITIONS_H_
