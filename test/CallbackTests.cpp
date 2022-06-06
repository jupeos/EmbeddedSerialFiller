/**
 * \file    CallbackTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
class CallbackTests : public ::testing::Test
{
   protected:
    CallbackTests() {}

    virtual ~CallbackTests() {}
};

static ByteQueue savedTxData;

TEST_F( CallbackTests, CallbackTest1 )
{
    EmbeddedSerialFiller embeddedSF;

    embeddedSF.txDataReady_ = ( [ & ]( const ByteQueue& txData ) -> void { savedTxData = txData; } );

    embeddedSF.Publish( "test-topic", ByteArray( { 'h', 'e', 'l', 'l', 'o' } ) );

    EXPECT_NE( 0, savedTxData.size() );
}

}  // namespace
