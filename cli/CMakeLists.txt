cmake_minimum_required(VERSION 3.25)
project (eps_cli)

if (NOT DEFINED TGTNAME)
  message(FATAL_ERROR "TGTNAME must be defined on the cmake command line (e.g. \"-DTGTNAME=cpu1\")")
endif()

include(../../CompFlags.cmake)

if(${TGTNAME} STREQUAL cpu1)
  # Find ZeroMQ via pkg-config
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(ZeroMQ REQUIRED libzmq)
  
  # Find Simulith headers
  set(SIMULITH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../simulith" CACHE PATH "Path to Simulith directory")
  if(NOT EXISTS "${SIMULITH_DIR}/include/simulith_i2c.h")
      message(FATAL_ERROR "Could not find simulith_i2c.h in ${SIMULITH_DIR}/include. Please set SIMULITH_DIR to the correct path.")
  endif()

  # Only add Simulith subdirectory if not already added
  if(NOT TARGET simulith)
    # Set policy to NEW to allow external project linking
    cmake_policy(SET CMP0079 NEW)
    add_subdirectory(${SIMULITH_DIR} ${CMAKE_BINARY_DIR}/simulith)
  endif()
endif()


# Use target_include_directories for eps_cli
set(eps_cli_includes
    "./"
    "../fsw/platform_inc"
    "../fsw/src"
    "../shared"
    "../../../fsw/apps/hwlib/fsw/public_inc"
)

set(eps_cli_src
  eps_cli.c 
  ../shared/eps_device.c
)

# Hardware library switch based on TGTNAME
if(${TGTNAME} STREQUAL cpu1)
  list(APPEND eps_cli_includes 
    "${SIMULITH_DIR}/include" 
    "../../../fsw/apps/hwlib/sim/inc"
  )
  set(eps_cli_src 
    ${eps_cli_src}
    ${SIMULITH_DIR}/src/simulith_i2c.c
    ${SIMULITH_DIR}/src/simulith_common.c
    ../../../fsw/apps/hwlib/sim/src/libi2c.c
  )
  set(eps_cli_libs
    ${ZeroMQ_LIBRARIES}
    simulith
  )
endif()
if(${TGTNAME} STREQUAL cpu2)
  set(eps_cli_src 
    ${eps_cli_src}
    ../../../fsw/apps/hwlib/fsw/linux/libi2c.c
  )
endif()


add_executable(eps_cli ${eps_cli_src})
target_include_directories(eps_cli PRIVATE ${eps_cli_includes})
target_link_libraries(eps_cli ${eps_cli_libs})

if(${TGTNAME} STREQUAL cpu1)
  set_target_properties(eps_cli PROPERTIES COMPILE_FLAGS "-g" LINK_FLAGS "-g")
endif()
