
cmake_minimum_required(VERSION 2.8)
project(reaper)

## Version ##

set(REAPER_VERSION "v13")

## Options ##

option(CPU_MINING_ONLY "Do not compile the GPU mining part" OFF)
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
option(REAPER_BUILD_64BIT "AUTOMATIC, DO NOT TOUCH: Compiling 64bit" ON)
else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
option(REAPER_BUILD_32BIT "AUTOMATIC, DO NOT TOUCH: Compiling 32bit" ON)
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )


## Global config ##

configure_file(
    "${PROJECT_SOURCE_DIR}/CMakeConf.h.in"
    "${PROJECT_BINARY_DIR}/CMakeConf.h"
)

## Source files ##

set(SOURCES
    ../src/App.cpp
    ../src/AppOpenCL.cpp
    ../src/Blake512.cpp
    ../src/Config.cpp
	../src/CPUAlgos.cpp
    ../src/CPUMiner.cpp
    ../src/Curl.cpp
    ../src/json_reader.cpp
    ../src/json_value.cpp
    ../src/json_writer.cpp
    ../src/main.cpp
    ../src/RSHash.cpp
	../src/scrypt.cpp
    ../src/ServerSettings.cpp
    ../src/SHA256.cpp
    ../src/Util.cpp
)

set(HEADERS 
	../headers/App.h
	../headers/AppOpenCL.h
	../headers/Blake512.h
	../headers/Config.h
	../headers/CPUAlgos.h
	../headers/CPUMiner.h
	../headers/Curl.h
	../headers/Global.h
	../headers/RSHash.h
	../headers/ServerSettings.h
	../headers/SHA256.h
	../headers/Util.h
)

## Targets ##

add_executable(reaper ${SOURCES} ${HEADERS})

include_directories(${PROJECT_BINARY_DIR})

set_target_properties(reaper PROPERTIES COMPILE_FLAGS -O2)

## OpenCL detection ##
if(NOT CPU_MINING_ONLY)
    
    if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		find_library(OPENCL_LIBRARY OpenCL PATHS "$ENV{AMDAPPSDKROOT}/lib/x86_64")
	else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		find_library(OPENCL_LIBRARY OpenCL PATHS "$ENV{AMDAPPSDKROOT}/lib/x86")
	endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    
    if(APPLE)
        find_path(OPENCL_INCLUDE_DIR OpenCL/cl.h PATHS "$ENV{AMDAPPSDKROOT}/include")
    else(APPLE)
        find_path(OPENCL_INCLUDE_DIR CL/cl.h PATHS "$ENV{AMDAPPSDKROOT}/include")
    endif(APPLE)
    
    mark_as_advanced(OPENCL_INCLUDE_DIR OPENCL_LIBRARY)
    
    include_directories(${OPENCL_INCLUDE_DIR})
    
endif(NOT CPU_MINING_ONLY)

## Detection of the rest of the libs ##

find_library(PTHREAD_LIBRARY pthread)
find_path(PTHREAD_INCLUDE_DIR NAMES pthread.h)

find_library(CURL_LIBRARY curl)
find_path(CURL_INCLUDE_DIR NAMES curl.h PATH_SUFFIXES curl)

include_directories(${PTHREAD_INCLUDE_DIR} ${CURL_INCLUDE_DIR})
link_directories(${PTHREAD_LIBRARY} ${CURL_LIBRARY})

## Library configuration ##

if(CPU_MINING_ONLY)
    target_link_libraries(reaper ${PTHREAD_LIBRARY} ${CURL_LIBRARY})
else(CPU_MINING_ONLY)
    target_link_libraries(reaper ${PTHREAD_LIBRARY} ${CURL_LIBRARY} ${OPENCL_LIBRARY})
endif(CPU_MINING_ONLY)

## Make MSVC use Runtime Library: "Multithreaded" instead of "Multithreaded DLL"

if(MSVC)
	foreach(flag_var
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
endif(MSVC)
