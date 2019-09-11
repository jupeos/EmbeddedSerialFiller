/**
 * \file    PacketizeTest.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "gtest/gtest.h"
#include "SerialFiller/SerialFiller.hpp"
#include "SerialFiller/SerialFillerHelper.hpp"

using namespace esf;

namespace {

    class PacketizeTest : public ::testing::Test {
    protected:

        PacketizeTest() {
        }

        virtual ~PacketizeTest() {
        }
    };

    TEST_F(PacketizeTest, SimplePacket) {

        auto newRxData = ByteQueue({ 0x01, 0x02, 0x00 });
        auto existingRxData = ByteQueue();

        std::vector<ByteArray> packets;
        ByteArray packet;

        while(Utilities::MoveRxDataInBuffer(newRxData, existingRxData, packet), !packet.empty()) {
            packets.push_back(packet);
        }

        EXPECT_EQ(1, packets.size());
        EXPECT_EQ(ByteArray({ 0x01, 0x02, 0x00 }), packets[0]);
    }

    TEST_F(PacketizeTest, TwoPackets) {

        auto newRxData = ByteQueue({ 0x01, 0x02, 0x00, 0x01, 0x00 });
        auto existingRxData = ByteQueue();

        std::vector<ByteArray> packets;
        ByteArray packet;

        while(Utilities::MoveRxDataInBuffer(newRxData, existingRxData, packet), !packet.empty()) {
            packets.push_back(packet);
        }

        EXPECT_EQ(2, packets.size());
        EXPECT_EQ(ByteArray({ 0x01, 0x02, 0x00 }), packets[0]);
        EXPECT_EQ(ByteArray({ 0x01, 0x00 }), packets[1]);
    }

    TEST_F(PacketizeTest, EmptyTest) {
        // Pass in an empty array for newRxData
        auto newRxData = ByteQueue({});
        auto existingRxData = ByteQueue();
        std::vector<ByteArray> packets;
        ByteArray packet;
        while(Utilities::MoveRxDataInBuffer(newRxData, existingRxData, packet), !packet.empty()) {
            packets.push_back(packet);
        }


        EXPECT_EQ(0, packets.size());
    }

    TEST_F(PacketizeTest, SegmenteddataTest) {
        auto newRxData = ByteQueue({0x01, 0x02, 0x03});
        auto existingRxData = ByteQueue();
        std::vector<ByteArray> packets;
        ByteArray packet;

        while(Utilities::MoveRxDataInBuffer(newRxData, existingRxData, packet), !packet.empty()) {
            packets.push_back(packet);
        }

        EXPECT_EQ(0, packets.size());
        EXPECT_EQ(ByteQueue({ 0x01, 0x02, 0x03 }), existingRxData);

        // Now add a EOF byte + start of next packet (which should complete the
        // packet partially received above)
        newRxData = ByteQueue({0x00, 0xAA, 0xAB});
        while(Utilities::MoveRxDataInBuffer(newRxData, existingRxData, packet), !packet.empty()) {
            packets.push_back(packet);
        }

        EXPECT_EQ(1, packets.size());
        EXPECT_EQ(ByteArray({ 0x01, 0x02, 0x03, 0x00 }), packets[0]);
        EXPECT_EQ(ByteQueue({ 0xAA, 0xAB }), existingRxData);

        newRxData = ByteQueue({0x00});
        while(Utilities::MoveRxDataInBuffer(newRxData, existingRxData, packet), !packet.empty()) {
            packets.push_back(packet);
        }

        EXPECT_EQ(2, packets.size());
        EXPECT_EQ(ByteArray({ 0xAA, 0xAB, 0x00 }), packets[1]);
        EXPECT_EQ(ByteQueue({ }), existingRxData);
    }

}  // namespace