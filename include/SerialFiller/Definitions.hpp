///
/// \file 				Definitions.hpp
/// \author 			Geoffrey Hunter <gbmhunter@gmail.com> (www.mbedded.ninja)
/// \edited             n/a
/// \created			2018-01-25
/// \last-modified		2018-01-25
/// \brief 				Contains definitions that are used in various serial filler modules.
/// \details
///		See README.md in root dir for more info.

#ifndef MN_SERIAL_FILLER_DEFINITIONS_H_
#define MN_SERIAL_FILLER_DEFINITIONS_H_

#include <etl/cstring.h>
#include <etl/vector.h>

#define MAX_PACKET_SIZE 512
#define MAX_TOPIC_LENGTH 16
#define MAX_SUBSCRIBERS 8
#define MAX_PENDING_ACKS 8

namespace mn {
namespace SerialFiller {

using ByteArray = etl::vector<uint8_t, MAX_PACKET_SIZE>;
using ByteQueue = ByteArray;
using Topic     = etl::string<MAX_TOPIC_LENGTH>;
enum class StatusCode { SUCCESS, ERROR_CRC_CHECK_FAILED, ERROR_NOT_ENOUGH_BYTES, ERROR_UNRECOGNISED_PACKET_TYPE, ERROR_AUTO_ACK_DISABLED, ERROR_LENGTH_OF_TOPIC_TOO_LONG, ERROR_UNRECOGNISED_SUBSCRIBER, ERROR_ZERO_BYTE_NOT_EXPECTED };

} // namespace SerialFiller
} // namespace mn

#endif // #ifndef MN_SERIAL_FILLER_DEFINITIONS_H_
