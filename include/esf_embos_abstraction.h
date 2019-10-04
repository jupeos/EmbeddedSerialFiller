/**
 * \file    esf_embos_abstraction.h
 * \author  Julian Mitchell
 * \date    30 Sep 2019
 */

#ifndef __ESF_EMBOS_ABSTRACTION_H__
#define __ESF_EMBOS_ABSTRACTION_H__

#include "..\..\Segger_Libs\Start\Inc\RTOS.h"
#include <chrono>

// A bare minimum implementation to support a mutex, lock & condition variable
// using suitable embOS equivalents.

#define ESF_MUTEX OS_MUTEX
#define ESF_LOCK Embos_Lock
#define ESF_DEFER_LOCK 1
#define ESF_CONDITION_VARIABLE Embos_ConditionVariable 
#define ESF_NO_TIMEOUT 0

class Embos_Lock
{
public:
  Embos_Lock(ESF_MUTEX& mutex, char defer_lock);
  ~Embos_Lock();
  void lock();
  void unlock();
  
private:
  ESF_MUTEX& mutex_;
};

class Embos_ConditionVariable
{
public:
  Embos_ConditionVariable();
  char wait_for(Embos_Lock& lock, std::chrono::milliseconds timeout);
  void notify_all();
  
private:
  OS_EVENT event_;
};

#endif // __ESF_EMBOS_ABSTRACTION_H__
