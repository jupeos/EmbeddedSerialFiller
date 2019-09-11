/**
 * \file    CallbackTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "gtest/gtest.h"
#include "SerialFiller/SerialFiller.hpp"

using namespace esf;

namespace {

    class CallbackTests : public ::testing::Test {
    protected:

        CallbackTests() {
        }

        virtual ~CallbackTests() {
        }
    };

    TEST_F(CallbackTests, CallbackTest1) {


        EmbeddedSerialFiller serialFiller;

        ByteQueue savedTxData;
        serialFiller.txDataReady_ = ([&](ByteQueue txData) -> void {
            savedTxData = txData;
        });

        serialFiller.Publish("test-topic", ByteArray({ 'h', 'e', 'l', 'l', 'o' }));

        EXPECT_NE(0, savedTxData.size());
    }

}  // namespace