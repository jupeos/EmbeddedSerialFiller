/**
 * \file    EmbeddedSerialFiller_NoRTOS.cpp
 * \author  Julian Mitchell
 * \date    18 Apr 2020
 */

#include "EmbeddedSerialFiller/CobsTranscoder.h"
#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "EmbeddedSerialFiller/Utilities.h"

namespace esf
{
EmbeddedSerialFiller::EmbeddedSerialFiller() : nextPacketId_( 1 ), nextFreeSubsriberId_( 0 )
{
}

uint8_t EmbeddedSerialFiller::Publish( const Topic& topic, const ByteArray& data )
{
    return PublishInternal( PacketType::BROADCAST, nextPacketId_, &topic, &data );
}

/**
 * This method implements a very small state machine (using a local continuation) to correctly handle multiple calls
 * in order to provide a non-blocking *Publish* while waiting for an acknowledgement.
 */
#define ENTRY_POINT ( 0 )
#define CONTINUATION_POINT ( 1 )
PublishResponse EmbeddedSerialFiller::PublishWait( const Topic& topic, const ByteArray& data, size_t timeout )
{
    static uint8_t pw_state = ENTRY_POINT;  // PublishWait state variable
    static size_t timeout_count = 0;

    PublishResponse gotAck = PublishResponse::TIMEOUT;

    switch( pw_state )
    {
        case ENTRY_POINT:
            timeout_count = 0;
            // JMI review if this should be kept for production.
            assert( ackEvent.packetId == 0 );

            // Record the packet identifier we expect an ACK for.
            ackEvent.packetId = nextPacketId_;

            // Call the standard publish
            PublishInternal( PacketType::PUBLISH, nextPacketId_, &topic, &data );

            pw_state = CONTINUATION_POINT;
        // Intentional fall through
        case CONTINUATION_POINT:
            if( timeout_count < timeout )
            {
                ++timeout_count;
                if( ackEvent.state == AckEvent::ACK )
                {
                    // ACK has been received.
                    gotAck = PublishResponse::SUCCESS;
                }
                else
                {
                    // Awaiting the ACK.
                    return PublishResponse::PENDING;
                }
            }
            else
            {
                // ACK not received within the given number of cycles.
                gotAck = PublishResponse::TIMEOUT;
            }
    }
    // Reset the event.
    ackEvent.packetId = 0;
    ackEvent.state = AckEvent::NO_ACK;

    // Reset the call state.
    pw_state = ENTRY_POINT;

    return gotAck;
}

uint32_t EmbeddedSerialFiller::Subscribe( const Topic& topic, etl::delegate<void( ByteArray& )> callback )
{
    // Assign ID and update free IDs
    auto id = nextFreeSubsriberId_;
    nextFreeSubsriberId_++;

    // Save subscription
    Subscriber subscriber;
    subscriber.id_ = id;
    subscriber.callback_ = callback;
    for( auto it = subscribers_.begin(); it != subscribers_.end(); ++it )
    {
        if( it->topic == topic )
        {
            it->subscribers.push_back( subscriber );
            return id;
        }
    }
    SubscriberType st;
    st.topic = topic;
    st.subscribers.push_back( subscriber );
    subscribers_.push_back( st );
    return id;
}

#if !defined( ESF_MINIMAL_IMPLEMENTATION )
StatusCode EmbeddedSerialFiller::Unsubscribe( uint32_t subscriberId )
{
    auto retVal = StatusCode::ERROR_UNRECOGNISED_SUBSCRIBER;

    // Search for subscriber with provided ID
    for( auto it = subscribers_.begin(); it != subscribers_.end(); ++it )
    {
        for( auto subIt = it->subscribers.begin(); subIt != it->subscribers.end();
             ++subIt )
        {
            if( subIt->id_ == subscriberId )
            {
                it->subscribers.erase( subIt );
                retVal = StatusCode::SUCCESS;
                break;
            }
        }
    }

    // If we reach here, no subscriber was found!
    return retVal;
}

void EmbeddedSerialFiller::UnsubscribeAll()
{
    subscribers_.clear();
}
#endif

StatusCode EmbeddedSerialFiller::GiveRxData( ByteArray& rxData )
{
    StatusCode result = StatusCode::SUCCESS;

    ByteArray packet;
    result = Utilities::MoveRxDataInBuffer( rxData, rxBuffer_, packet );  // ~25us
    if( result == StatusCode::SUCCESS )
    {
        while( !packet.empty() )
        {
            Topic topic;

            //==============================//
            //======= FOR EACH PACKET ======//
            //==============================//

            // Remove COBS encoding
            ByteArray decodedData;
            result = CobsTranscoder::Decode( packet, decodedData );  // ~4us
            if( result == StatusCode::SUCCESS )
            {
                // Verify CRC
                result = Utilities::VerifyCrc( decodedData );
                if( result == StatusCode::SUCCESS )
                {
                    // Look at packet type
                    auto packetType = static_cast<PacketType>( decodedData[ 0 ] );
                    // Extract packet ID
                    uint8_t packetId = static_cast<uint8_t>( decodedData.at( 1 ) );
                    if( ( packetType == PacketType::BROADCAST ) || ( packetType == PacketType::PUBLISH ) )
                    {
                        // 4. Then split packet into topic and data (let's just reuse the packet container for this);
                        ByteArray& data = packet;
                        result = Utilities::SplitPacket( decodedData, 2, topic, data );
                        if( result == StatusCode::SUCCESS )
                        {
                            // WARNING: Make sure to send ack BEFORE invoking topic callbacks, as they may cause other messages
                            // to be sent, and we always want the ACK to be the first thing sent back to the sender.
                            if( packetType == PacketType::PUBLISH )
                            {
                                PublishInternal( PacketType::ACK, packetId );
                            }

                            // 5. Call every callback associated with this topic
                            auto it = subscribers_.begin();
                            for( ; it != subscribers_.end(); ++it )
                            {
                                if( it->topic == topic )
                                {
                                    break;
                                }
                            }
                            if( it == subscribers_.end() )
                            {
                                // If no subscribers are listening to this topic,
                                // notify clients using the "no subscribers for topic" callback.
                                if( noSubscribersForTopic_ )
                                {
                                    noSubscribersForTopic_( topic, data );
                                }
                            }
                            else
                            {
                                for( auto subIter = it->subscribers.begin(); subIter != it->subscribers.end(); ++subIter )
                                {
                                    subIter->callback_( data );
                                }
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                    else if( packetType == PacketType::ACK )
                    {
                        if( ackEvent.packetId == packetId )
                        {
                            ackEvent.state = AckEvent::ACK;
                        }
                        else
                        {
                            return StatusCode::ERROR_UNEXPECTED_ACK;
                        }
                    }
                    else
                    {
                        return StatusCode::ERROR_UNRECOGNISED_PACKET_TYPE;
                    }

                    // If there's more data to process then do so.
                    if( rxData.size() )
                    {
                        result = Utilities::MoveRxDataInBuffer( rxData, rxBuffer_, packet );
                    }
                    else
                    {
                        packet.clear();
                    }
                    if( result == StatusCode::SUCCESS )
                    {
                        // All good.
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    return result;
}

bool EmbeddedSerialFiller::TaskPending() { return ackEvent.packetId != 0; }

static ByteArray packet;
uint8_t EmbeddedSerialFiller::PublishInternal( const PacketType& packetType, uint8_t& packetId, const Topic* topic /* = nullptr*/, const ByteArray* data /* = nullptr*/ )
{
    uint8_t retVal = packetId;
    packet.clear();

    // 1st byte is the packet type, in this case it's PUBLISH
    packet.emplace_back( static_cast<uint8_t>( packetType ) );

    // 2nd byte is the packet identifier
    packet.emplace_back( packetId );
    switch( packetType )
    {
        case PacketType::BROADCAST:
        case PacketType::PUBLISH:
        {
            if( topic != nullptr )
            {
                // 3rd byte (pre-COBS encoded) is num. of bytes for topic
                packet.emplace_back( static_cast<uint8_t>( topic->size() ) );
#if defined( ESF_OPTIMISE )
                // ~5us
                Topic::const_iterator iter = topic->begin();
                Topic::const_iterator endIter = topic->end();
                for( ; iter < endIter; ++iter )
                {
                    packet.emplace_back( *iter );
                }
#else
                // ~14us
                packet.insert( packet.end(), topic->begin(), topic->end() );
#endif
            }
            if( data != nullptr )
            {
#if defined( ESF_OPTIMISE )
                // ~15us
                assert( data->size() <= ESF_MAX_PACKET_SIZE );
                packet.insert( packet.end(), data->begin(), data->end() );
#else
                // ~30us
                ByteArray::const_iterator iter = data->begin();
                ByteArray::const_iterator endIter = data->end();
                for( ; iter < endIter; ++iter )
                {
                    packet.emplace_back( *iter );
                }
#endif
            }
        }
        break;
        case PacketType::ACK:
            break;
        default:
            printf( "!!!  Unrecognised packet type !!!\r\n" );
            break;
    }

    // Add CRC
    {
        // ~18us
        Utilities::AddCrc( packet );
    }

    // Encode data using COBS
    ByteArray encodedData;
    {
        // ~18us
        CobsTranscoder::Encode( packet, encodedData );
    }
    // Emit TX send event
    if( txDataReady_ )
    {
        // ~11us
        txDataReady_( encodedData );
    }
    else
    {
        return retVal;
    }

    if( packetType != PacketType::ACK )
    {
        // If everything was successful, increment packet ID
        ++packetId;
        if( packetId == 0 )
        {
            // ID == 0 is invalid.
            ++packetId;
        }
    }
    return retVal;
}

}  // namespace esf
