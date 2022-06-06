/**
 * \file    CobsTranscoder.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/CobsTranscoder.h"

#include <iostream>

namespace esf
{
void CobsTranscoder::Encode( const ByteArray& rawData, ByteArray& encodedData )
{
#if defined( ESF_OPTIMISE )
    // Pre-size the encoded data container.
    encodedData.resize( 2 + rawData.size() + ( rawData.size() / 254 ) );
    Encode( rawData.data(), rawData.size(), encodedData.data() );
#else
    size_t startOfCurrBlock = 0;
    uint8_t numElementsInCurrBlock = 0;

    encodedData.resize( 2 + rawData.size() + ( rawData.size() / 254 ) );
    auto encodedIter = encodedData.begin();
    // Create space for first (this will be overwritten once count to next 0x00 is known)
    *encodedIter = 0;
    ++encodedIter;
    size_t encodedDataSize = 1;
    auto rawIter = rawData.begin();
    while( rawIter != rawData.end() )
    {
        if( *rawIter == 0x00 )
        {
            // Save the number of elements before the next 0x00 into
            // the output
            encodedData[ startOfCurrBlock ] = static_cast<uint8_t>( numElementsInCurrBlock + 1 );

            // Add placeholder at start of next block
            *encodedIter = 0;
            ++encodedIter;
            ++encodedDataSize;

            startOfCurrBlock = encodedDataSize - 1;

            // Reset count of num. elements in current block
            numElementsInCurrBlock = 0;
        }
        else
        {
            *encodedIter = *rawIter;
            ++encodedIter;
            ++encodedDataSize;

            numElementsInCurrBlock++;

            if( numElementsInCurrBlock == 254 )
            {
                encodedData[ startOfCurrBlock ] = static_cast<uint8_t>( numElementsInCurrBlock + 1 );

                // Add placeholder at start of next block
                *encodedIter = 0;
                ++encodedIter;
                ++encodedDataSize;

                startOfCurrBlock = encodedDataSize - 1;

                // Reset count of num. elements in current block
                numElementsInCurrBlock = 0;
            }
        }
        ++rawIter;
    }

    // Finish the last block
    // Insert pointer to the terminating 0x00 character
    encodedData[ startOfCurrBlock ] = numElementsInCurrBlock + 1;
    *encodedIter = 0;
#endif
}

void CobsTranscoder::Encode( const uint8_t* rawData, size_t length, uint8_t* encodedData )
{
    size_t startOfBlock = 0;
    uint8_t elementsInBlock = 0;

    // Reserve space for the zero count (this will be overwritten once a count to the next 0x00 is known)
    encodedData[ startOfBlock ] = 0;
    size_t encodedDataSize = 1;

    size_t i = 0;
    while( i < length )
    {
        if( rawData[ i ] == 0 )
        {
            encodedData[ startOfBlock ] = elementsInBlock + 1;
            startOfBlock = encodedDataSize;
            // Place holder for start of next block.
            encodedData[ startOfBlock ] = 0;
            ++encodedDataSize;

            // Reset element count.
            elementsInBlock = 0;
        }
        else
        {
            encodedData[ encodedDataSize ] = rawData[ i ];
            ++encodedDataSize;
            ++elementsInBlock;
            if( elementsInBlock == 254 )
            {
                encodedData[ startOfBlock ] = elementsInBlock + 1;
                startOfBlock = encodedDataSize;
                // Place holder for start of next block.
                encodedData[ startOfBlock ] = 0;
                ++encodedDataSize;

                // Reset element count.
                elementsInBlock = 0;
            }
        }
        ++i;
    }
    // Finish the last block...
    encodedData[ startOfBlock ] = elementsInBlock + 1;
    // ...and terminate.
    encodedData[ encodedDataSize ] = 0;
}

StatusCode CobsTranscoder::Decode( const ByteArray& encodedData, ByteArray& decodedData )
{
#if defined( ESF_OPTIMISE )
    StatusCode result = StatusCode::ERROR_NOT_ENOUGH_BYTES;
    if( encodedData.size() >= ESF_MIN_BYTES )
    {
        // Pre-size the decoded data container.
        decodedData.resize( encodedData.size() - 2 - ( encodedData.size() - 2 - ( encodedData.size() - 2 ) / 254 ) / 254 );
        result = Decode( encodedData.data(), encodedData.size(), decodedData.data() );
    }
    return result;
#else
    decodedData.clear();

    size_t encodedDataPos = 0;

    while( encodedDataPos < encodedData.size() )
    {
        int numElementsInBlock = static_cast<uint8_t>( encodedData[ encodedDataPos ] ) - 1;
        encodedDataPos++;

        // Copy across all bytes within block
        for( int i = 0; i < numElementsInBlock; i++ )
        {
            uint8_t byteOfData = encodedData[ encodedDataPos ];
            if( byteOfData == 0x00 )
            {
                decodedData.clear();
                return StatusCode::ERROR_ZERO_BYTE_NOT_EXPECTED;
            }

            decodedData.emplace_back( encodedData[ encodedDataPos ] );
            encodedDataPos++;
        }

        if( encodedData[ encodedDataPos ] == 0x00 )
        {
            // End of packet found!
            break;
        }

        // We only add a 0x00 byte to the decoded data
        // IF the num. of elements in block was less than 254.
        // If num. elements in block is max (254), then we know that
        // the block was created due to it reaching maximum size, not because
        // a 0x00 was found
        if( numElementsInBlock < 0xFE )
        {
            decodedData.emplace_back( 0x00 );
        }
    }
    return StatusCode::SUCCESS;
#endif
}

StatusCode CobsTranscoder::Decode( const uint8_t* encodedData, size_t length, uint8_t* decodedData )
{
    size_t encodedDataPos = 0;
    size_t decodedDataPos = 0;
    while( encodedDataPos < length )
    {
        size_t elementsInBlock = encodedData[ encodedDataPos ] - 1;
        ++encodedDataPos;

        // Copy across all bytes within block
        for( size_t i = 0; i < elementsInBlock; i++ )
        {
            uint8_t byteOfData = encodedData[ encodedDataPos ];
            if( byteOfData == 0x00 )
            {
                return StatusCode::ERROR_ZERO_BYTE_NOT_EXPECTED;
            }

            decodedData[ decodedDataPos ] = encodedData[ encodedDataPos ];
            ++encodedDataPos;
            ++decodedDataPos;
        }

        if( encodedData[ encodedDataPos ] == 0x00 )
        {
            // End of packet found!
            break;
        }

        // We only add a 0x00 byte to the decoded data if the number of elements
        // in the block was less than 254. If the number of elements in the block
        // is max (254), then we know that the block was created due to it
        // reaching maximum size, not because a 0x00 was found.
        if( elementsInBlock < 0xFE )
        {
            decodedData[ encodedDataPos ] = 0;
            ++decodedDataPos;
        }
    }
    return StatusCode::SUCCESS;
}

}  // namespace esf
