/**
 * \file    esf_freertos_abstraction.h
 * \author  Julian Mitchell
 * \date    05 Oct 2019
 */

#ifndef __ESF_FREERTOS_ABSTRACTION_H__
#define __ESF_FREERTOS_ABSTRACTION_H__

#include <chrono>

#include "..\..\FreeRTOS\Source\include\FreeRTOS.h"
#include "..\..\FreeRTOS\Source\include\event_groups.h"
#include "..\..\FreeRTOS\Source\include\semphr.h"

// A bare minimum implementation to support a mutex, lock & condition variable
// using suitable embOS equivalents.

#define ESF_MUTEX SemaphoreHandle_t
#define ESF_LOCK FreeRTOS_Lock
#define ESF_DEFER_LOCK 1
#define ESF_CONDITION_VARIABLE FreeRTOS_ConditionVariable
#define ESF_NO_TIMEOUT 0

class FreeRTOS_Lock
{
   public:
    FreeRTOS_Lock( ESF_MUTEX& mutex, char defer_lock );
    ~FreeRTOS_Lock();
    void lock();
    void unlock();

   private:
    ESF_MUTEX& mutex_;
    StaticSemaphore_t mutexBuffer_;
};

class FreeRTOS_ConditionVariable
{
   public:
    FreeRTOS_ConditionVariable();
    char wait_for( FreeRTOS_Lock& lock, std::chrono::milliseconds timeout );
    void notify_all();

   private:
    EventGroupHandle_t eventGroup_;
    StaticEventGroup_t eventGroupBuffer_;
};

#endif  // __ESF_FREERTOS_ABSTRACTION_H__
