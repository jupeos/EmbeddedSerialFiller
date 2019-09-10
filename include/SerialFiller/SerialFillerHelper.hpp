///
/// \file 				SerialFillerHelper.hpp
/// \author 			Geoffrey Hunter <gbmhunter@gmail.com> (www.mbedded.ninja)
/// \edited             n/a
/// \created			2017-06-10
/// \last-modified		2018-01-30
/// \brief 				Contains the SerialFillerHelper class.
/// \details
///		See README.rst in root dir for more info.

#ifndef MN_SERIAL_FILLER_SERIAL_FILLER_HELPER_H_
#define MN_SERIAL_FILLER_SERIAL_FILLER_HELPER_H_

#include "SerialFiller/Definitions.hpp"
#include <cstdint>

namespace mn {
namespace SerialFiller {

class SerialFillerHelper
{
public:
    static StatusCode SplitPacket(const ByteArray& packet, uint32_t startAt, Topic& topic, ByteArray& data);

    /// \details    Moves new RX data into the RX buffer, while looking for the
    ///             end-of-frame character. If EOF is found, packet is populated
    ///             and this method returns.
    static void MoveRxDataInBuffer(ByteArray& newRxData, ByteArray& rxDataBuffer, ByteArray& packet);

    static void AddCrc(ByteArray& packet);

    /// \param  packet  Packet must be COBS decoded before passing into here. Expects
    ///                 last two bytes to be the CRC value of all the bytes proceeding it.
    static StatusCode VerifyCrc(const ByteArray& packet);

private:
};
} // namespace SerialFiller
} // namespace mn

#endif // #ifndef MN_SERIAL_FILLER_SERIAL_FILLER_HELPER_H_
