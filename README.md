

# EmbeddedSerialFiller

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

For fully compliant C++11 compilers/OS's nothing further is needed. When used with a RTOS a minimal implementation is needed to fulfil the abstraction of a mutex, conditional_variable etc.. Refer to the file

`esf_abstraction.h` and `esf_embos_abstraction.h/c` for an implementation for Segger's embOS RTOS. In this case an additional definition is needed to select the correct implementation, e.g. `PROFILE_EMBOS`.

Building/Installing
===================

Clone this repository (`git clone https://github.com/jupeos/EmbeddedSerialFiller.git`). Then `cd` into the repo's root directory and do one of the following:

Use The Script
--------------

    NOT IMPLEMENTED

Manual
------

    ~/EmbeddedSerialFiller$ mkdir build
    ~/EmbeddedSerialFiller$ cd build
    ~/EmbeddedSerialFiller/build$ cmake ..
    ~/EmbeddedSerialFiller/build$ make
Run the unit tests from `~/EmbeddedSerialFiller/build/test$` with `./EmbeddedSerialFillerTests`

Tests passing on Linux & Windows.
