/**
 * \file    Definitions.h
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#ifndef ESF_DEFINITIONS_H
#define ESF_DEFINITIONS_H

#include <etl/cstring.h>
#include <etl/vector.h>

#define MAX_PACKET_SIZE 1024
#define MAX_TOPIC_LENGTH 16
#define MAX_SUBSCRIBERS 8
#define MAX_PENDING_ACKS 8

#if defined( PROFILE_NO_RTOS )
#define ESF_MINIMAL_IMPLEMENTATION
#else
// Uncomment to reduce the footprint.
//#define ESF_MINIMAL_IMPLEMENTATION
#endif

namespace esf
{
using ByteArray = etl::vector<uint8_t, MAX_PACKET_SIZE>;
using ByteQueue = ByteArray;
using Topic = etl::string<MAX_TOPIC_LENGTH>;
enum class StatusCode
{
    SUCCESS,
    ERROR_CRC_CHECK_FAILED,
    ERROR_NOT_ENOUGH_BYTES,
    ERROR_UNRECOGNISED_PACKET_TYPE,
    ERROR_UNEXPECTED_ACK,
    ERROR_LENGTH_OF_TOPIC_TOO_LONG,
    ERROR_UNRECOGNISED_SUBSCRIBER,
    ERROR_ZERO_BYTE_NOT_EXPECTED
};

}  // namespace esf

#endif  // #ifndef ESF_DEFINITIONS_H
