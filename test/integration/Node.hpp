/**
 * \file    Node.hpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#ifndef ESF_SERIAL_FILLER_NODE_H_
#define ESF_SERIAL_FILLER_NODE_H_

#include <atomic>
#include <string>
#include <iostream>
#include <functional>
#include <thread>
#include "SerialFiller/SerialFiller.hpp"
#include "ThreadSafeQ.hpp"

namespace esf {

        class Node {

        public:

            std::shared_ptr<Logger> logger_;
            EmbeddedSerialFiller serialFiller_;
            CppUtils::ThreadSafeQ<uint8_t> rxQueue_;

            Node(std::string name) :
                    logger_(new Logger("EmbeddedSerialFiller", Logger::Severity::NONE, Logger::Color::CYAN, [](std::string msg) { std::cout << msg << std::endl; })),
                    name_(name),
                    breakThread_(false) {
                rxThread_ = std::thread(&Node::RxThreadFn, this);

                serialFiller_.SetAckEnabled(true);
            }

            ~Node() {
                Join();
            }

            void Join() {
                if(rxThread_.joinable()) {
                    breakThread_.store(true);
                    rxThread_.join();
                }
            }

            void RxThreadFn() {
//                std::cout << __FUNCTION__ << "() called for " << name_ << std::endl;

                while (true) {
                    // Wait for data to arrive on the queue
//                    std::cout << "RX thread for " << name_ << " still running..." << std::endl;
                    uint8_t data;
                    if (rxQueue_.TryPop(data, std::chrono::milliseconds(1000))) {
//                        std::cout << name_ << " received data." << std::endl;
                        ByteQueue dataAsQ;
                        dataAsQ.push_back(data);
                        serialFiller_.GiveRxData(dataAsQ);
                    }

                    if(breakThread_.load())
                        break;
                }

            }

        private:
            std::string name_;
            std::thread rxThread_;
            std::atomic<bool> breakThread_;
        };
} // namespace esf

#endif // #ifndef ESF_SERIAL_FILLER_NODE_H_