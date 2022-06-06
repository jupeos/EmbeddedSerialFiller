/**
 * \file    esf_full_std_support.h
 * \author  Julian Mitchell
 * \date    30 Sep 2019
 */

#ifndef __ESF_FULL_STD_SUPPORT_H__
#define __ESF_FULL_STD_SUPPORT_H__

#include <condition_variable>
#include <mutex>

// Used for any OS that fully supports the standard C++11 library.

#define ESF_MUTEX std::mutex
#define ESF_LOCK std::unique_lock<std::mutex>
#define ESF_DEFER_LOCK std::defer_lock
#define ESF_CONDITION_VARIABLE std::condition_variable
#define ESF_NO_TIMEOUT std::cv_status::no_timeout
#define ESF_CONSTRUCTOR

#endif  // __ESF_FULL_STD_SUPPORT_H__
