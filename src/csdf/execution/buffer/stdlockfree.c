/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "stdlockfree.h"

#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#if ATOMIC_INT_LOCK_FREE != 2
#error "atomic_int not supported!"
#endif

typedef struct CsdfBufferStdLockFreeData
{
    atomic_uint start;
    atomic_uint end;
    unsigned maxTokens;
    uint8_t *tokens;
} CsdfBufferStdLockFreeData;

static void buffer_push(CsdfBuffer *buffer, const uint8_t *token)
{
    CsdfBufferStdLockFreeData *data = buffer->data;
    unsigned end = atomic_load(&data->end);
    int tokenSize = buffer->connection->tokenSize;
    uint8_t *bufferEnd = data->tokens + tokenSize * end;
    end = (end + 1) % data->maxTokens;
    unsigned start = atomic_load(&data->start);
    if (end == start)
    {
        exit(123);
    }
    memcpy(bufferEnd, token, tokenSize);
    atomic_exchange(&data->end, end);
}

static void atomic_modulus(atomic_uint *value, unsigned divisor)
{
    int currentValue, remainder;
    do
    {
        currentValue = atomic_load(value);
        remainder = currentValue % divisor;
    } while (!atomic_compare_exchange_weak(value, &currentValue, remainder));
}

static void buffer_pop(CsdfBuffer *buffer, uint8_t *token)
{
    CsdfBufferStdLockFreeData *data = buffer->data;
    unsigned start = atomic_fetch_add(&data->start, 1);
    uint8_t *bufferStart = data->tokens + buffer->connection->tokenSize * start;
    memcpy(token, bufferStart, buffer->connection->tokenSize);
    atomic_modulus(&data->start, data->maxTokens);
}

static unsigned number_tokens(CsdfBuffer *buffer)
{
    const CsdfBufferStdLockFreeData *data = buffer->data;
    unsigned start = atomic_load(&data->start);
    unsigned end = atomic_load(&data->end);

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
