/**
 * \file    SplitPacketTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "EmbeddedSerialFiller/Utilities.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
class SplitPacketTests : public ::testing::Test
{
   protected:
    SplitPacketTests() {}

    virtual ~SplitPacketTests() {}
};

TEST_F( SplitPacketTests, BasicTest )
{
    // The "12" at the end of the string is a fake (and incorrect) CRC
    // value, but this doesn't matter as SplitPacket does not validate the CRC
    auto packet = ByteArray( { 0x01, 0x00, 0x01, 0x04, 't', 'e', 's', 't', 'h', 'e', 'l', 'l', 'o', 0x01, 0x01 } );
    Topic topic;
    auto data = ByteArray();
    Utilities::SplitPacket( packet, 3, topic, data );
    EXPECT_EQ( "test", topic );
    EXPECT_EQ( ByteArray( { 'h', 'e', 'l', 'l', 'o' } ), data );
}

TEST_F( SplitPacketTests, BogusTopicLength )
{
    auto packet = ByteArray( { 0x01, 0x00, 0x01, 0x06, 0x02, 0x03 } );
    Topic topic;
    auto data = ByteArray();
    EXPECT_EQ( Utilities::SplitPacket( packet, 3, topic, data ), StatusCode::ERROR_LENGTH_OF_TOPIC_TOO_LONG );
    EXPECT_EQ( "", topic );
    EXPECT_TRUE( data.empty() );
}

}  // namespace
