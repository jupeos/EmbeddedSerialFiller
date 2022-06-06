/**
 * \file    TwoNodeAckTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include <thread>

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "Node.h"
#include "ThreadSafeQ.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
static ByteArray savedData1;
auto dataStore1 = []( ByteArray& data ) { savedData1 = data; };
static ByteArray savedData2;
auto dataStore2 = []( ByteArray& data ) { savedData2 = data; };

// The fixture for testing class Foo.
class TwoNodeAckTests : public ::testing::Test
{
   public:
    void operator()( ByteArray& data )
    {
        // This should test that the node2 mutex is unlocked before calling any subscribed callbacks
        // otherwise we would get into deadlock on this call
        node2_.embeddedSF.Publish( "response", { 0x02 } );
        savedData2 = data;
    }

   protected:
    Node node1_;
    Node node2_;

    TwoNodeAckTests()
        : node1_( "node1" ), node2_( "node2" )
    {
        // Connect node 1 output to node 2 input and vise versa
        node1_.embeddedSF.txDataReady_ = etl::delegate<void( const ByteQueue& )>::create<TwoNodeAckTests, &TwoNodeAckTests::loopbackHandler1>( *this );
        node2_.embeddedSF.txDataReady_ = etl::delegate<void( const ByteQueue& )>::create<TwoNodeAckTests, &TwoNodeAckTests::loopbackHandler2>( *this );
    }

    void loopbackHandler1( const ByteQueue& data )
    {
        for( ByteQueue::const_iterator iter = data.begin(); iter != data.end(); ++iter )
        {
            node2_.rxQueue_.Push( *iter );
        }
        ByteQueue& data2 = const_cast<ByteQueue&>( data );
        data2.clear();
    }

    void loopbackHandler2( const ByteQueue& data )
    {
        for( ByteQueue::const_iterator iter = data.begin(); iter != data.end(); ++iter )
        {
            node1_.rxQueue_.Push( *iter );
        }
        ByteQueue& data2 = const_cast<ByteQueue&>( data );
        data2.clear();
    }

    virtual ~TwoNodeAckTests() {}
};

#if defined( ESF_REJECT_INCOMPLETE_PACKETS )
#pragma message( "Some tests have been removed due to ESF_REJECT_INCOMPLETE_PACKETS being defined." )
#else
TEST_F( TwoNodeAckTests, SingleTopic )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Subscribe to a test topic
    node2_.embeddedSF.Subscribe( "test-topic", etl::delegate<void( ByteArray & data )>( dataStore2 ) );

    auto dataToSend = ByteArray( { 0x01, 0x02, 0x03, 0x04 } );

    // Call PublishWait
    PublishResponse node1Result = node1_.embeddedSF.PublishWait( "test-topic", dataToSend, 1000 );

    EXPECT_TRUE( node1Result == PublishResponse::SUCCESS );
    EXPECT_EQ( dataToSend, savedData2 );
    EXPECT_EQ( 0, node1_.embeddedSF.NumThreadsWaiting() );
}

TEST_F( TwoNodeAckTests, NoResponse )
{
    /* TODO
        auto topic = ByteArray();
        auto data = ByteArray();

        // Subscribe to a test topic
        node2_.embeddedSF.Subscribe("test-topic", etl::delegate<void(ByteArray& data)>(dataStore2));

        // Publish data on topic
        auto dataToSend = ByteArray({ 0x01, 0x02, 0x03, 0x04 });
        auto gotAck = node1_.embeddedSF.PublishWait("test-topic", dataToSend, std::chrono::milliseconds(1000));
        EXPECT_FALSE(gotAck);
        EXPECT_EQ(dataToSend, savedData2);
         * */
}

TEST_F( TwoNodeAckTests, TwoWayTest )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Subscribe to a test topic
    node1_.embeddedSF.Subscribe( "test-topic", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    node2_.embeddedSF.Subscribe( "test-topic", etl::delegate<void( ByteArray & data )>( dataStore2 ) );

    // Publish data on topic
    auto node1DataToSend = ByteArray( { 0x01, 0x02, 0x03, 0x04 } );
    auto node2DataToSend = ByteArray( { 0x0A, 0x0B, 0x0C, 0x0D } );

    PublishResponse node1Result;
    std::thread t1( [ & ]() { node1Result = node1_.embeddedSF.PublishWait( "test-topic", node1DataToSend, 1000 ); } );
    PublishResponse node2Result;
    std::thread t2( [ & ]() { node2Result = node2_.embeddedSF.PublishWait( "test-topic", node2DataToSend, 1000 ); } );

    t1.join();
    t2.join();

    EXPECT_TRUE( node1Result == PublishResponse::SUCCESS );
    EXPECT_TRUE( node2Result == PublishResponse::SUCCESS );
    EXPECT_EQ( node1DataToSend, savedData2 );
    EXPECT_EQ( node2DataToSend, savedData1 );
}

TEST_F( TwoNodeAckTests, TwoPublishWaitCalls )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Subscribe to two topics
    node2_.embeddedSF.Subscribe( "topic1", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    node2_.embeddedSF.Subscribe( "topic2", etl::delegate<void( ByteArray & data )>( dataStore2 ) );

    // Publish data on both topics
    auto msg1Data = ByteArray( { 0x01, 0x02, 0x03, 0x04 } );
    auto msg2Data = ByteArray( { 0x0A, 0x0B, 0x0C, 0x0D } );

    PublishResponse msg1Result;
    std::thread t1( [ & ]() { msg1Result = node1_.embeddedSF.PublishWait( "topic1", msg1Data, 1000 ); } );
    PublishResponse msg2Result;
    std::thread t2( [ & ]() { msg2Result = node1_.embeddedSF.PublishWait( "topic2", msg2Data, 1000 ); } );

    t1.join();
    t2.join();

    EXPECT_TRUE( msg1Result == PublishResponse::SUCCESS );
    EXPECT_TRUE( msg2Result == PublishResponse::SUCCESS );
    EXPECT_EQ( msg1Data, savedData1 );
    EXPECT_EQ( msg2Data, savedData2 );
}

TEST_F( TwoNodeAckTests, SubscribeCallsPublish )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Subscribe to a test topic
    node1_.embeddedSF.Subscribe( "response", etl::delegate<void( ByteArray & data )>( dataStore1 ) );
    node2_.embeddedSF.Subscribe( "request", etl::delegate<void( ByteArray & data )>( *this ) );

    PublishResponse node1Result = node1_.embeddedSF.PublishWait( "request", { 0x01 }, 5000 );

    node2_.Join();

    EXPECT_TRUE( node1Result == PublishResponse::SUCCESS );
    EXPECT_EQ( ByteArray( { 0x02 } ), savedData1 );
    EXPECT_EQ( ByteArray( { 0x01 } ), savedData2 );
}

TEST_F( TwoNodeAckTests, MultiPacket )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Subscribe to a test topic
    node2_.embeddedSF.Subscribe( "test-topic", etl::delegate<void( ByteArray & data )>( dataStore2 ) );

    auto dataToSend = ByteArray( { 0x01, 0x02, 0x03, 0x04 } );

    for( int i = 0; i < 512; ++i )
    {
        // Call PublishWait
        savedData2.clear();
        PublishResponse node1Result = node1_.embeddedSF.PublishWait( "test-topic", dataToSend, 1000 );

        EXPECT_TRUE( node1Result == PublishResponse::SUCCESS );
        EXPECT_EQ( dataToSend, savedData2 );
        EXPECT_EQ( 0, node1_.embeddedSF.NumThreadsWaiting() );
    }
}
#endif

}  // namespace
