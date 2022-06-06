/**
 * \file    EmbeddedSerialFiller_NoRTOS.h
 * \author  Julian Mitchell
 * \date    18 Apr 2020
 */

#ifndef ESF_EMBEDDED_SERIAL_FILLER_H
#define ESF_EMBEDDED_SERIAL_FILLER_H

#include <etl/delegate.h>

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

    /**
     * \brief Publishes data on a topic, and then "waits" until either an acknowledge is received, or a timeout occurs.
     * \param topic The topic
     * \param data The data
     * \param timeout Number of cycles (calls to this method) within which we expect an acknowledgement.
     * \return
     */
    PublishResponse PublishWait( const Topic& topic, const ByteArray& data, size_t timeout /* in call cycles */ );

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

    /// \brief      Call to find out if a task is currently waiting on an ACK.
    bool TaskPending();

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

    struct AckEvent
    {
        enum AckState
        {
            NO_ACK,
            ACK
        };
        AckEvent() : packetId( 0 ), state( NO_ACK )
        {
        }
        uint8_t packetId;
        AckState state;
    };

    AckEvent ackEvent;
    AckEvent events;

    /// \brief      Holds the value of the next ID that will be assigned when Subscribe() is called.
    uint32_t nextFreeSubsriberId_;

    /// \brief      Internal publish method which does not lock the classMutex_.
    uint8_t PublishInternal( const PacketType& packetType, uint8_t& packetId, const Topic* topic = nullptr, const ByteArray* data = nullptr );
};

}  // namespace esf

#endif  // #ifndef ESF_EMBEDDED_SERIAL_FILLER_H
