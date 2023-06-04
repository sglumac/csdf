/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "buffer.h"

#include <stdlib.h>

CsdfBuffer *new_buffer(const CsdfConnection *connection, void *data,
                       CsdfBufferPush push, CsdfBufferPop pop,
                       CsdfBufferNumberOfTokens numberOfTokens)
{
    CsdfBuffer *buffer = malloc(sizeof(CsdfBuffer));
    buffer->connection = connection;
    buffer->data = data;
    buffer->pop = pop;
    buffer->push = push;
    buffer->numberOfTokens = numberOfTokens;
    return buffer;
}

void delete_buffer(CsdfBuffer *buffer)
{
    free(buffer);
}