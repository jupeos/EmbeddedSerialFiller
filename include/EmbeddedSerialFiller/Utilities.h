/**
 * \file    Utilities.h
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#ifndef ESF_UTILITIES_H
#define ESF_UTILITIES_H

#include <cstdint>

#include "EmbeddedSerialFiller/Definitions.h"

namespace esf
{
class Utilities
{
   public:
    static StatusCode SplitPacket( const ByteArray& packet, uint32_t startAt, Topic& topic, ByteArray& data );

    /// \details    Moves new RX data into the RX buffer, while looking for the
    ///             end-of-frame character. If EOF is found, packet is populated
    ///             and this method returns.
    static StatusCode MoveRxDataInBuffer( ByteArray& newRxData, ByteArray& rxDataBuffer, ByteArray& packet );

    static void AddCrc( ByteArray& packet );

    /// \param  packet  Packet must be COBS decoded before passing into here. Expects
    ///                 last two bytes to be the CRC value of all the bytes proceeding it.
    static StatusCode VerifyCrc( const ByteArray& packet );

    static const char* StatusCodeToString( StatusCode statusCode );
};

}  // namespace esf

#endif  // #ifndef ESF_UTILITIES_H
