/**
 * \file    BasicExample.cpp
 * \author  Julian Mitchell
 * \date    11 Sep 2019
 */

#include "EmbeddedSerialFiller/EmbeddedSerialFiller.h"
#include <vector>

using namespace esf;

int main() {
#if 0
    EmbeddedSerialFiller embeddedSF;

    // Connect the I/O together, to make
    // a software "loop-back"
    embeddedSF.txDataReady_ = ([&](ByteQueue txData) -> void {
        embeddedSF.GiveRxData(txData);
    });

    // Subscribe to topic "mytopic"
    embeddedSF.Subscribe("mytopic", [](std::vector<uint8_t> rxData) -> void {
        std::cout << "Received packet on mytopic!" << std::endl;

        std::cout << " Data = ";
        for(auto dataByte : rxData) {
            std::cout << std::to_string(dataByte);
        }
        std::cout << std::endl;
    });

    embeddedSF.Publish("mytopic", {0x01, 0x02, 0x03});
#endif
    return 0;
}