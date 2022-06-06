/**
 * \file    esf_freertos_abstraction.c
 * \author  Julian Mitchell
 * \date    05 Oct 2019
 */

#include "esf_freertos_abstraction.h"

//----------------------------------------------------------------------------//

FreeRTOS_Lock::FreeRTOS_Lock( ESF_MUTEX& mutex, char defer_lock ) : mutex_( mutex )
{
    if( mutex_ == NULL )
    {
        mutex_ = xSemaphoreCreateMutexStatic( &mutexBuffer_ );
    }
    if( defer_lock )
    {
        // Default.
    }
    else
    {
        lock();
    }
}

FreeRTOS_Lock::~FreeRTOS_Lock()
{
    unlock();
}

void FreeRTOS_Lock::lock()
{
    xSemaphoreTake( mutex_, portMAX_DELAY );
}

void FreeRTOS_Lock::unlock()
{
    xSemaphoreGive( mutex_ );
}

//----------------------------------------------------------------------------//

FreeRTOS_ConditionVariable::FreeRTOS_ConditionVariable()
{
    eventGroup_ = xEventGroupCreateStatic( &eventGroupBuffer_ );
}

char FreeRTOS_ConditionVariable::wait_for( FreeRTOS_Lock& lock, std::chrono::milliseconds timeout )
{
    return xEventGroupWaitBits( eventGroup_, 1, pdTRUE, pdFALSE, (TickType_t)timeout.count() );
}

void FreeRTOS_ConditionVariable::notify_all()
{
    xEventGroupSetBits( eventGroup_, 1 );
}
