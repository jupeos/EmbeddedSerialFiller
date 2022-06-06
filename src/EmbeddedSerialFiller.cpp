/**
 * \file    EmbeddedSerialFiller.cpp
 * \author  Julian Mitchell
 * \date    15th Apr 2020
 */

#if defined( PROFILE_NO_RTOS )
#include "EmbeddedSerialFiller_NoRTOS.cpp"
#else
#include "EmbeddedSerialFiller_RTOS.cpp"
#endif