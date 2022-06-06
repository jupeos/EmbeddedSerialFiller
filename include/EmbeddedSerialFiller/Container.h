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

template <class STRUCT, size_t MAX_SIZE = ESF_MAX_PACKET_SIZE, class BYTE_ARRAY = esf::ByteArray>
class Container
{
   public:
    Container()
    {
        assert( MAX_SIZE >= sizeof( STRUCT ) );
        bytes.resize( sizeof( STRUCT ) );
        value = ::new( bytes.data() )( STRUCT );
    }
    Container( BYTE_ARRAY& input )
    {
        assert( input.size() == sizeof( STRUCT ) );
        bytes = input;
        value = ::new( bytes.data() )( STRUCT );
    }
    STRUCT* value;
    BYTE_ARRAY bytes;
};

}  // namespace esf

#endif  // ESF_CONTAINER_H
