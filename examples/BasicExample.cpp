/**
 * \file    BasicExample.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "SerialFiller/SerialFiller.hpp"
#include <vector>

using namespace esf;

int main() {
#if 0
    EmbeddedSerialFiller serialFiller;

    // Connect the I/O together, to make
    // a software "loop-back"
    serialFiller.txDataReady_ = ([&](ByteQueue txData) -> void {
        serialFiller.GiveRxData(txData);
    });

    // Subscribe to topic "mytopic"
    serialFiller.Subscribe("mytopic", [](std::vector<uint8_t> rxData) -> void {
        std::cout << "Received packet on mytopic!" << std::endl;

        std::cout << " Data = ";
        for(auto dataByte : rxData) {
            std::cout << std::to_string(dataByte);
        }
        std::cout << std::endl;
    });

    serialFiller.Publish("mytopic", {0x01, 0x02, 0x03});
#endif
    return 0;
}