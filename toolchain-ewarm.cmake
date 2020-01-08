# BEGIN CMAKE_TOOLCHAIN_FILE

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

# This project started with CMake 2.8.2
# "Generic" is used when cross-compiling
set(CMAKE_SYSTEM_NAME Generic)

#set(CMAKE_SYSTEM_PROCESSOR  STM32 )
#set(CPU_VARIANT STM32F103 CACHE STRINGChoose a member of STM32 Family )

# Compiler flags
set(CPU_FLAGS "--cpu Cortex-M7")

# Set the toolkit root dir
set(EWARM_ROOT_DIR "C:/Program Files (x86)/IAR Systems/Embedded Workbench 8.3/arm")

# Setup the compiler

set(CMAKE_C_COMPILER ${EWARM_ROOT_DIR}/bin/iccarm.exe "${CPU_FLAGS} -e")
set(CMAKE_CXX_COMPILER ${EWARM_ROOT_DIR}/bin/iccarm.exe "${CPU_FLAGS} --c++")
set(CMAKE_ASM_COMPILER ${EWARM_ROOT_DIR}/bin/iasmarm.exe "${CPU_FLAGS} -r")

#set(DLIBS --dlib_config \${COMPILER_PATH}/arm/inc/DLib_Config_Full.h\)
#set(DLIBS --dlib_config \D:/Program Files/IAR Systems/Embedded Workbench 5.5/arm/inc/DLib_Config_Full.h\)
#set(CMAKE_C_FLAGS_INIT  --silent --endian=little --cpu=Cortex-M3 -e --fpu=None --dlib_config \\\D:/Program Files/IAR Systems/Embedded Workbench 5.5/arm/inc/DLib_Config_Full.h\\\ )
#set(CMAKE_CXX_FLAGS_INIT  --silent --endian=little --cpu=Cortex-M3 -e --fpu=None --dlib_config \\\D:/Program Files/IAR Systems/Embedded Workbench 5.5/arm/inc/DLib_Config_Full.h\\\ )


# I guess these are necessary for compiling anything ?
#include_directories(
#${COMPILER_PATH}/arm/inc
#${COMPILER_PATH}/arm/inc/st
#)

# Setup the linker
set(LINKER_SCRIPT "${EWARM_ROOT_DIR}$/config/linker/NXP/MIMXRT1064xxx6A.icf")
set(CMAKE_C_LINK_FLAGS "--semihosting --config ${LINKER_SCRIPT}")
set(CMAKE_CXX_LINK_FLAGS "--semihosting --config ${LINKER_SCRIPT}")

#set(link_file_icf D:/Src/Embedded/KNX/stm32_knx_device/v1.0.0.0/etc/workspace/make/stm32f10x_flash.icf)

# Is this the absolute minimum set of options necessary for linking ?
#set(CMAKE_EXE_LINKER_FLAGS --config ${link_file_icf} --redirect _Scanf=_ScanfSmall --map ${PROJECT_NAME}.map  --entry __iar_program_start )


# search for programs in the build host directories
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM   NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY   ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE   ONLY)
