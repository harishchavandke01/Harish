cmake_minimum_required(VERSION 3.16)

project(rtklib C)

# Compile as C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
set(RTKLIB_SOURCES
    rcv/binex.c
    rcv/crescent.c
    rcv/javad.c
    rcv/novatel.c
    rcv/nvs.c
    rcv/rt17.c
    rcv/septentrio.c
    rcv/skytraq.c
    rcv/swiftnav.c
    rcv/ublox.c
    rcv/unicore.c
    convgpx.c convkml.c convrnx.c datum.c download.c ephemeris.c
    geoid.c gis.c ionex.c lambda.c options.c pntpos.c postpos.c ppp.c
    ppp_ar.c preceph.c rcvraw.c rinex.c rtcm.c rtcm2.c rtcm3.c rtcm3e.c
    rtkcmn.c rtkpos.c rtksvr.c sbas.c sofa.c solution.c
    stream.c streamsvr.c tides.c tle.c trace.c batchls.c batchls.h
)


# Add the library (shared or static)
option(BUILD_SHARED_LIBS "Build rtklib as a shared library (DLL)" ON)

add_library(rtklib ${RTKLIB_SOURCES}
)
target_include_directories(rtklib
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)

target_compile_definitions(rtklib PRIVATE ENAGLO=1)

# Platform-specific compile definitions
if(WIN32)
    target_compile_definitions(rtklib PRIVATE WIN_DLL=1)
    target_link_libraries(rtklib PRIVATE ws2_32 winmm)
else()
    target_compile_definitions(rtklib PRIVATE DLL=1)
    target_link_libraries(rtklib PRIVATE m pthread)
endif()

target_include_directories(rtklib PUBLIC ${PROJECT_SOURCE_DIR}/..)
