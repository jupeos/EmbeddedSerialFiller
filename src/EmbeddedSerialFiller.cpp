/**
 * \file    EmbeddedSerialFiller.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "EmbeddedSerialFiller/CobsTranscoder.h"
#include "EmbeddedSerialFiller/Utilities.h"

namespace esf {

EmbeddedSerialFiller::EmbeddedSerialFiller()
    : nextPacketId_(0)
    , threadSafetyEnabled_(true)
    , nextFreeSubsriberId_(0)
{}

void EmbeddedSerialFiller::Publish(const Topic& topic, const ByteArray& data)
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    PublishInternal(PacketType::BROADCAST, nextPacketId_, &topic, &data);
}

bool EmbeddedSerialFiller::PublishWait(const Topic& topic, const ByteArray& data, std::chrono::milliseconds timeout)
{
    LOG((*logger_), DEBUG, "PublishWait called. nextPacketId_=" + std::to_string(nextPacketId_));

    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    // Take a copy since PublishInternal updates the value of nextPacketId_.
    auto packetId = nextPacketId_;
    auto ackEvent = AckEvents({packetId, std::make_shared<EventType>()});
    if (ackEvents_.full() == false) {
        // Create cv and bool
        ackEvents_.push_back(ackEvent);
    } else {
        LOG((*logger_), ERROR, "PublishWait() ackEvents is full. Increase the size of MAX_PENDING_ACKS.");
        return false;
    }

    // Call the standard publish
    PublishInternal(PacketType::PUBLISH, nextPacketId_, &topic, &data);

    bool gotAck = false;
    if (ackEvent.eventType->first.wait_for(lock, timeout) == std::cv_status::no_timeout) {
        for (auto it = ackEvents_.begin(); it != ackEvents_.end(); ++it) {
            if (it->ID == packetId) {
                gotAck = it->eventType->second;
                // Remove event from map
                ackEvents_.erase(it);
                break;
            }
        }
    }

    LOG((*logger_), DEBUG, "Method returning...");
    return gotAck;
}

uint32_t EmbeddedSerialFiller::Subscribe(const Topic& topic, etl::delegate<void(ByteArray&)> callback)
{
    LOG((*logger_), DEBUG, std::string() + "Method called.");
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    // Assign ID and update free IDs
    auto id = nextFreeSubsriberId_;
    nextFreeSubsriberId_++;

    // Save subscription
    Subscriber subscriber;
    subscriber.id_       = id;
    subscriber.callback_ = callback;
    for (auto it = subscribers_.begin(); it != subscribers_.end(); ++it) {
        if (it->topic == topic) {
            it->subscribers.push_back(subscriber);
            return id;
        }
    }
    SubscriberType st;
    st.topic = topic;
    st.subscribers.push_back(subscriber);
    subscribers_.push_back(st);
    return id;
}

StatusCode EmbeddedSerialFiller::Unsubscribe(uint32_t subscriberId)
{
    auto                         retVal = StatusCode::ERROR_UNRECOGNISED_SUBSCRIBER;
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    // Search for subscriber with provided ID
    for (auto it = subscribers_.begin(); it != subscribers_.end(); ++it) {
        for (auto subIt = it->subscribers.begin(); subIt != it->subscribers.end(); ++subIt) {
            if (subIt->id_ == subscriberId) {
                it->subscribers.erase(subIt);
                retVal = StatusCode::SUCCESS;
                break;
            }
        }
    }

    // If we reach here, no subscriber was found!
    LOG((*logger_), ERROR, std::string() + __FUNCTION__ + " called but subscriber ID of " + std::to_string(subscriberId) + " was not found.");
    return retVal;
}

void EmbeddedSerialFiller::UnsubscribeAll()
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    subscribers_.clear();
}

StatusCode EmbeddedSerialFiller::GiveRxData(ByteArray& rxData)
{
    StatusCode result = StatusCode::SUCCESS LOG((*logger_), DEBUG, std::string() + "Method called with rxData = " + mn::CppUtils::String::ToHex(rxData));
    std::unique_lock<std::mutex>            lock(classMutex_, std::defer_lock);

    if (threadSafetyEnabled_) {
        LOG((*logger_), DEBUG, "Locking mutex...");
        lock.lock();
        LOG((*logger_), DEBUG, "Mutex locked.");
    }

    LOG((*logger_), DEBUG, "Before extracting packets, rxBuffer_ = " + CppUtils::String::ToHex(rxBuffer_));
    LOG((*logger_), DEBUG, "Before extracting packets, rxData = " + CppUtils::String::ToHex(rxData));

    ByteArray packet;
    Utilities::MoveRxDataInBuffer(rxData, rxBuffer_, packet);
    while (!packet.empty()) {
        LOG((*logger_), DEBUG, "Found packet. Data (COBS encoded) = " + CppUtils::String::ToHex(packet));
        LOG((*logger_), DEBUG, "rxBuffer_ now = " + CppUtils::String::ToHex(rxBuffer_));
        LOG((*logger_), DEBUG, "rxData now = " + CppUtils::String::ToHex(rxData));
        Topic topic;

        //==============================//
        //======= FOR EACH PACKET ======//
        //==============================//

        // Remove COBS encoding
        ByteArray decodedData;
        result = CobsTranscoder::Decode(packet, decodedData);
        if (result == StatusCode::SUCCESS) {
            // Verify CRC
            result = Utilities::VerifyCrc(decodedData);
            if (result == StatusCode::SUCCESS) {
                // Look at packet type
                auto packetType = static_cast<PacketType>(decodedData[0]);
                // Extract packet ID
                uint8_t packetId = static_cast<uint8_t>(decodedData.at(1));
                if ((packetType == PacketType::BROADCAST) || (packetType == PacketType::PUBLISH)) {
                    LOG((*logger_), DEBUG, "Received PUBLISH packet. [1]=" + std::to_string(decodedData.at(1)));
                    // 4. Then split packet into topic and data (let's just reuse the packet container for this);
                    ByteArray& data = packet;
                    result          = Utilities::SplitPacket(decodedData, 2, topic, data);
                    if (result == StatusCode::SUCCESS) {
                        // WARNING: Make sure to send ack BEFORE invoking topic callbacks, as they may cause other messages
                        // to be sent, and we always want the ACK to be the first thing sent back to the sender.
                        if (packetType == PacketType::PUBLISH)
                            PublishInternal(PacketType::ACK, packetId);

                        // 5. Call every callback associated with this topic
                        //RangeType range = subscribers_.equal_range(topic);
                        auto it = subscribers_.begin();
                        for (; it != subscribers_.end(); ++it) {
                            if (it->topic == topic) {
                                break;
                            }
                        }
                        if (it == subscribers_.end()) {
                            // If no subscribers are listening to this topic,
                            // fire the "no subscribers for topic" event (user can
                            // listen to this)
                            if (threadSafetyEnabled_)
                                lock.unlock();
                            if (noSubscribersForTopic_) {
                                noSubscribersForTopic_(topic, data);
                            }
                            if (threadSafetyEnabled_)
                                lock.lock();
                        } else {
                            for (auto subIter = it->subscribers.begin(); subIter != it->subscribers.end(); ++subIter) {
                                if (threadSafetyEnabled_)
                                    lock.unlock();
                                //                        std::cout << "Calling listener..." << std::endl;
                                subIter->callback_(data);
                                //                        std::cout << "Listener finished, relocking..." << std::endl;
                                if (threadSafetyEnabled_)
                                    lock.lock();
                                //                        std::cout << "Relocked." << std::endl;
                            }
                        }
                    } else {
                        break;
                    }
                } else if (packetType == PacketType::ACK) {
                    LOG((*logger_), DEBUG, "Received ACK packet.");
                    //                    std::cout << "Looking for packetId = " << packetId << std::endl;
                    auto it = ackEvents_.begin();
                    for (; it != ackEvents_.end(); ++it) {
                        if (it->ID == packetId) {
                            break;
                        }
                    }
                    if (it == ackEvents_.end()) {
                        //                        std::cout << "No threads waiting on ACK." << std::endl;
                        return StatusCode::ERROR_UNEXPECTED_ACK;
                    } else {
                        it->eventType->second = true;
                        it->eventType->first.notify_all();
                    }
                } else {
                    LOG((*logger_), ERROR, "Received packet type not recognized.");
                    return StatusCode::ERROR_UNRECOGNISED_PACKET_TYPE;
                }

                Utilities::MoveRxDataInBuffer(rxData, rxBuffer_, packet);
            } else {
                break;
            }
        } else {
            break;
        }
    }
    LOG((*logger_), DEBUG, std::string() + "Method finished.");
    return result;
}

uint32_t EmbeddedSerialFiller::NumThreadsWaiting()
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    return static_cast<uint32_t>(ackEvents_.size());
}

void EmbeddedSerialFiller::PublishInternal(const PacketType& packetType, uint8_t& packetId, const Topic* topic /* = nullptr*/, const ByteArray* data /* = nullptr*/)
{
    ByteArray packet;

    // 1st byte is the packet type, in this case it's PUBLISH
    packet.push_back(static_cast<uint8_t>(packetType));

    // 2nd byte is the packet identifier
    packet.push_back(packetId);

    switch (packetType) {
    case PacketType::BROADCAST:
    case PacketType::PUBLISH: {
        if (topic != nullptr) {
            // 3rd byte (pre-COBS encoded) is num. of bytes for topic
            packet.push_back(static_cast<uint8_t>(topic->size()));

            for (auto it = topic->begin(); it != topic->end(); ++it) {
                packet.push_back(*it);
            }
        }
        if (data != nullptr) {
            for (auto it = data->begin(); it != data->end(); ++it) {
                packet.push_back(*it);
            }
        }
    } break;
    case PacketType::ACK:
    default:
        break;
    }

    // Add CRC
    Utilities::AddCrc(packet);

    // Encode data using COBS
    ByteArray encodedData;
    CobsTranscoder::Encode(packet, encodedData);

    // Emit TX send event
    if (txDataReady_) {
        txDataReady_(encodedData);
    } else {
        LOG((*logger_), ERROR, std::string() + __FUNCTION__ + " was called but txDataReady_ function has no valid function object.");
        return;
    }

    if (packetType != PacketType::ACK) {
        // If everything was successful, increment packet ID
        ++packetId;
    }
}

void EmbeddedSerialFiller::SetThreadSafetyEnabled(bool value)
{
    threadSafetyEnabled_ = value;
}

} // namespace esf
