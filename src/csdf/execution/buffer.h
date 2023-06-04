/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_BUFFER_H
#define CSDF_EXECUTION_BUFFER_H

#include <csdf/graph.h>
#include <stdint.h>

typedef struct CsdfBuffer CsdfBuffer;

typedef void (*CsdfBufferPush)(CsdfBuffer *buffer, const uint8_t *token);
typedef void (*CsdfBufferPop)(CsdfBuffer *buffer, uint8_t *token);
typedef unsigned (*CsdfBufferNumberOfTokens)(CsdfBuffer *buffer);

struct CsdfBuffer
{
    const CsdfConnection *connection;
    void *data;
    CsdfBufferPush push;
    CsdfBufferPop pop;
    CsdfBufferNumberOfTokens numberOfTokens;
};

CsdfBuffer *new_buffer(const CsdfConnection *connection, void *data,
                       CsdfBufferPush push, CsdfBufferPop pop,
                       CsdfBufferNumberOfTokens numberOfTokens);

void delete_buffer(CsdfBuffer *buffer);

#endif // CSDF_EXECUTION_BUFFER_H
