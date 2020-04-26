/**
 * \file    EmbeddedSerialFiller.h
 * \author  Julian Mitchell
 * \date    15th Apr 2020
 */

#if defined( PROFILE_NO_RTOS )
#include "EmbeddedSerialFiller_NoRTOS.h"
#else
#include "EmbeddedSerialFiller_RTOS.h"
#endif