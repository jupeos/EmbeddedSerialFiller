/**
 * \file    NoSubscribersTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
auto emptyLambda = []( ByteArray& data ) {};

class NoSubscribersTests : public ::testing::Test
{
   protected:
    EmbeddedSerialFiller embeddedSF;

    NoSubscribersTests()
    {
        // Connect output to input (software loopback)
        embeddedSF.txDataReady_ = etl::delegate<void( const ByteQueue& )>::create<NoSubscribersTests, &NoSubscribersTests::loopbackHandler>( *this );
        embeddedSF.SetThreadSafetyEnabled( false );
    }

    ByteArray loopbackData;
    void loopbackHandler( const ByteQueue& data ) { loopbackData = data; }

    virtual ~NoSubscribersTests() {}
};

static bool listenerCalled = false;
static Topic savedTopic;
static ByteArray savedData;

TEST_F( NoSubscribersTests, NoSubscribersEventFired )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Add listener to "no subscribers for topic" event

    embeddedSF.noSubscribersForTopic_ = [ & ]( Topic topic, ByteArray data ) {
        listenerCalled = true;
        savedTopic = topic;
        savedData = data;
    };

    // Publish data on topic
    embeddedSF.Publish( "BogusTopic", { 'h', 'e', 'l', 'l', 'o' } );
    embeddedSF.GiveRxData( loopbackData );

    // Since there was no subscribers, the "no subscribers for topic" event
    // should of fired!
    EXPECT_TRUE( listenerCalled );
    EXPECT_EQ( "BogusTopic", savedTopic );
    EXPECT_EQ( ByteArray( { 'h', 'e', 'l', 'l', 'o' } ), savedData );
}

TEST_F( NoSubscribersTests, NoSubscribersEventNotFired )
{
    auto topic = ByteArray();
    auto data = ByteArray();

    // Add listener to "no subscribers for topic" event
    bool listenerCalled = false;
    Topic savedTopic;
    ByteArray savedData;
    embeddedSF.noSubscribersForTopic_ = [ & ]( Topic topic, ByteArray data ) {
        listenerCalled = true;
        savedTopic = topic;
        savedData = data;
    };

    // Subscribe to TestTopic, so that the "no subscribers" event
    // should not be fired
    embeddedSF.Subscribe( "TestTopic", etl::delegate<void( ByteArray & data )>( emptyLambda ) );

    // Publish data on topic
    embeddedSF.Publish( "TestTopic", { 'h', 'e', 'l', 'l', 'o' } );
    embeddedSF.GiveRxData( loopbackData );

    EXPECT_FALSE( listenerCalled );
}

}  // namespace
