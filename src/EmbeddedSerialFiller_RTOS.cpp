/**
 * \file    EmbeddedSerialFiller.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/CobsTranscoder.h"
#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "EmbeddedSerialFiller/Utilities.h"

namespace esf
{
ESF_MUTEX EmbeddedSerialFiller::classMutex_;

EmbeddedSerialFiller::EmbeddedSerialFiller() : nextPacketId_( 1 ), threadSafetyEnabled_( true ), maxAckPacketIndex( 0 ), nextFreeSubsriberId_( 0 )
{
    ESF_CONSTRUCTOR( classMutex_ );
}

uint8_t EmbeddedSerialFiller::Publish( const Topic& topic, const ByteArray& data )
{
    ESF_LOCK lock( classMutex_, ESF_DEFER_LOCK );
    if( threadSafetyEnabled_ )
        lock.lock();

    return PublishInternal( PacketType::BROADCAST, nextPacketId_, &topic, &data );
}

PublishResponse EmbeddedSerialFiller::PublishWait( const Topic& topic, const ByteArray& data, size_t timeout )
{
    ESF_LOCK lock( classMutex_, ESF_DEFER_LOCK );
    if( threadSafetyEnabled_ )
        lock.lock();

    bool gotAck = false;
    // Take a copy since PublishInternal updates the value of nextPacketId_.
    auto packetId = nextPacketId_;
    if( ackEvents_.full() == false )
    {
        // Find first free ackEvent
        uint8_t eventIndex;
        for( eventIndex = 0; eventIndex < ESF_MAX_PENDING_ACKS; ++eventIndex )
        {
            if( events[ eventIndex ].packetId == 0 )
            {
                if( eventIndex > maxAckPacketIndex )
                {
                    maxAckPacketIndex = eventIndex;
                }
                break;
            }
        }
        if( eventIndex < ESF_MAX_PENDING_ACKS )
        {
            // Create cv and bool
            AckEvent& ackEvent = events[ eventIndex ];
            ackEvent.packetId = packetId;
            ackEvents_.push_back( &ackEvent );

            // Call the standard publish
            PublishInternal( PacketType::PUBLISH, nextPacketId_, &topic, &data );
            gotAck = ( ackEvent.cv.wait_for( lock, std::chrono::milliseconds( timeout ) ) == ESF_NO_TIMEOUT );

            for( auto it = ackEvents_.begin(); it != ackEvents_.end(); ++it )
            {
                if( ( *it )->packetId == packetId )
                {
                    // Remove event
                    // Allow this event to be reused.
                    ( *it )->packetId = 0;
                    *it = 0;
                    ackEvents_.erase( it );
                    break;
                }
            }
        }
        else
        {
            printf( "!!!  Invalid index !!!\r\n" );
        }
    }
    else
    {
        printf( "!!!  Run out of AckEvents !!!\r\n" );
    }
    return gotAck ? PublishResponse::SUCCESS : PublishResponse::TIMEOUT;
}

uint32_t EmbeddedSerialFiller::Subscribe( const Topic& topic, etl::delegate<void( ByteArray& )> callback )
{
    ESF_LOCK lock( classMutex_, ESF_DEFER_LOCK );
    if( threadSafetyEnabled_ )
        lock.lock();

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

StatusCode EmbeddedSerialFiller::Unsubscribe( uint32_t subscriberId )
{
    auto retVal = StatusCode::ERROR_UNRECOGNISED_SUBSCRIBER;
    ESF_LOCK lock( classMutex_, ESF_DEFER_LOCK );
    if( threadSafetyEnabled_ )
        lock.lock();

    // Search for subscriber with provided ID
    for( auto it = subscribers_.begin(); it != subscribers_.end(); ++it )
    {
        for( auto subIt = it->subscribers.begin(); subIt != it->subscribers.end(); ++subIt )
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
    ESF_LOCK lock( classMutex_, ESF_DEFER_LOCK );
    if( threadSafetyEnabled_ )
        lock.lock();

    subscribers_.clear();
}

StatusCode EmbeddedSerialFiller::GiveRxData( ByteArray& rxData )
{
    StatusCode result = StatusCode::SUCCESS;
    ESF_LOCK lock( classMutex_, ESF_DEFER_LOCK );

    if( threadSafetyEnabled_ )
    {
        lock.lock();
    }

    ByteArray packet;
    result = Utilities::MoveRxDataInBuffer( rxData, rxBuffer_, packet );
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
            result = CobsTranscoder::Decode( packet, decodedData );
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
                                    if( threadSafetyEnabled_ )
                                    {
                                        lock.unlock();
                                    }
                                    noSubscribersForTopic_( topic, data );
                                    if( threadSafetyEnabled_ )
                                    {
                                        lock.lock();
                                    }
                                }
                            }
                            else
                            {
                                for( auto subIter = it->subscribers.begin(); subIter != it->subscribers.end(); ++subIter )
                                {
                                    if( threadSafetyEnabled_ )
                                    {
                                        lock.unlock();
                                    }
                                    subIter->callback_( data );
                                    if( threadSafetyEnabled_ )
                                    {
                                        lock.lock();
                                    }
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
                        auto it = ackEvents_.begin();
                        for( ; it != ackEvents_.end(); ++it )
                        {
                            if( ( *it )->packetId == packetId )
                            {
                                break;
                            }
                        }
                        if( it == ackEvents_.end() )
                        {
                            return StatusCode::ERROR_UNEXPECTED_ACK;
                        }
                        else
                        {
                            ( *it )->cv.notify_all();
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

uint32_t EmbeddedSerialFiller::NumThreadsWaiting()
{
    ESF_LOCK lock( classMutex_, ESF_DEFER_LOCK );
    if( threadSafetyEnabled_ )
        lock.lock();

    return static_cast<uint32_t>( ackEvents_.size() );
}

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
                Topic::const_iterator iter = topic->begin();
                Topic::const_iterator endIter = topic->end();
                for( ; iter < endIter; ++iter )
                {
                    packet.emplace_back( *iter );
                }
#else
                packet.insert( packet.end(), topic->begin(), topic->end() );
#endif
            }
            if( data != nullptr )
            {
#if defined( ESF_OPTIMISE )
                assert( data->size() <= ESF_MAX_PACKET_SIZE );
                packet.insert( packet.end(), data->begin(), data->end() );
#else
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
    Utilities::AddCrc( packet );

    // Encode data using COBS
    ByteArray encodedData;
    CobsTranscoder::Encode( packet, encodedData );

    // Emit TX send event
    if( txDataReady_ )
    {
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

void EmbeddedSerialFiller::SetThreadSafetyEnabled( bool value )
{
    threadSafetyEnabled_ = value;
}

}  // namespace esf
