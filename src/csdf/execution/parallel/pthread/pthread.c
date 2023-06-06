/****************************************************************************
C implementation of Synchronous Data Flow (CSDF)

MIT License

Copyright (c) 2023 Slaven Glumac
****************************************************************************/

#include "pthread.h"

#include <pthread.h>
#include <unistd.h>

static void csdf_sleep(unsigned int microseconds)
{
    usleep(microseconds);
}

typedef struct PThreadData
{
    pthread_t pThread;
    CsdfTask task;
    void *taskData;
} PThreadData;

void *pthread_task(void *threadData)
{
    PThreadData *pThreadData = threadData;
    pThreadData->task(pThreadData->taskData);
    pthread_exit(NULL);
}

static bool csdf_create_thread(void *threadData, CsdfTask task, void *taskData)
{
    PThreadData *pThreadData = threadData;
    pThreadData->task = task;
    pThreadData->taskData = taskData;
    return !pthread_create(&pThreadData->pThread, NULL, pthread_task, pThreadData);
}

bool csdf_join_thread(void *threadData)
{
    PThreadData *pThreadData = threadData;

    return !pthread_join(pThreadData->pThread, NULL);
}

const CsdfThreading CSDF_PTHREAD_THREADING = {
    .threadDataSize = sizeof(PThreadData),
    .sleep = csdf_sleep,
    .createThread = csdf_create_thread,
    .joinThread = csdf_join_thread,
    .microsecondsSleep = 100};
