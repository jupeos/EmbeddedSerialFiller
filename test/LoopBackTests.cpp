/**
 * \file    LoopBackTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
static ByteArray savedData1;
auto dataStore1 = []( ByteArray& data ) { savedData1 = data; };
static ByteArray savedData2;
static bool callbackCalled = false;
auto dataStore2 = []( ByteArray& data ) {
    savedData2 = data;
    callbackCalled = true;
};

// The fixture for testing class Foo.
class LoopBackTests : public ::testing::Test
{
   public:
    void noSubscriberHandler( const Topic& topic, const ByteArray& data )
    {
        noSubscribersForTopicEventFired = true;
        savedNoSubscriberTopic = topic;
        savedNoSubscriberData = data;
    }

   protected:
    EmbeddedSerialFiller embeddedSF;
    bool noSubscribersForTopicEventFired;
    Topic savedNoSubscriberTopic;
    ByteArray savedNoSubscriberData;

    LoopBackTests()
    {
        // Connect output to input (software loopback)
        embeddedSF.txDataReady_ = etl::delegate<void( const ByteQueue& )>::create<LoopBackTests, &LoopBackTests::loopbackHandler>( *this );
        embeddedSF.SetThreadSafetyEnabled( false );
    }

    void loopbackHandler( const ByteQueue& data ) { embeddedSF.GiveRxData( const_cast<ByteQueue&>( data ) ); }

    virtual ~LoopBackTests() {}
};

TEST_F( LoopBackTests, SingleTopicTest )
{
    Topic topic;
    auto data = ByteArray();

    // Subscribe to a test topic
    embeddedSF.Subscribe( "test-topic", etl::delegate<void( ByteArray & data )>( dataStore1 ) );

    // Publish data on topic
    embeddedSF.Publish( "test-topic", { 'h', 'e', 'l', 'l', 'o' } );

    EXPECT_EQ( ByteArray( { 'h', 'e', 'l', 'l', 'o' } ), savedData1 );
}

TEST_F( LoopBackTests, DataWithZerosTest )
{
    Topic topic;
    auto data = ByteArray();

    // Subscribe to a test topic
    embeddedSF.Subscribe( "test-topic", etl::delegate<void( ByteArray & data )>( dataStore1 ) );

    // Publish data on topic
    embeddedSF.Publish( "test-topic", ByteArray( { '\x00', '\x00' } ) );

    EXPECT_EQ( ByteArray( { '\x00', '\x00' } ), savedData1 );
}

TEST_F( LoopBackTests, MultiTopicTest )
{
    Topic topic;
    auto data = ByteArray();

    // Subscribe to some topics (sharing the data object)
    embeddedSF.Subscribe( "topic1", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    embeddedSF.Subscribe( "topic2", etl::delegate<void( ByteArray & data )>( dataStore1 ) );

    // Publish data on topic
    embeddedSF.Publish( "topic1", { 'h', 'e', 'l', 'l', 'o' } );
    EXPECT_EQ( ByteArray( { 'h', 'e', 'l', 'l', 'o' } ), savedData1 );
    savedData1.clear();

    embeddedSF.Publish( "topic2", { 'w', 'o', 'r', 'l', 'd' } );
    EXPECT_EQ( ByteArray( { 'w', 'o', 'r', 'l', 'd' } ), savedData1 );
    savedData1.clear();

    // Publish on topic we haven't registered on
    embeddedSF.Publish( "topic3", {} );
    EXPECT_EQ( ByteArray{}, savedData1 );
    savedData1.clear();
}

TEST_F( LoopBackTests, LargeAmountOfDataTest )
{
    Topic topic;
    // 419 bytes
    auto txData = ByteArray( { 0x0A, 0x2D, 0x0A, 0x10, 0x74, 0x74, 0x30, 0x61, 0x61, 0x61, 0x31, 0x36, 0x37, 0x37, 0x31, 0x37,
                               0x31, 0x34, 0x35, 0x32, 0x11, 0x27, 0x9D, 0x05, 0x80, 0x3E, 0x39, 0xCD, 0x3F, 0x19, 0x9B, 0x2A,
                               0xDC, 0x57, 0xE0, 0x41, 0x5B, 0xC0, 0x21, 0x7B, 0x14, 0xAE, 0x47, 0xE1, 0x7A, 0x84, 0x3F, 0x0A,
                               0x2C, 0x0A, 0x0F, 0x74, 0x74, 0x31, 0x61, 0x61, 0x61, 0x37, 0x37, 0x30, 0x35, 0x39, 0x33, 0x34,
                               0x31, 0x30, 0x11, 0x85, 0x98, 0x31, 0xC7, 0x5A, 0xED, 0x08, 0x40, 0x19, 0x84, 0x4D, 0x8E, 0xBC,
                               0x61, 0x2F, 0x5A, 0xC0, 0x21, 0x7B, 0x14, 0xAE, 0x47, 0xE1, 0x7A, 0x84, 0x3F, 0x0A, 0x2C, 0x0A,
                               0x0F, 0x74, 0x74, 0x32, 0x61, 0x61, 0x61, 0x31, 0x32, 0x31, 0x35, 0x31, 0x32, 0x32, 0x35, 0x30,
                               0x11, 0x4C, 0x4B, 0xF3, 0x4F, 0xBD, 0x6E, 0x0D, 0x40, 0x19, 0xFF, 0xCF, 0x5F, 0xD3, 0x21, 0x05,
                               0x5A, 0xC0, 0x21, 0x7B, 0x14, 0xAE, 0x47, 0xE1, 0x7A, 0x84, 0x3F, 0x0A, 0x2C, 0x0A, 0x0F, 0x74,
                               0x74, 0x33, 0x61, 0x61, 0x61, 0x35, 0x35, 0x38, 0x37, 0x33, 0x36, 0x36, 0x36, 0x35, 0x11, 0x35,
                               0xEF, 0xF6, 0x99, 0x0A, 0x8E, 0x0C, 0x40, 0x19, 0x5E, 0x03, 0x55, 0x20, 0x44, 0x43, 0x5A, 0xC0,
                               0x21, 0x7B, 0x14, 0xAE, 0x47, 0xE1, 0x7A, 0x84, 0x3F, 0x0A, 0x2C, 0x0A, 0x0F, 0x74, 0x74, 0x34,
                               0x61, 0x61, 0x61, 0x32, 0x39, 0x34, 0x39, 0x34, 0x39, 0x34, 0x34, 0x32, 0x11, 0xD3, 0x36, 0x65,
                               0xDB, 0x75, 0xFF, 0x0C, 0x40, 0x19, 0xC1, 0x8C, 0x85, 0xED, 0x74, 0x5D, 0x5A, 0xC0, 0x21, 0x7B,
                               0x14, 0xAE, 0x47, 0xE1, 0x7A, 0x84, 0x3F, 0x0A, 0x2D, 0x0A, 0x10, 0x74, 0x74, 0x35, 0x61, 0x61,
                               0x61, 0x32, 0x30, 0x33, 0x31, 0x38, 0x34, 0x33, 0x36, 0x34, 0x33, 0x11, 0xD3, 0x3E, 0xEF, 0x32,
                               0x61, 0x94, 0x11, 0x40, 0x19, 0xD5, 0x3D, 0x98, 0x4B, 0x86, 0x89, 0x5B, 0xC0, 0x21, 0x7B, 0x14,
                               0xAE, 0x47, 0xE1, 0x7A, 0x84, 0x3F, 0x0A, 0x2D, 0x0A, 0x10, 0x74, 0x74, 0x36, 0x61, 0x61, 0x61,
                               0x31, 0x36, 0x35, 0x30, 0x30, 0x35, 0x36, 0x34, 0x34, 0x34, 0x11, 0x4C, 0x67, 0x44, 0xAB, 0xC4,
                               0x47, 0xE1, 0x7A, 0x84, 0x3F, 0x0A, 0x2D, 0x0A, 0x10, 0x74, 0x74, 0x37, 0x61, 0x61, 0x61, 0x31,
                               0x31, 0x30, 0x30, 0x37, 0x33, 0x32, 0x33, 0x32, 0x31, 0x11, 0xE4, 0x11, 0xA8, 0x9C, 0x02, 0x5B,
                               0x15, 0x40, 0x19, 0x79, 0x2D, 0x55, 0x65, 0x2B, 0xA1, 0x5B, 0xC0, 0x21, 0x7B, 0x14, 0xAE, 0x47,
                               0xE1, 0x7A, 0x84, 0x3F, 0x0A, 0x2D, 0x0A, 0x10, 0x74, 0x74, 0x38, 0x61, 0x61, 0x61, 0x31, 0x35,
                               0x32, 0x30, 0x38, 0x33, 0x38, 0x39, 0x38, 0x35, 0x11, 0x38, 0x02, 0x35, 0xDA, 0x68, 0x6D, 0x1B,
                               0x40, 0x19, 0x10, 0x39, 0xE2, 0x25, 0xA9, 0xE5, 0x5A, 0xC0, 0x21, 0x7B, 0x14, 0xAE, 0x47, 0xE1,
                               0x7A, 0x84, 0x3F } );

    // Subscribe to some topics (sharing the data object)
    embeddedSF.Subscribe( "ATopicWithLotsOfData", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    embeddedSF.Subscribe( "topic2", etl::delegate<void( ByteArray & data )>( dataStore1 ) );

    noSubscribersForTopicEventFired = false;
    savedNoSubscriberTopic = "";
    savedNoSubscriberData.clear();
    embeddedSF.noSubscribersForTopic_ = etl::delegate<void( const Topic& topic, const ByteArray& data )>::create<LoopBackTests, &LoopBackTests::noSubscriberHandler>( *this );

    // Publish data on topic
    embeddedSF.Publish( "ATopicWithLotsOfData", txData );
    EXPECT_EQ( txData, savedData1 );
    savedData1.clear();

    embeddedSF.Publish( "topic2", { 'w', 'o', 'r', 'l', 'd' } );
    EXPECT_EQ( ByteArray( { 'w', 'o', 'r', 'l', 'd' } ), savedData1 );
    savedData1.clear();

    EXPECT_FALSE( noSubscribersForTopicEventFired );

    // Publish on topic we haven't subscribed to
    embeddedSF.Publish( "topic3", {} );
    EXPECT_EQ( ByteArray{}, savedData1 );
    savedData1.clear();

    EXPECT_TRUE( noSubscribersForTopicEventFired );
    EXPECT_EQ( "topic3", savedNoSubscriberTopic );
    EXPECT_EQ( ByteArray{}, savedNoSubscriberData );
}

TEST_F( LoopBackTests, UnsubscribeCorrectId )
{
    Topic topic;
    auto data = ByteArray();

    // Subscribe to some topics (sharing the data object)
    embeddedSF.Subscribe( "topic1", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    auto topic2Id = embeddedSF.Subscribe( "topic2", etl::delegate<void( ByteArray & data )>( dataStore1 ) );

    // Publish data on topic
    embeddedSF.Publish( "topic1", { 'h', 'e', 'l', 'l', 'o' } );
    EXPECT_EQ( ByteArray( { 'h', 'e', 'l', 'l', 'o' } ), savedData1 );
    savedData1.clear();

    embeddedSF.Publish( "topic2", { 'w', 'o', 'r', 'l', 'd' } );
    EXPECT_EQ( ByteArray( { 'w', 'o', 'r', 'l', 'd' } ), savedData1 );
    savedData1.clear();

    // Now unsubscribe to topic2
    embeddedSF.Unsubscribe( topic2Id );

    // We should now not get any data when publishing to topic2
    embeddedSF.Publish( "topic2", { 'w', 'o', 'r', 'l', 'd' } );
    EXPECT_EQ( ByteArray( {} ), savedData1 );
    savedData1.clear();
}

TEST_F( LoopBackTests, UnsubscribeWrongId )
{
    Topic topic;
    auto data = ByteArray();

    // Subscribe to some topics (sharing the data object)
    embeddedSF.Subscribe( "topic1", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    auto topic2Id = embeddedSF.Subscribe( "topic2", etl::delegate<void( ByteArray & data )>( dataStore1 ) );

    // Attempt to unsubscribe to a ID 1 higher than previously assigned ID
    EXPECT_EQ( embeddedSF.Unsubscribe( topic2Id + 1 ), StatusCode::ERROR_UNRECOGNISED_SUBSCRIBER );
}

TEST_F( LoopBackTests, UnsubscribeAll )
{
    Topic topic;
    auto data = ByteArray();

    // Subscribe to some topics (sharing the data object)
    embeddedSF.Subscribe( "topic1", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    embeddedSF.Subscribe( "topic2", etl::delegate<void( ByteArray & data )>( dataStore1 ) );

    // Publish data on topic (this is before unsubscribing to all)
    embeddedSF.Publish( "topic1", { 'h', 'e', 'l', 'l', 'o' } );
    EXPECT_EQ( ByteArray( { 'h', 'e', 'l', 'l', 'o' } ), savedData1 );
    savedData1.clear();

    embeddedSF.Publish( "topic2", { 'w', 'o', 'r', 'l', 'd' } );
    EXPECT_EQ( ByteArray( { 'w', 'o', 'r', 'l', 'd' } ), savedData1 );
    savedData1.clear();

    // Now unsubscribe to all topics
    embeddedSF.UnsubscribeAll();

    // We should now not get any data when publishing to topic 1 or 2
    embeddedSF.Publish( "topic1", { 'w', 'o', 'r', 'l', 'd' } );
    EXPECT_EQ( ByteArray( {} ), savedData1 );
    savedData1.clear();

    embeddedSF.Publish( "topic2", { 'w', 'o', 'r', 'l', 'd' } );
    EXPECT_EQ( ByteArray( {} ), savedData1 );
    savedData1.clear();
}

TEST_F( LoopBackTests, NoDataTest )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Subscribe to a test topic
    embeddedSF.Subscribe( "test-topic", etl::delegate<void( ByteArray & data )>( dataStore2 ) );

    // Publish on topic, no data
    embeddedSF.Publish( "test-topic", {} );

    EXPECT_EQ( true, callbackCalled );
    EXPECT_EQ( ByteArray( {} ), savedData2 );
}

}  // namespace
