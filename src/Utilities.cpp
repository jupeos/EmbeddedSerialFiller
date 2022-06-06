/**
 * \file    Utilities.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/Utilities.h"

#include <etl/crc16_ccitt.h>

#include <iostream>

namespace esf
{
StatusCode Utilities::MoveRxDataInBuffer( ByteArray& newRxData, ByteArray& rxDataBuffer, ByteArray& packet )
{
    StatusCode retVal = StatusCode::SUCCESS;
    // Clear any existing data from packet
    packet.clear();
    size_t idx = 0;
    const size_t newRxDataSize = newRxData.size();
    uint8_t* rxBuffer = newRxData.data();

    // Pop bytes from front of queue
    while( idx < newRxDataSize )
    {
        uint8_t byteOfData = rxBuffer[ idx ];
        ++idx;
        if( rxDataBuffer.full() )
        {
            retVal = StatusCode::ERROR_RX_DATA_BUFFER_FULL;
            break;
        }
        rxDataBuffer.emplace_back( byteOfData );

        // Look for 0x00 byte in data
        if( byteOfData == 0x00 )
        {
            // Found end-of-packet!
            // Move everything from the start to byteOfData from rxData into a new packet
#if defined( ESF_OPTIMISE )
            ByteArray::const_iterator iter = rxDataBuffer.begin();
            ByteArray::const_iterator endIter = rxDataBuffer.end();
            for( ; iter < endIter; ++iter )
            {
                packet.emplace_back( *iter );
            }
#else
            packet.insert( packet.end(), rxDataBuffer.begin(), rxDataBuffer.end() );
#endif

            rxDataBuffer.clear();
            if( idx == newRxDataSize )
            {
                newRxData.clear();
            }
            else
            {
                newRxData = ByteArray( newRxData.begin() + idx, newRxData.end() );
            }
            return retVal;
        }
    }
#if defined( ESF_REJECT_INCOMPLETE_PACKETS )
    rxDataBuffer.clear();
    retVal = StatusCode::ERROR_PACKET_INCOMPLETE;
#endif
    newRxData.clear();
    return retVal;
}

void Utilities::AddCrc( ByteArray& packet )
{
    uint16_t crcVal = etl::crc16_ccitt( packet.begin(), packet.end() );

    // Add CRC value to end of packet, MSB of CRC comes first
    packet.emplace_back( static_cast<uint8_t>( ( crcVal >> 8 ) & 0xFF ) );
    packet.emplace_back( static_cast<uint8_t>( ( crcVal >> 0 ) & 0xFF ) );
}

StatusCode Utilities::VerifyCrc( const ByteArray& packet )
{
    if( packet.size() < ESF_MIN_BYTES )
    {
        return StatusCode::ERROR_NOT_ENOUGH_BYTES;
    }
    else
    {
        // Extract the sent CRC value
        auto endIter = packet.end();
        uint16_t sentCrcVal = static_cast<uint16_t>( ( static_cast<uint8_t>( *( endIter - 2 ) ) << 8 ) | static_cast<uint8_t>( *( endIter - 1 ) << 0 ) );

        // Calculate CRC
        uint16_t calcCrcVal = etl::crc16_ccitt( packet.begin(), packet.end() - 2 );

        if( sentCrcVal != calcCrcVal )
        {
            return StatusCode::ERROR_CRC_CHECK_FAILED;
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode Utilities::SplitPacket( const ByteArray& packet, uint32_t startAt, Topic& topic, ByteArray& data )
{
    // Get length of topic
    assert( startAt < packet.size() );
    size_t lengthOfTopic = packet[ startAt ];

    size_t availableBytes = packet.size() - 2 - startAt;
    // Verify that length of topic is not longer than total length of packet - 2 bytes for CRC - start position
    if( lengthOfTopic > availableBytes )
    {
        return StatusCode::ERROR_LENGTH_OF_TOPIC_TOO_LONG;
    }

    auto topicBegin = packet.begin() + 1 + startAt;
    auto topicEnd = topicBegin + lengthOfTopic;
    auto dataEnd = packet.end() - 2;
    topic.clear();
    topic.insert( topic.end(), topicBegin, topicEnd );

    data.clear();
    data.insert( data.end(), topicEnd, dataEnd );
    return StatusCode::SUCCESS;
}

const char* Utilities::StatusCodeToString( StatusCode statusCode )
{
    switch( statusCode )
    {
        case StatusCode::SUCCESS:
            return "SUCCESS";
        case StatusCode::ERROR_CRC_CHECK_FAILED:
            return "ERROR_CRC_CHECK_FAILED";
        case StatusCode::ERROR_NOT_ENOUGH_BYTES:
            return "ERROR_NOT_ENOUGH_BYTES";
        case StatusCode::ERROR_UNRECOGNISED_PACKET_TYPE:
            return "ERROR_UNRECOGNISED_PACKET_TYPE";
        case StatusCode::ERROR_UNEXPECTED_ACK:
            return "ERROR_UNEXPECTED_ACK";
        case StatusCode::ERROR_LENGTH_OF_TOPIC_TOO_LONG:
            return "ERROR_LENGTH_OF_TOPIC_TOO_LONG";
        case StatusCode::ERROR_UNRECOGNISED_SUBSCRIBER:
            return "ERROR_UNRECOGNISED_SUBSCRIBER";
        case StatusCode::ERROR_ZERO_BYTE_NOT_EXPECTED:
            return "ERROR_ZERO_BYTE_NOT_EXPECTED";
        case StatusCode::ERROR_RX_DATA_BUFFER_FULL:
            return "ERROR_RX_DATA_BUFFER_FULL";
#if defined( ESF_REJECT_INCOMPLETE_PACKETS )
        case StatusCode::ERROR_PACKET_INCOMPLETE:
            return "ERROR_PACKET_INCOMPLETE";
#endif
        default:
            return "UNKNOWN";
    }
}

}  // namespace esf
