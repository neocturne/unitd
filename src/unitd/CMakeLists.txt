add_executable(unitd unitd.c signal.c state.c askconsole.c ubus.c system.c early.c
  service/service.c service/instance.c utils.c)
set_property(TARGET unitd PROPERTY COMPILE_FLAGS "-std=gnu99 -Wall -D_GNU_SOURCE ${JSON_C_CFLAGS_OTHER}")
set_property(TARGET unitd PROPERTY LINK_FLAGS "${JSON_C_LDFLAGS_OTHER}")
set_property(TARGET unitd PROPERTY INCLUDE_DIRECTORIES ${JSON_C_INCLUDE_DIR})
target_link_libraries(unitd ubox ubus blobmsg_json ${JSON_C_LIBRARIES})

install(TARGETS unitd RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/unitd)
