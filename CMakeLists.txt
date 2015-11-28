cmake_minimum_required(VERSION 2.6)

PROJECT(procd C)
INCLUDE(GNUInstallDirs)
ADD_DEFINITIONS(-Os -ggdb -Wall -Werror --std=gnu99 -Wmissing-declarations)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")


SET(SOURCES procd.c signal.c watchdog.c state.c	inittab.c rcS.c	ubus.c system.c early.c
	service/service.c service/instance.c service/trigger.c service/watch.c
	utils/utils.c)

SET(LIBS ubox ubus json-c blobmsg_json json_script)

IF(DEBUG)
  ADD_DEFINITIONS(-DDEBUG -g3)
ENDIF()

ADD_EXECUTABLE(init ${SOURCES})
TARGET_LINK_LIBRARIES(init ${LIBS})
INSTALL(TARGETS init
	RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)


ADD_EXECUTABLE(askfirst utils/askfirst.c)
INSTALL(TARGETS askfirst
	RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)
