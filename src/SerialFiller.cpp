///
/// \file 				SerialFiller.cpp
/// \author 			Geoffrey Hunter <gbmhunter@gmail.com> (www.mbedded.ninja)
/// \edited             n/a
/// \created			2017-06-10
/// \last-modified		2018-01-30
/// \brief 				Contains the SerialFiller class.
/// \details
///		See README.md in root dir for more info.

#include <SerialFiller/String.hpp>
#include "SerialFiller/CobsTranscoder.hpp"
#include "SerialFiller/SerialFillerHelper.hpp"
#include "SerialFiller/SerialFiller.hpp"

namespace mn {
namespace SerialFiller {

SerialFiller::SerialFiller()
    : nextPacketId_(0)
    , ackEnabled_(false)
    , threadSafetyEnabled_(true)
    , nextFreeSubsriberId_(0)
{

}

void SerialFiller::Publish(const Topic& topic, const ByteArray& data)
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    PublishInternal(topic, data);
}

bool SerialFiller::PublishWait(const Topic&              topic,
                               const ByteArray&          data,
                               std::chrono::milliseconds timeout)
{
    LOG((*logger_), DEBUG, "PublishWait called. nextPacketId_=" + std::to_string(nextPacketId_));

    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    if (!ackEnabled_) {
        LOG((*logger_), WARNING, "PublishWait() called but auto-ACK was not enabled.");
    }

    auto packetId        = nextPacketId_;
    if (ackEvents_.available()) {
        // Create cv and bool
        ackEvents_[packetId] = std::make_shared<EventType>();
    }
    else {
        LOG((*logger_),
            ERROR,
            "PublishWait() ackEvents is full. Increase the size of MAX_PENDING_ACKS.");
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
            }

            return it->second->second;
        });

        // Remove event from map
        ackEvents_.erase(packetId);
    }
    else {
        gotAck = true;
    }
    LOG((*logger_), DEBUG, "Method returning...");
    return gotAck;
}

uint32_t SerialFiller::Subscribe(const Topic& topic, etl::delegate<void(ByteArray&)> callback)
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

void SerialFiller::Unsubscribe(uint32_t subscriberId)
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    // Search for subscriber with provided ID
    for (auto it = subscribers_.begin(); it != subscribers_.end(); ++it) {
        if ((*it).second.id_ == subscriberId) {
            subscribers_.erase(it);
            return;
        }
    }

    // If we reach here, no subscriber was found!
    LOG((*logger_),
        ERROR,
        std::string() + __FUNCTION__ + " called but subscriber ID of "
            + std::to_string(subscriberId) + " was not found.");
}

void SerialFiller::UnsubscribeAll()
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    subscribers_.clear();
}

void SerialFiller::GiveRxData(ByteArray& rxData)
{
    LOG((*logger_),
        DEBUG,
        std::string() + "Method called with rxData = " + mn::CppUtils::String::ToHex(rxData));
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);

    if (threadSafetyEnabled_) {
        LOG((*logger_), DEBUG, "Locking mutex...");
        lock.lock();
        LOG((*logger_), DEBUG, "Mutex locked.");
    }

    LOG((*logger_),
        DEBUG,
        "Before extracting packets, rxBuffer_ = " + CppUtils::String::ToHex(rxBuffer_));
    LOG((*logger_), DEBUG, "Before extracting packets, rxData = " + CppUtils::String::ToHex(rxData));

    ByteArray packet;
    SerialFillerHelper::MoveRxDataInBuffer(rxData, rxBuffer_, packet);
    while (!packet.empty()) {
        LOG((*logger_),
            DEBUG,
            "Found packet. Data (COBS encoded) = " + CppUtils::String::ToHex(packet));
        LOG((*logger_), DEBUG, "rxBuffer_ now = " + CppUtils::String::ToHex(rxBuffer_));
        LOG((*logger_), DEBUG, "rxData now = " + CppUtils::String::ToHex(rxData));
        Topic     topic;
        ByteArray data;

        //==============================//
        //======= FOR EACH PACKET ======//
        //==============================//

        // Remove COBS encoding
        //    This method will throw if the packet does not have a valid COBS
        //    encoding.
        ByteArray decodedData;
        CobsTranscoder::Decode(packet, decodedData);

        // Verify CRC
        if (SerialFillerHelper::VerifyCrc(decodedData)) {
            // Look at packet type
            auto packetType = static_cast<PacketType>(decodedData[0]);
            // Extract packet ID
            uint8_t packetId = static_cast<uint8_t>(decodedData.at(1));
            if (packetType == PacketType::PUBLISH) {
                LOG((*logger_),
                    DEBUG,
                    "Received PUBLISH packet. [1]=" + std::to_string(decodedData.at(1)));
                // 4. Then split packet into topic and data
                //    This will throw if topic length is invalid
                SerialFillerHelper::SplitPacket(decodedData, 2, topic, data);

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

                for (Subscribers::iterator rangeIt = range.first; rangeIt != range.second;
                     ++rangeIt) {
                    if (threadSafetyEnabled_)
                        lock.unlock();
                    //                        std::cout << "Calling listener..." << std::endl;
                    rangeIt->second.callback_(data);
                    //                        std::cout << "Listener finished, relocking..." << std::endl;
                    if (threadSafetyEnabled_)
                        lock.lock();
                    //                        std::cout << "Relocked." << std::endl;
                }
            }
            else if (packetType == PacketType::ACK) {
                LOG((*logger_), DEBUG, "Received ACK packet.");
                if (!ackEnabled_) {
                    LOG((*logger_),
                        ERROR,
                        "SerialFiller node received ACK packet but auto-ACK was not enabled.");
                    return;
                }

                //                    std::cout << "Looking for packetId = " << packetId << std::endl;
                auto it = ackEvents_.find(packetId);
                if (it == ackEvents_.end()) {
                    //                        std::cout << "No threads waiting on ACK." << std::endl;
                }
                else {
                    it->second->second = true;
                    it->second->first.notify_all();
                }
            }
            else {
                LOG((*logger_), ERROR, "Received packet type not recognized.");
                break;
            }

            SerialFillerHelper::MoveRxDataInBuffer(rxData, rxBuffer_, packet);
        }
        else {
            break;
        }
    }
    LOG((*logger_), DEBUG, std::string() + "Method finished.");
}

void SerialFiller::SetAckEnabled(bool value)
{
    ackEnabled_ = value;
}

void SerialFiller::SendAck(uint8_t packetId)
{
    LOG((*logger_), DEBUG, "SendAck called with packetId = " + std::to_string(packetId));

    ByteArray packet;

    // 1st byte is the packet type, in this case it's ACK
    packet.push_back(static_cast<uint8_t>(PacketType::ACK));

    // 2nd byte is the packet identifier
    packet.push_back(static_cast<char>(packetId));

    // Add CRC
    SerialFillerHelper::AddCrc(packet);

    // Encode data using COBS
    ByteArray encodedData;
    CobsTranscoder::Encode(packet, encodedData);

    ByteArray txData(encodedData.begin(), encodedData.end());

    // Emit TX send event
    if (txDataReady_) {
        txDataReady_(txData);
    }
    else {
        LOG((*logger_),
            ERROR,
            std::string() + __FUNCTION__
                + " was called but txDataReady_ function has no valid function object.");
    }

    //            std::cout << "SendAck() finished." << std::endl;
}

uint32_t SerialFiller::NumThreadsWaiting()
{
    std::unique_lock<std::mutex> lock(classMutex_, std::defer_lock);
    if (threadSafetyEnabled_)
        lock.lock();

    return static_cast<uint32_t>(ackEvents_.size());
}

void SerialFiller::PublishInternal(const Topic& topic, const ByteArray& data)
{
    ByteArray packet;

    // 1st byte is the packet type, in this case it's PUBLISH
    packet.push_back(static_cast<uint8_t>(PacketType::PUBLISH));

    // 2nd byte is the packet identifier
    packet.push_back(static_cast<char>(nextPacketId_));

    // 3rd byte (pre-COBS encoded) is num. of bytes for topic
    packet.push_back(static_cast<char>(topic.size()));

    std::copy(topic.begin(), topic.end(), std::back_inserter(packet));

    std::copy(data.begin(), data.end(), std::back_inserter(packet));

    // Add CRC
    SerialFillerHelper::AddCrc(packet);

    // Encode data using COBS
    ByteArray encodedData;
    CobsTranscoder::Encode(packet, encodedData);

    ByteArray txData(encodedData.begin(), encodedData.end());

    // Emit TX send event
    if (txDataReady_) {
        txDataReady_(txData);
    }
    else {
        LOG((*logger_),
            ERROR,
            std::string() + __FUNCTION__
                + " was called but txDataReady_ function has no valid function object.");
    }

    // If everything was successful, increment packet ID
    nextPacketId_ += 1;
}

void SerialFiller::SetThreadSafetyEnabled(bool value)
{
    threadSafetyEnabled_ = value;
}

} // namespace SerialFiller
} // namespace mn
