/******************************************************************************
The MIT License(MIT)

Embedded Serial Filler.
https://github.com/jupeos/EmbeddedSerialFiller

Copyright(c) 2019 Julian Mitchell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#ifndef __ESF_ABSTRACTION_H__
#define __ESF_ABSTRACTION_H__

#if defined( PROFILE_WINDOWS ) || defined( PROFILE_GCC_LINUX_X86 )
#include "esf_full_std_support.h"
//#pragma message( "OS Profile: PROFILE_WINDOWS or PROFILE_LINUX" )
#elif defined( PROFILE_EMBOS )
//#pragma message( "OS Profile: PROFILE_EMBOS" )
#include "esf_embos_abstraction.h"
#elif defined( PROFILE_FREERTOS )
//#pragma message( "OS Profile: PROFILE_FREERTOS" )
#include "esf_freertos_abstraction.h"
#elif defined( PROFILE_OTHER_OS )
#include "esf_other_os_abstraction.h"
#elif defined( PROFILE_NO_RTOS )
// No header for this.
#else
#error Must provide an implementation for various OS specific abstratctions e.g. mutex, condition_variable etc..
#endif

#endif  // __ESF_ABSTRACTION_H__
