/**
 * \file    CobsTranscoder.h
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#ifndef ESF_COBS_TRANSCODER_H
#define ESF_COBS_TRANSCODER_H

#include "EmbeddedSerialFiller/Definitions.h"

namespace esf
{
class CobsTranscoder
{
   public:
    /// \details    The encoding process cannot fail.
    static void Encode( const ByteArray& rawData, ByteArray& encodedData );

    /// \brief      Decode data using "Consistent Overhead Byte Stuffing" (COBS).
    /// \details    Provided encodedData is expected to be a single, valid COBS encoded packet. If not, method
    ///             will return #DecodeStatus::ERROR_ZERO_BYTE_NOT_EXPECTED.
    ///             #decodedData is emptied of any pre-existing data. If the decode fails, decodedData is left empty.
    static StatusCode Decode( const ByteArray& encodedData, ByteArray& decodedData );
};

}  // namespace esf

#endif  // #ifndef ESF_COBS_TRANSCODER_H
