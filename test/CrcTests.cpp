/**
 * \file    CrcTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include <etl/crc16_ccitt.h>

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "EmbeddedSerialFiller/Utilities.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
// The fixture for testing class Foo.
class CrcTests : public ::testing::Test
{
   protected:
    CrcTests() {}

    virtual ~CrcTests() {}
};

TEST_F( CrcTests, StandardCheckTest )
{
    auto packet = ByteArray( { '1', '2', '3', '4', '5', '6', '7', '8', '9' } );
    uint16_t crcVal2 = etl::crc16_ccitt( packet.begin(), packet.end() );
    EXPECT_EQ( 0x29B1, crcVal2 );
}

TEST_F( CrcTests, EmptyTest )
{
    auto packet = ByteArray();
    uint16_t crcVal2 = etl::crc16_ccitt( packet.begin(), packet.end() );
    EXPECT_EQ( 0xFFFF, crcVal2 );
}

TEST_F( CrcTests, ZeroTest )
{
    auto packet = ByteArray( { 0 } );
    uint16_t crcVal2 = etl::crc16_ccitt( packet.begin(), packet.end() );
    EXPECT_EQ( 0xE1F0, crcVal2 );
}

TEST_F( CrcTests, LargeTest )
{
    ByteArray data;
    ByteArray sequence( { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' } );
    // Populate data with 300 characters
    for( int i = 0; i < 30; i++ )
    {
        std::copy( sequence.begin(), sequence.end(), std::back_inserter( data ) );
    }

    uint16_t crcVal2 = etl::crc16_ccitt( data.begin(), data.end() );
    EXPECT_EQ( 0xC347, crcVal2 );
}

}  // namespace
