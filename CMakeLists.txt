cmake_minimum_required(VERSION 3.1.0)
project(EmbeddedSerialFiller)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DEPENDENCIES "If set to ON, dependencies will be downloaded and built as part of the build process." OFF)
option(DEFINE_PROFILE_WINDOWS "Option description" OFF) # Enabled by default

if (DEFINE_PROFILE_WINDOWS)
    add_definitions(-DPROFILE_WINDOWS)
endif (DEFINE_PROFILE_WINDOWS)

if (BUILD_DEPENDENCIES)
    message("BUILD_DEPENDENCIES=ON, dependencies will be downloaded and built automatically.")
    # EXTERNAL_INSTALL_LOCATION is used by external projects, except for gtest
    include(ExternalProject)
    set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
else ()
    message("BUILD_DEPENDENCIES=OFF, dependencies have to be downloaded, built and installed manually.")
endif ()


if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	set(ETL_PROFILE "PROFILE_LINUX")
	add_definitions(-DPROFILE_GCC_LINUX_X86)
	message("ETL_PROFILE detected to be PROFILE_GCC_LINUX_X86")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(ETL_PROFILE "PROFILE_CLANG_GENERIC")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	# set(ETL_PROFILE "PROFILE_MSVC")
	add_definitions(-DPROFILE_WINDOWS)
	add_definitions(-DPROFILE_MSVC)
	message("ETL_PROFILE detected to be PROFILE_MSVC")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "ARMCC" OR CMAKE_CXX_COMPILER_ID MATCHES "IAR")
	if (CXX_STANDARD EQUAL 11)
          set(ETL_PROFILE "PROFILE_ARMV6")
          add_definitions(-DPROFILE_ARMV6)
          message("ETL_PROFILE detected to be PROFILE_ARMV6")
	else ()
          set(ETL_PROFILE "PROFILE_ARMV5")
          add_definitions(-DPROFILE_ARMV6)
          message("ETL_PROFILE set to PROFILE_ARMV6")
	endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "TI")
	set(ETL_PROFILE "PROFILE_TICC")
else()
	message(FATAL_ERROR "Can't generate default profile for compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

option(BUILD_TESTS "If set to true, unit tests will be build as part of make all." OFF)
if (BUILD_TESTS)
    message("BUILD_TESTS=TRUE, unit tests will be built.")
    set(BUILD_DEPENDENCIES ON)
else ()
    message("BUILD_TESTS=FALSE, unit tests will NOT be built.")
endif ()

#=================================================================================================#
#============================================== gtest ============================================#
#=================================================================================================#

if (BUILD_DEPENDENCIES)
    # Download and unpack googletest at configure time
    include(FetchContent)
    FetchContent_Declare(
      googletest
      # Specify the commit you depend on and update it regularly.
      URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

#=================================================================================================#
#========================================= This Project ==========================================#
#=================================================================================================#

# Now simply link your own targets against gtest, gmock,
# etc. as appropriate
include_directories(include)
include_directories(etl/include)

add_subdirectory(src)
if(BUILD_TESTS)
    add_subdirectory(test)
endif()
