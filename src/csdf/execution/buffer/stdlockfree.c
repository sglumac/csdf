/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "stdlockfree.h"

#include <stdlib.h>
#include <string.h>

typedef struct CsdfBufferStdLockFreeData
{
    size_t start;
    size_t end;
    size_t maxTokens;
    uint8_t *tokens;
} CsdfBufferStdLockFreeData;

static void buffer_push(CsdfBuffer *buffer, const uint8_t *token)
{
    CsdfBufferStdLockFreeData *data = buffer->data;
    size_t tokenSize = buffer->connection->tokenSize;
    uint8_t *bufferEnd = data->tokens + tokenSize * data->end;
    memcpy(bufferEnd, token, tokenSize);
    data->end = (data->end + 1) % data->maxTokens;
    if (data->end == data->start)
    {
        exit(-123);
    }
}

static void buffer_pop(CsdfBuffer *buffer, uint8_t *token)
{
    CsdfBufferStdLockFreeData *data = buffer->data;
    uint8_t *bufferStart = data->tokens + buffer->connection->tokenSize * data->start;
    data->start = (data->start + 1) % data->maxTokens;
    memcpy(token, bufferStart, buffer->connection->tokenSize);
}

static unsigned number_tokens(CsdfBuffer *buffer)
{
    const CsdfBufferStdLockFreeData *data = buffer->data;
    unsigned end = data->end;
    unsigned start = data->start;

    return end >= start
               ? end - start
               : end + data->maxTokens - start;
}

static CsdfBufferStdLockFreeData *new_stdlockfree_buffer_data(const CsdfConnection *connection, unsigned maxTokens)
{
    CsdfBufferStdLockFreeData *data = malloc(sizeof(CsdfBufferStdLockFreeData));
    data->start = 0;
    data->end = 0;
    data->tokens = malloc(maxTokens * connection->tokenSize);
    data->maxTokens = maxTokens;
    memcpy(data->tokens, connection->initialTokens, connection->numTokens * connection->tokenSize);
    data->end = connection->numTokens;
    return data;
}

static void delete_stdlockfree_buffer_data(void *bufferData)
{
    CsdfBufferStdLockFreeData *data = bufferData;
    free(data->tokens);
    free(data);
}

CsdfBuffer *new_stdlockfree_buffer(const CsdfConnection *connection, unsigned maxTokens)
{
    CsdfBuffer *buffer = malloc(sizeof(CsdfBuffer));
    buffer->connection = connection;
    buffer->data = new_stdlockfree_buffer_data(connection, maxTokens);
    buffer->pop = buffer_pop;
    buffer->push = buffer_push;
    buffer->numberOfTokens = number_tokens;
    return buffer;
}

void delete_stdlockfree_buffer(CsdfBuffer *buffer)
{
    delete_stdlockfree_buffer_data(buffer->data);
    free(buffer);
}
