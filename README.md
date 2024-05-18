

# [EmbeddedSerialFiller](https://github.com/jupeos/EmbeddedSerialFiller#embeddedserialfiller)

A serial publish/subscribe based communication protocol for Embedded MCU's with support for bare metal or RTOS. May be used for sending messages across serial links (although it is agnostic to the transport layer) with the ability to transfer any type of data on a "topic".

Combining the [ETL](https://github.com/ETLCPP/etl) with [SerialFiller](https://github.com/gbmhunter/SerialFiller) intended for small footprint embedded systems.

The main differences to SerialFiller are:

- No use of STL containers (all replaced with ETL)
- As a result no dynamic heap allocation
- Exact footprint determined by user definitions in `Definitions.h`
- Protocol change to use a single byte packet identifier vs 2 bytes in SF
- No exceptions (replaced by `StatusCode` enumeration)
- Removal of Logger (temporarily)
- Added multi-packet test

# Setup

For fully compliant C++11 compilers/OS's nothing further is needed. You may wish to review some of the switches in *Definitions.h*.

When used with a RTOS a minimal implementation is needed to fulfil the abstraction of a mutex, conditional_variable etc.. Refer to the file `esf_abstraction.h` and `esf_freertos_abstraction.h/c` for an implementation for [FreeRTOS](https://www.freertos.org/). A similar implementation exists for Segger's embOS RTOS in `esf_embos_abstraction.h/c`. In this case an additional definition is needed to select the correct implementation, e.g. `PROFILE_FREERTOS` or `PROFILE_EMBOS`.

[ESF](https://github.com/jupeos/EmbeddedSerialFiller) provides an implementation for no RTOS targets. The expectation is some form of time driven architecture where periodic calls to `PublishWait` drive the 'wait' timeout. Refer to the header file `EmbeddedSerialFiller_NoRTOS.h` for more information.

Building/Installing
===================

Clone this repository (`git clone https://github.com/jupeos/EmbeddedSerialFiller.git` as well as the etl submodule `git submodule update --init`). Then `cd` into the repo's root directory and do one of the following:

Use The Script
--------------

    NOT IMPLEMENTED

Manual
------

### Linux

    ~/EmbeddedSerialFiller$ mkdir build
    ~/EmbeddedSerialFiller$ cd build
    ~/EmbeddedSerialFiller/build$ cmake ..
    ~/EmbeddedSerialFiller/build$ make
To include the tests use `cmake -DBUILD_TESTS=ON ..`

### Windows

```
~/EmbeddedSerialFiller$ mkdir build
~/EmbeddedSerialFiller$ cd build
~/EmbeddedSerialFiller/build$ cmake .. -G "NMake Makefiles"
~/EmbeddedSerialFiller/build$ nmake
```

To include the tests use `cmake -DBUILD_TESTS=ON .. -G "NMake Makefiles"`

---

### Testing

Run the unit tests from `~/EmbeddedSerialFiller/build/test$` with `./EmbeddedSerialFillerTests`

No tests exist for the NoRTOS profile as yet.

Tests passing on Linux (gcc 9.3.0) & Windows (nmake 14.29.30038.1).
