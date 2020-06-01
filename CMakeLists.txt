
cmake_minimum_required(VERSION 3.10)

include(FindPkgConfig)

pkg_check_modules(SYSTEMD libsystemd>=211 REQUIRED)
set(LDFLAGS "${LDFLAGS}" systemd)

option(WITH_GPIO "add gpio set with dbus" OFF)
if(WITH_GPIO)
add_definitions(-DWITH_GPIO)
pkg_check_modules(PC_LIBGPIOD libgpiodcxx REQUIRED)
set(LDFLAGS "${LDFLAGS}" gpiodcxx)
endif(WITH_GPIO)

option(USE_HOST "add gpio set with dbus" OFF)
if(USE_HOST)
add_definitions(-DUSE_HOST)
endif(USE_HOST)
# cpp requirements
set (CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_executable(sd-server ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp)
add_executable(sd-client ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)

target_link_libraries(sd-server ${LDFLAGS})
target_link_libraries(sd-client ${LDFLAGS})

install (TARGETS sd-server DESTINATION sbin)
install (TARGETS sd-client DESTINATION sbin)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.linux.gateway.conf
	           DESTINATION /etc/dbus-1/system.d/)
