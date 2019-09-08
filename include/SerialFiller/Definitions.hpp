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

#define MAX_PACKET_SIZE 64
#define MAX_TOP_LENGTH 8
#define MAX_SUBSCRIBERS 8
#define MAX_PENDING_ACKS 8
// Log related.
#define MAX_LOGGER_NAME_LENGTH 32
#define MAX_LOG_LINE_LENGTH 512
#define MAX_LOG_MESSAGE_LENGTH 32
#define MAX_LOG_FILE_NAME_LENGTH 32
#define MAX_LOG_FUNCTION_NAME_LENGTH 32
#define MSG(X) etl::string<MAX_LOG_MESSAGE_LENGTH>(X)

#define MAX_HEX_STRING_LENGTH 16
#define MAX_ASCII_STRING_LENGTH 16

namespace mn {
namespace SerialFiller {
using ByteArray = etl::vector<uint8_t, MAX_PACKET_SIZE>;
using ByteQueue = ByteArray;
using Topic     = etl::string<MAX_TOP_LENGTH>;
} // namespace SerialFiller
} // namespace mn

#endif // #ifndef MN_SERIAL_FILLER_DEFINITIONS_H_
