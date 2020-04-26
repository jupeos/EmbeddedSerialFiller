/**
 * \file    esf_embos_abstraction.h
 * \author  Julian Mitchell
 * \date    30 Sep 2019
 */

#ifndef __ESF_NOOS_ABSTRACTION_H__
#define __ESF_NOOS_ABSTRACTION_H__

#if( OS_DEBUG != 0 )
#pragma message( "Defined OS_DEBUG != 0" )
#endif

// A bare minimum implementation to support a mutex, lock & condition variable
// using suitable embOS equivalents.

#define ESF_MUTEX int

#define SLASH( s ) / ##s
#define COMMENT SLASH( / )

#define ESF_NO_TIMEOUT 0
//#define ESF_CONSTRUCTOR do{}while();
#define ESF_CONSTRUCTOR( X )
//#define ESF_CONDITION_VARIABLE COMMENT
#define ESF_CONDITION_VARIABLE int

#endif  // __ESF_NOOS_ABSTRACTION_H__
