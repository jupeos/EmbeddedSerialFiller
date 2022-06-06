/**
 * \file    AddAndVerifyCrcTests.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/Utilities.h"
#include "gtest/gtest.h"

using namespace esf;

namespace
{
class AddAndVerifyCrcTests : public ::testing::Test
{
   protected:
    AddAndVerifyCrcTests() {}

    virtual ~AddAndVerifyCrcTests() {}
};

TEST_F( AddAndVerifyCrcTests, BasicTest )
{
    ByteArray packet( { 0x01, 0x02, 0x03 } );
    Utilities::AddCrc( packet );
    EXPECT_EQ( ByteArray( { 0x01, 0x02, 0x03, 0xAD, 0xAD } ), packet );
    EXPECT_EQ( StatusCode::SUCCESS, Utilities::VerifyCrc( packet ) );
}

TEST_F( AddAndVerifyCrcTests, StandardCrcValTest )
{
    ByteArray packet( { '1', '2', '3', '4', '5', '6', '7', '8', '9' } );
    Utilities::AddCrc( packet );
    EXPECT_EQ( ByteArray( { '1', '2', '3', '4', '5', '6', '7', '8', '9', 0x29, 0xB1 } ), packet );
    EXPECT_EQ( StatusCode::SUCCESS, Utilities::VerifyCrc( packet ) );
}

TEST_F( AddAndVerifyCrcTests, BadCrcTest )
{
    // Provide incorrect CRC
    EXPECT_EQ( StatusCode::ERROR_CRC_CHECK_FAILED, Utilities::VerifyCrc( ByteArray( { 0x01, 0x02, 0x03, 0xAD, 0xAE } ) ) );
}

}  // namespace
