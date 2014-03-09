
cmake_minimum_required(VERSION 2.8)

project(psf2wav)

set(CMAKE_C_FLAGS_RELEASE "-Wall -O3")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3")
set(CMAKE_C_FLAGS_DEBUG "-Wall -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g")

add_executable(psf2wav
        src/psf2wav.cpp
        sexypsf/PsxBios.c
        sexypsf/PsxCounters.c
        sexypsf/PsxDma.c
        sexypsf/Spu.c
        sexypsf/PsxHw.c
        sexypsf/PsxMem.c
        sexypsf/Misc.c
        sexypsf/R3000A.c
        sexypsf/PsxInterpreter.c
        sexypsf/PsxHLE.c
        sexypsf/spu/spu.c
)

target_link_libraries(psf2wav z)
