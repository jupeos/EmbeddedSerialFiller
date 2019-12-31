/**
 * \file    Utilities.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/Utilities.h"
#include <etl/crc16_ccitt.h>
#include <iostream>
#include "EmbeddedSerialFiller/Logger.h"

namespace esf
{
void Utilities::MoveRxDataInBuffer( ByteArray& newRxData, ByteArray& rxDataBuffer, ByteArray& packet )
{
    //            std::cout << "Move RX data called." << std::endl;

    // Clear any existing data from packet
    packet.clear();
    size_t idx = 0;

    // Pop bytes from front of queue
    while( idx < newRxData.size() )
    {
        uint8_t byteOfData = newRxData.at( idx++ );
        if( rxDataBuffer.full() )
        {
            //std::cout << config_TERM_TEXT_COLOUR_RED << "rxDataBuffer is full" << config_TERM_TEXT_FORMAT_NORMAL << std::endl;
            break;
        }
        rxDataBuffer.push_back( byteOfData );

        // Look for 0x00 byte in data
        if( byteOfData == 0x00 )
        {
            // Found end-of-packet!

            // Move everything from the start to byteOfData from rxData
            // into a new packet
            for( auto it = rxDataBuffer.begin(); it != rxDataBuffer.end(); ++it )
            {
                packet.push_back( *it );
            }

            rxDataBuffer.clear();
            newRxData = ByteArray( newRxData.begin() + idx, newRxData.end() );
            //                    std::cout << "Move RX data returning." << std::endl;
            return;
        }
    }
    newRxData.clear();
    //            std::cout << "Move RX data returning." << std::endl;
}

void Utilities::AddCrc( ByteArray& packet )
{
    uint16_t crcVal = etl::crc16_ccitt( packet.begin(), packet.end() );

    // Add CRC value to end of packet, MSB of CRC
    // comes first
    packet.push_back( static_cast<uint8_t>( ( crcVal >> 8 ) & 0xFF ) );
    packet.push_back( static_cast<uint8_t>( ( crcVal >> 0 ) & 0xFF ) );
}

StatusCode Utilities::VerifyCrc( const ByteArray& packet )
{
    if( packet.size() < 3 )
    {
        //LOG((*logger_), ERROR, "Cannot verify CRC with less than 3 bytes in packet.");
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
            //std::cout << config_TERM_TEXT_COLOUR_RED << "CRC check failed." << config_TERM_TEXT_FORMAT_NORMAL << std::endl;
            return StatusCode::ERROR_CRC_CHECK_FAILED;
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode Utilities::SplitPacket( const ByteArray& packet, uint32_t startAt, Topic& topic, ByteArray& data )
{
    // Get length of topic
    size_t lengthOfTopic = packet.at( startAt );

    size_t availableBytes = packet.size() - 2 - startAt;
    // Verify that length of topic is not longer than total length of packet - 2 bytes for CRC - start position
    if( lengthOfTopic > availableBytes )
    {
        //std::cout << config_TERM_TEXT_COLOUR_RED << "Length of topic too long." << config_TERM_TEXT_FORMAT_NORMAL << std::endl;
        return StatusCode::ERROR_LENGTH_OF_TOPIC_TOO_LONG;
    }

    auto it1 = packet.begin() + 1 + startAt + lengthOfTopic;
    auto it2 = packet.end() - 2;
    topic.clear();
    for( auto it = packet.begin() + 1 + startAt; it != it1; ++it )
    {
        topic.push_back( *it );
    }
    data.clear();
    for( auto it = it1; it != it2; ++it )
    {
        data.push_back( *it );
    }
    return StatusCode::SUCCESS;
}

}  // namespace esf
