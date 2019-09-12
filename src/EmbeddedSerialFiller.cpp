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
    , ackEnabled_(false)
    , threadSafetyEnabled_(true)
    , nextFreeSubsriberId_(0)
{}

void EmbeddedSerialFiller::Publish(const Topic& topic, const ByteArray& data)
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    PublishInternal(topic, data);
}

bool EmbeddedSerialFiller::PublishWait(const Topic& topic, const ByteArray& data, std::chrono::milliseconds timeout)
{
    LOG((*logger_), DEBUG, "PublishWait called. nextPacketId_=" + std::to_string(nextPacketId_));

    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    if (!ackEnabled_) {
        LOG((*logger_), WARNING, "PublishWait() called but auto-ACK was not enabled.");
    }

    auto packetId = nextPacketId_;
    if (ackEvents_.available()) {
        // Create cv and bool
        ackEvents_[packetId] = std::make_shared<EventType>();
    } else {
        LOG((*logger_), ERROR, "PublishWait() ackEvents is full. Increase the size of MAX_PENDING_ACKS.");
        return false;
    }

    // Call the standard publish
    PublishInternal(topic, data);

    bool gotAck;
    if (ackEnabled_) {
        gotAck = ackEvents_[packetId]->first.wait_for(lock, timeout, [this, packetId]() {
            auto it = ackEvents_.find(packetId);
            if (it == ackEvents_.end()) {
                LOG((*logger_), ERROR, "Could not find entry in map.");
                return false;
            }

            return it->second->second;
        });

        // Remove event from map
        ackEvents_.erase(packetId);
    } else {
        gotAck = true;
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
    subscribers_.insert({topic, subscriber});

    return id;
}

StatusCode EmbeddedSerialFiller::Unsubscribe(uint32_t subscriberId)
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    // Search for subscriber with provided ID
    for (auto it = subscribers_.begin(); it != subscribers_.end(); ++it) {
        if ((*it).second.id_ == subscriberId) {
            subscribers_.erase(it);
            return StatusCode::SUCCESS;
        }
    }

    // If we reach here, no subscriber was found!
    LOG((*logger_), ERROR, std::string() + __FUNCTION__ + " called but subscriber ID of " + std::to_string(subscriberId) + " was not found.");
    return StatusCode::ERROR_UNRECOGNISED_SUBSCRIBER;
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
        Topic     topic;
        ByteArray data;

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
                if (packetType == PacketType::PUBLISH) {
                    LOG((*logger_), DEBUG, "Received PUBLISH packet. [1]=" + std::to_string(decodedData.at(1)));
                    // 4. Then split packet into topic and data
                    result = Utilities::SplitPacket(decodedData, 2, topic, data);
                    if (result == StatusCode::SUCCESS) {
                        // WARNING: Make sure to send ack BEFORE invoking topic callbacks, as they may cause other messages
                        // to be sent, and we always want the ACK to be the first thing sent back to the sender.
                        if (ackEnabled_)
                            SendAck(packetId);

                        // 5. Call every callback associated with this topic
                        RangeType range = subscribers_.equal_range(topic);

                        // If no subscribers are listening to this topic,
                        // fire the "no subscribers for topic" event (user can
                        // listen to this)
                        if (range.first == range.second) {
                            if (threadSafetyEnabled_)
                                lock.unlock();
                            if (noSubscribersForTopic_) {
                                noSubscribersForTopic_(topic, data);
                            }
                            if (threadSafetyEnabled_)
                                lock.lock();
                        }

                        for (auto rangeIt = range.first; rangeIt != range.second; ++rangeIt) {
                            if (threadSafetyEnabled_)
                                lock.unlock();
                            //                        std::cout << "Calling listener..." << std::endl;
                            rangeIt->second.callback_(data);
                            //                        std::cout << "Listener finished, relocking..." << std::endl;
                            if (threadSafetyEnabled_)
                                lock.lock();
                            //                        std::cout << "Relocked." << std::endl;
                        }
                    } else {
                        break;
                    }
                } else if (packetType == PacketType::ACK) {
                    LOG((*logger_), DEBUG, "Received ACK packet.");
                    if (!ackEnabled_) {
                        LOG((*logger_), ERROR, "EmbeddedSerialFiller node received ACK packet but auto-ACK was not enabled.");
                        return StatusCode::ERROR_AUTO_ACK_DISABLED;
                    }

                    //                    std::cout << "Looking for packetId = " << packetId << std::endl;
                    auto it = ackEvents_.find(packetId);
                    if (it == ackEvents_.end()) {
                        //                        std::cout << "No threads waiting on ACK." << std::endl;
                    } else {
                        it->second->second = true;
                        it->second->first.notify_all();
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

void EmbeddedSerialFiller::SetAckEnabled(bool value)
{
    ackEnabled_ = value;
}

void EmbeddedSerialFiller::SendAck(uint8_t packetId)
{
    LOG((*logger_), DEBUG, "SendAck called with packetId = " + std::to_string(packetId));

    ByteArray packet;

    // 1st byte is the packet type, in this case it's ACK
    packet.push_back(static_cast<uint8_t>(PacketType::ACK));

    // 2nd byte is the packet identifier
    packet.push_back(packetId);

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
    }

    //            std::cout << "SendAck() finished." << std::endl;
}

uint32_t EmbeddedSerialFiller::NumThreadsWaiting()
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    return static_cast<uint32_t>(ackEvents_.size());
}

void EmbeddedSerialFiller::PublishInternal(const Topic& topic, const ByteArray& data)
{
    ByteArray packet;

    // 1st byte is the packet type, in this case it's PUBLISH
    packet.push_back(static_cast<uint8_t>(PacketType::PUBLISH));

    // 2nd byte is the packet identifier
    packet.push_back(nextPacketId_);

    // 3rd byte (pre-COBS encoded) is num. of bytes for topic
    packet.push_back(static_cast<uint8_t>(topic.size()));

    for(auto it = topic.begin(); it != topic.end(); ++it)
    {
        packet.push_back(*it);
    }

    for(auto it = data.begin(); it != data.end(); ++it)
    {
        packet.push_back(*it);
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

    // If everything was successful, increment packet ID
    nextPacketId_ += 1;
}

void EmbeddedSerialFiller::SetThreadSafetyEnabled(bool value)
{
    threadSafetyEnabled_ = value;
}

} // namespace esf
