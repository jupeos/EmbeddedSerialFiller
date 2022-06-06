/**
 * \file    CobsEncodeDecodeTest.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/CobsTranscoder.h"
#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
class CobsEncodeDecodeTest : public ::testing::Test
{
   protected:
    CobsEncodeDecodeTest() {}

    virtual ~CobsEncodeDecodeTest() {}
};

TEST_F( CobsEncodeDecodeTest, NoZerosInDataTest )
{
    ByteArray rawData = ByteArray( { 0x01, 0x02, 0x03 } );
    ByteArray encodedData;
    CobsTranscoder::Encode( rawData, encodedData );
    EXPECT_EQ( ByteArray( { 0x04, 0x01, 0x02, 0x03, 0x00 } ), encodedData );

    ByteArray decodedData;
    CobsTranscoder::Decode( encodedData, decodedData );
    EXPECT_EQ( rawData, decodedData );
}

TEST_F( CobsEncodeDecodeTest, ZeroInDataTest )
{
    ByteArray rawData = ByteArray( { 0xAA, 0x00, 0xAB } );
    ByteArray encodedData;
    CobsTranscoder::Encode( rawData, encodedData );
    EXPECT_EQ( ByteArray( { 0x02, 0xAA, 0x02, 0xAB, 0x00 } ), encodedData );

    ByteArray decodedData;
    CobsTranscoder::Decode( encodedData, decodedData );
    EXPECT_EQ( rawData, decodedData );
}

TEST_F( CobsEncodeDecodeTest, AllZerosTest )
{
    ByteArray rawData = ByteArray( { 0x00, 0x00, 0x00 } );
    ByteArray encodedData;
    CobsTranscoder::Encode( rawData, encodedData );
    EXPECT_EQ( ByteArray( { 0x01, 0x01, 0x01, 0x01, 0x00 } ), encodedData );
    ByteArray decodedData;
    CobsTranscoder::Decode( encodedData, decodedData );
    EXPECT_EQ( rawData, decodedData );
}

TEST_F( CobsEncodeDecodeTest, ManyBytesTest )
{
    ByteArray rawData = ByteArray( { 0x00, 0xAA, 0xAB, 0xAC, 0x00, 0x00, 0xAD } );
    ByteArray encodedData;
    CobsTranscoder::Encode( rawData, encodedData );
    EXPECT_EQ( ByteArray( { 0x01, 0x04, 0xAA, 0xAB, 0xAC, 0x01, 0x02, 0xAD, 0x00 } ), encodedData );
    ByteArray decodedData;
    CobsTranscoder::Decode( encodedData, decodedData );
    EXPECT_EQ( rawData, decodedData );
}

TEST_F( CobsEncodeDecodeTest, IncorrectDecodeTest )
{
    // Encoded data is incorrectly formed, 0x02 states there should be 1 byte of valid data following,
    // but next byte is 0x00 (end of frame)
    ByteArray decodedData;
    EXPECT_EQ( CobsTranscoder::Decode( ByteArray( { 0x02, 0x00, 0x00 } ), decodedData ), StatusCode::ERROR_ZERO_BYTE_NOT_EXPECTED );
}

TEST_F( CobsEncodeDecodeTest, IncorrectDecodeTest2 )
{
    // Error here is the 0x00 after 0x03
    ByteArray decodedData;
    EXPECT_EQ( CobsTranscoder::Decode( ByteArray( { 0x02, 0xAA, 0x03, 0x00, 0xAB } ), decodedData ), StatusCode::ERROR_ZERO_BYTE_NOT_EXPECTED );
}

TEST_F( CobsEncodeDecodeTest, Exactly254Bytes )
{
    ByteArray rawData;
    for( int i = 1; i < 255; i++ )
    {
        rawData.push_back( (char)i );
    }

    ByteArray encodedData;
    CobsTranscoder::Encode( rawData, encodedData );
    EXPECT_EQ( 257, encodedData.size() );
    EXPECT_EQ( 0xFF, encodedData[ 0 ] );
    for( int i = 1; i < 255; i++ )
    {
        EXPECT_EQ( i, encodedData[ i ] );
    }
    EXPECT_EQ( 0x01, encodedData[ 255 ] );
    EXPECT_EQ( 0x00, encodedData[ 256 ] );

    ByteArray decodedData;
    CobsTranscoder::Decode( encodedData, decodedData );
    EXPECT_EQ( rawData, decodedData );
}

TEST_F( CobsEncodeDecodeTest, MoreThan254Bytes )
{
    ByteArray rawData;
    for( int i = 0; i < 260; i++ )
    {
        rawData.push_back( 0x01 );
    }

    ByteArray encodedData;
    CobsTranscoder::Encode( rawData, encodedData );
    EXPECT_EQ( 263, encodedData.size() );
    EXPECT_EQ( 0xFF, encodedData[ 0 ] );
    for( int i = 1; i < 255; i++ )
    {
        EXPECT_EQ( 0x01, encodedData[ i ] );
    }
    EXPECT_EQ( 0x07, encodedData[ 255 ] );
    for( int i = 256; i < 262; i++ )
    {
        EXPECT_EQ( 0x01, encodedData[ i ] );
    }
    EXPECT_EQ( 0x00, encodedData[ 262 ] );

    ByteArray decodedData;
    CobsTranscoder::Decode( encodedData, decodedData );
    EXPECT_EQ( rawData, decodedData );

    // A much larger data set
    rawData.clear();
    for( int i = 0; i < 762; i++ )
    {
        rawData.push_back( 0x01 );
    }

    encodedData.clear();
    CobsTranscoder::Encode( rawData, encodedData );
    EXPECT_EQ( 767, encodedData.size() );

    decodedData.clear();
    CobsTranscoder::Decode( encodedData, decodedData );
    EXPECT_EQ( rawData, decodedData );
}

}  // namespace
