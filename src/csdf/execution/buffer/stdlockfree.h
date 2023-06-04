/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_BUFFER_STDLOCKFREE_H
#define CSDF_EXECUTION_BUFFER_STDLOCKFREE_H

#include <csdf/execution/buffer.h>

CsdfBuffer *new_stdlockfree_buffer(const CsdfConnection *connection, unsigned maxTokens);

void delete_stdlockfree_buffer(CsdfBuffer *buffer);

#endif // CSDF_EXECUTION_BUFFER_STDLOCKFREE_H
