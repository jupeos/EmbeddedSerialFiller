/**
 * \file    Definitions.h
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#ifndef ESF_DEFINITIONS_H
#define ESF_DEFINITIONS_H

#include <etl/string.h>
#include <etl/vector.h>

#ifndef ESF_MAX_PACKET_SIZE
#define ESF_MAX_PACKET_SIZE 1024
#endif
#ifndef ESF_MAX_TOPIC_LENGTH
#define ESF_MAX_TOPIC_LENGTH 16
#endif
#ifndef ESF_MAX_SUBSCRIBERS
#define ESF_MAX_SUBSCRIBERS 8
#endif
#ifndef ESF_MAX_PENDING_ACKS
#define ESF_MAX_PENDING_ACKS 8
#endif

#define ESF_OPTIMISE // Optimisation is on by default.

#if defined(PROFILE_NO_RTOS)
#define ESF_MINIMAL_IMPLEMENTATION
#else
// Uncomment to reduce the footprint.
//#define ESF_MINIMAL_IMPLEMENTATION
#endif

// This disallows partial packet receptions.
//#define ESF_REJECT_INCOMPLETE_PACKETS

namespace esf
{
    using ByteArray = etl::vector<uint8_t, ESF_MAX_PACKET_SIZE>;
    using ByteQueue = ByteArray;
    using Topic = etl::string<ESF_MAX_TOPIC_LENGTH>;

    /**
 * \enum PacketType
 * \brief Enumerates the available EmbeddedSerialFiller packet types.
 */
    enum class PacketType : uint8_t
    {
        UNKNOWN = 0x0,
        BROADCAST = 0x42, /* 'B' No response expected */
        ACK = 0x41,       /* 'A' Acknowledge */
        PUBLISH = 0x50,   /* 'P' Expects an ACK response */
    };

    /**
 * \enum PublishResponse
 * \brief Response codes to a *PublishWait*.
 */
    enum class PublishResponse : uint8_t
    {
        UNKNOWN,
        SUCCESS,
        PENDING,
        TIMEOUT,
    };

    /**
 * \enum StatusCode
 * \brief The result of various functions within the *ESF* library.
 */
    enum class StatusCode : uint8_t
    {
        SUCCESS,
        ERROR_CRC_CHECK_FAILED,
        ERROR_NOT_ENOUGH_BYTES,
        ERROR_UNRECOGNISED_PACKET_TYPE,
        ERROR_UNEXPECTED_ACK,
        ERROR_LENGTH_OF_TOPIC_TOO_LONG,
        ERROR_UNRECOGNISED_SUBSCRIBER,
        ERROR_ZERO_BYTE_NOT_EXPECTED,
        ERROR_RX_DATA_BUFFER_FULL,
#if defined(ESF_REJECT_INCOMPLETE_PACKETS)
        ERROR_PACKET_INCOMPLETE,
#endif
    };

#define ESF_MIN_BYTES (3)

} // namespace esf

#endif // #ifndef ESF_DEFINITIONS_H
