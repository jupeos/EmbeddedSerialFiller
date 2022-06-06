/**
 * \file    Node.h
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#ifndef ESF_NODE_H
#define ESF_NODE_H

#include <atomic>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include "ThreadSafeQ.h"

namespace esf
{
class Node
{
   public:
    EmbeddedSerialFiller embeddedSF;
    CppUtils::ThreadSafeQ<uint8_t> rxQueue_;

    Node( std::string name )
        : name_( name ), breakThread_( false )
    {
        rxThread_ = std::thread( &Node::RxThreadFn, this );
    }

    ~Node() { Join(); }

    void Join()
    {
        if( rxThread_.joinable() )
        {
            breakThread_.store( true );
            rxThread_.join();
        }
    }

    void RxThreadFn()
    {
        //std::cout << __FUNCTION__ << "() called for " << name_ << std::endl;

        while( true )
        {
            // Wait for data to arrive on the queue
            //std::cout << "RX thread for " << name_ << " still running..." << std::endl;
            uint8_t data;
            if( rxQueue_.TryPop( data, std::chrono::milliseconds( 1000 ) ) )
            {
                //std::cout << name_ << " received data." << std::endl;
                ByteQueue dataAsQ;
                dataAsQ.push_back( data );
                embeddedSF.GiveRxData( dataAsQ );
            }

            if( breakThread_.load() )
                break;
        }
    }

   private:
    std::string name_;
    std::thread rxThread_;
    std::atomic<bool> breakThread_;
};
}  // namespace esf

#endif  // #ifndef ESF_NODE_H
