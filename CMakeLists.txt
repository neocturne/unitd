cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(unitd C)
include(GNUInstallDirs)


find_package(JSON_C REQUIRED)


add_executable(init unitd.c signal.c watchdog.c state.c askconsole.c ubus.c system.c early.c
  service/service.c service/instance.c utils/utils.c)
set_property(TARGET init PROPERTY COMPILE_FLAGS "${JSON_C_CFLAGS_OTHER}")
set_property(TARGET init PROPERTY LINK_FLAGS "${JSON_C_LDFLAGS_OTHER}")
set_property(TARGET init PROPERTY INCLUDE_DIRECTORIES ${JSON_C_INCLUDE_DIR})
target_link_libraries(init ubox ubus blobmsg_json ${JSON_C_LIBRARIES})

install(TARGETS init RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})


add_executable(askfirst utils/askfirst.c)

install(TARGETS askfirst RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})
