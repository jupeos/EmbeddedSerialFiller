/**
 * \file    PacketizeTest.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "EmbeddedSerialFiller/Utilities.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
class PacketizeTest : public ::testing::Test
{
   protected:
    PacketizeTest() {}

    virtual ~PacketizeTest() {}
};

TEST_F( PacketizeTest, SimplePacket )
{
    auto newRxData = ByteQueue( { 0x01, 0x02, 0x00 } );
    auto existingRxData = ByteQueue();

    std::vector<ByteArray> packets;
    ByteArray packet;

    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 1, packets.size() );
    EXPECT_EQ( ByteArray( { 0x01, 0x02, 0x00 } ), packets[ 0 ] );
}

TEST_F( PacketizeTest, TwoPackets )
{
    auto newRxData = ByteQueue( { 0x01, 0x02, 0x00, 0x01, 0x00 } );
    auto existingRxData = ByteQueue();

    std::vector<ByteArray> packets;
    ByteArray packet;

    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 2, packets.size() );
    EXPECT_EQ( ByteArray( { 0x01, 0x02, 0x00 } ), packets[ 0 ] );
    EXPECT_EQ( ByteArray( { 0x01, 0x00 } ), packets[ 1 ] );
}

TEST_F( PacketizeTest, EmptyTest )
{
    // Pass in an empty array for newRxData
    auto newRxData = ByteQueue( {} );
    auto existingRxData = ByteQueue();
    std::vector<ByteArray> packets;
    ByteArray packet;
    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 0, packets.size() );
}

#if defined( ESF_REJECT_INCOMPLETE_PACKETS )
#pragma message( "Some tests have been removed due to ESF_REJECT_INCOMPLETE_PACKETS being defined." )
#else
TEST_F( PacketizeTest, SegmentedDataTest )
{
    auto newRxData = ByteQueue( { 0x01, 0x02, 0x03 } );
    auto existingRxData = ByteQueue();
    std::vector<ByteArray> packets;
    ByteArray packet;

    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 0, packets.size() );
    EXPECT_EQ( ByteQueue( { 0x01, 0x02, 0x03 } ), existingRxData );

    // Now add a EOF byte + start of next packet (which should complete the
    // packet partially received above)
    newRxData = ByteQueue( { 0x00, 0xAA, 0xAB } );
    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 1, packets.size() );
    EXPECT_EQ( ByteArray( { 0x01, 0x02, 0x03, 0x00 } ), packets[ 0 ] );
    EXPECT_EQ( ByteQueue( { 0xAA, 0xAB } ), existingRxData );

    newRxData = ByteQueue( { 0x00 } );
    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 2, packets.size() );
    EXPECT_EQ( ByteArray( { 0xAA, 0xAB, 0x00 } ), packets[ 1 ] );
    EXPECT_EQ( ByteQueue( {} ), existingRxData );
}
#if 0
TEST_F( PacketizeTest, SegmentedDataTest2 )
{
    auto newRxData1 = ByteQueue( { 0x0C, 0x50, 0xD1 /*, 0x05 is missing*/ } );
    auto newRxData2 = ByteQueue( { 0x43, 0x4F, 0x4D, 0x4E, 0x44, 0x03, 0x6C, 0x78, 0x00 } );
    auto existingRxData = ByteQueue();
    std::vector<ByteArray> packets;
    ByteArray packet;

    while( Utilities::MoveRxDataInBuffer( newRxData1, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 0, packets.size() );
    EXPECT_EQ( ByteQueue( { 0x01, 0x02, 0x03 } ), existingRxData );

    // Now add a EOF byte + start of next packet (which should complete the
    // packet partially received above)
    newRxData = ByteQueue( { 0x00, 0xAA, 0xAB } );
    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 1, packets.size() );
    EXPECT_EQ( ByteArray( { 0x01, 0x02, 0x03, 0x00 } ), packets[ 0 ] );
    EXPECT_EQ( ByteQueue( { 0xAA, 0xAB } ), existingRxData );

    newRxData = ByteQueue( { 0x00 } );
    while( Utilities::MoveRxDataInBuffer( newRxData, existingRxData, packet ), !packet.empty() )
    {
        packets.push_back( packet );
    }

    EXPECT_EQ( 2, packets.size() );
    EXPECT_EQ( ByteArray( { 0xAA, 0xAB, 0x00 } ), packets[ 1 ] );
    EXPECT_EQ( ByteQueue( {} ), existingRxData );
}
#endif
#endif

}  // namespace
