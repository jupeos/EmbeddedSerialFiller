/**
 * \file    Container.h
 * \author  Julian Mitchell
 * \date    8th Jan 2020
 */

#ifndef ESF_CONTAINER_H
#define ESF_CONTAINER_H

#include "EmbeddedSerialFiller/Definitions.h"

namespace esf
{
// Generic container for converting structures to an array of bytes.
#include <cassert>

template <class T>
class Container
{
   public:
    Container()
    {
        assert( ESF_MAX_PACKET_SIZE >= sizeof( T ) );
        bytes.resize( sizeof( T ) );
        value = ::new( bytes.data() )( T );
    }
    Container( esf::ByteArray& input )
    {
        assert( input.size() == sizeof( T ) );
        bytes = input;
        value = ::new( bytes.data() )( T );
    }
    T* value;
    esf::ByteArray bytes;
};

}  // namespace esf

#endif  // ESF_CONTAINER_H
