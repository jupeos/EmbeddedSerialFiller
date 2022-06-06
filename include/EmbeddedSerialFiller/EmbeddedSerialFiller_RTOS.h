/**
 * \file    EmbeddedSerialFiller.h
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#ifndef ESF_EMBEDDED_SERIAL_FILLER_H
#define ESF_EMBEDDED_SERIAL_FILLER_H

#include <etl/delegate.h>

#include <chrono>
#include <cstdint>
#include <iostream>

#include "EmbeddedSerialFiller/Definitions.h"
#include "esf_abstraction.h"

namespace esf
{
/// \brief This EmbeddedSerialFiller class represents a single serial node.
/// \details
/// Packet format, pre COBS encoded:
/// [ <packet type>, <packet type dependent data...> ]
///
/// Broadcast/Publish packet structure:
/// [ 0x01/0x04, <packet ID>, <length of topic id>, <topic ID 1>, ..., <topic ID n>, <data 1>, ... , <data n>, <CRC MSB>, <CRC LSB> ]
/// Acknowledge packet Structure:
/// [ 0x02, <packet ID>, <CRC MSB>, <CRC LSB> ]
///
/// This is then COBS encoded, which frames the end-of-packet with a unique 0x00 byte,
/// and escapes all pre-existing 0x00's present in packet.
class EmbeddedSerialFiller
{
   public:
    /// \brief      Basic constructor.
    EmbeddedSerialFiller();

    /// \brief      Publishes data on a topic, and then immediately returns. Does not block (see PublishWait()).
    uint8_t Publish( const Topic& topic, const ByteArray& data );

    /// \brief      Publishes data on a topic, and then blocks the calling thread until either an acknowledge
    ///             is received, or a timeout occurs.
    /// \returns    True if an acknowledge was received before the timeout occurred, otherwise false.
    PublishResponse PublishWait( const Topic& topic, const ByteArray& data, size_t timeout );

    /// \brief      Call to subscribe to a particular topic.
    /// \returns    A unique subscription ID which can be used to delete the subsriber.
    uint32_t Subscribe( const Topic& topic, etl::delegate<void( ByteArray& )> callback );

    /// \brief      Unsubscribes a subscriber using the provided ID.
    /// \details    ID is returned from #Subscribe() method.
    StatusCode Unsubscribe( uint32_t subscriberId );

    /// \brief      Unsubscribes all subscribers.
    void UnsubscribeAll();

    /// \brief      Pass in received RX data to EmbeddedSerialFiller.
    /// \details    EmbeddedSerialFiller will add this data to it's internal RX data buffer, and then
    ///             attempt to find and extract valid packets. If EmbeddedSerialFiller finds valid packets,
    ///             it will then call all callbacks associated with that topic.
    StatusCode GiveRxData( ByteArray& rxData );

    /// \brief      Use to enable/disable thread safety (enabled by default). Enabling thread safety makes all EmbeddedSerialFiller API
    ///             methods take out a lock on enter, and release on exit. PublishWait() releases lock when it blocks (so
    ///             PublishWait() can be called multiple times from different threads).
    void SetThreadSafetyEnabled( bool value );

    /// \brief      Call to find out how many threads are currently waiting on ACKs for this node.
    uint32_t NumThreadsWaiting();

    /// \brief      This is called by EmbeddedSerialFiller whenever it has data that is ready
    ///             to be sent out of the serial port.
    etl::delegate<void( const ByteArray& )> txDataReady_;

    /// \brief      This is called whenever a valid message is received, but
    ///             there are no subscribers listening to it.
    etl::delegate<void( const Topic& topic, const ByteArray& data )> noSubscribersForTopic_;

    uint8_t NextPacketID() { return nextPacketId_; }

   private:
    /// \brief      Stores received data until a packet EOF is received, at which point the packet is
    ///             processed.
    ByteArray rxBuffer_;

    struct Subscriber
    {
        uint32_t id_;
        etl::delegate<void( ByteArray& )> callback_;
    };

    struct SubscriberType
    {
        Topic topic;
        etl::vector<Subscriber, ESF_MAX_SUBSCRIBERS> subscribers;
    };
    typedef etl::vector<SubscriberType, ESF_MAX_SUBSCRIBERS> SubscriberList;
    SubscriberList subscribers_;

    /// \brief      Stores what the next sent packet ID should be.
    uint8_t nextPacketId_;

    /// \brief      Mutex that provides thread safety for the EmbeddedSerialFiller class.
    /// \details    Only used if thread safety is enabled via SetThreadSafetyEnabled().
    static ESF_MUTEX classMutex_;

    bool threadSafetyEnabled_;

    struct AckEvent
    {
        AckEvent() : packetId( 0 )
        {
        }
        uint8_t packetId;
        ESF_CONDITION_VARIABLE cv;
    };

    etl::vector<AckEvent*, ESF_MAX_PENDING_ACKS> ackEvents_;
    AckEvent events[ ESF_MAX_PENDING_ACKS ];

    int32_t maxAckPacketIndex;

    /// \brief      Holds the value of the next ID that will be assigned when Subscribe() is called.
    uint32_t nextFreeSubsriberId_;

    /// \brief      Internal publish method which does not lock the classMutex_.
    uint8_t PublishInternal( const PacketType& packetType, uint8_t& packetId, const Topic* topic = nullptr, const ByteArray* data = nullptr );
};

}  // namespace esf

#endif  // #ifndef ESF_EMBEDDED_SERIAL_FILLER_H
