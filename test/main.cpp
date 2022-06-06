/**
 * \file    main.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "gtest/gtest.h"

int main( int argc, char** argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
