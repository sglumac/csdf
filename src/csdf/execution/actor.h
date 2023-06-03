/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#ifndef CSDF_EXECUTION_ACTOR_H
#define CSDF_EXECUTION_ACTOR_H

#include "buffer.h"

#include <csdf/actor.h>
#include <csdf/record.h>

#include <stdbool.h>

typedef struct CsdfActorRun
{
    const CsdfActor *actor;
    uint8_t *consumed;
    uint8_t *produced;
    CsdfRecordData *recordData;
    CsdfBuffer **inputBuffers;
    CsdfBuffer **outputBuffers;
} CsdfActorRun;

CsdfActorRun *new_actor_run(const CsdfActor *actor, CsdfRecordData *recordData);

void delete_actor_run(CsdfActorRun *runData);

bool can_fire(CsdfActorRun *runData);

void fire(CsdfActorRun *runData);

#endif // CSDF_EXECUTION_ACTOR_H
