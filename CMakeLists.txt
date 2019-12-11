
cmake_minimum_required(VERSION 3.10)

include(FindPkgConfig)

pkg_check_modules(SYSTEMD libsystemd>=211 REQUIRED)
set(LDFLAGS "${LDFLAGS}" systemd)

pkg_check_modules(PC_LIBGPIOD libgpiodcxx REQUIRED)
set(LDFLAGS "${LDFLAGS}" gpiodcxx)

if(USE_HOST)
add_definitions(-DUSE_HOST)
endif(USE_HOST)
# cpp requirements
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

# check results and add flag
if(COMPILER_SUPPORTS_CXX11)#
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)#
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_executable(sd-server ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp)
add_executable(sd-client ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)

target_link_libraries(sd-server ${LDFLAGS})
target_link_libraries(sd-client ${LDFLAGS})

install (TARGETS sd-server DESTINATION sbin)
install (TARGETS sd-client DESTINATION sbin)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.linux.gateway.conf
	           DESTINATION /etc/dbus-1/system.d/)
