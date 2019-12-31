/**
 * \file    esf_embos_abstraction.c
 * \author  Julian Mitchell
 * \date    30 Sep 2019
 */

#include "esf_embos_abstraction.h"

#include <stdio.h>

void Embos_Builder( ESF_MUTEX& mutex )
{
    OS_MUTEX_Create( &mutex );
}

//----------------------------------------------------------------------------//

Embos_Lock::Embos_Lock( ESF_MUTEX& mutex, char defer_lock ) : mutex_( mutex )
{
    if( defer_lock )
    {
        // Default.
    }
    else
    {
        lock();
    }
}

Embos_Lock::~Embos_Lock()
{
    unlock();
}

void Embos_Lock::lock()
{
    OS_MUTEX_LockBlocked( &mutex_ );
}

void Embos_Lock::unlock()
{
    OS_MUTEX_Unlock( &mutex_ );
}

//----------------------------------------------------------------------------//

Embos_ConditionVariable::Embos_ConditionVariable()
{
    OS_EVENT_Create( &event_ );
    OS_EVENT_SetResetMode( &event_, OS_EVENT_RESET_MODE_AUTO );
}

char Embos_ConditionVariable::wait_for( Embos_Lock& lock, std::chrono::milliseconds timeout )
{
    lock.unlock();
    int8_t result = OS_EVENT_GetTimed( &event_, timeout.count() );
    if( result != ESF_NO_TIMEOUT )
    {
        // Timeout ocurred.
        OS_EVENT_Reset( &event_ );
    }
    lock.lock();
    return result;
}

void Embos_ConditionVariable::notify_all()
{
    OS_EVENT_Set( &event_ );
}
