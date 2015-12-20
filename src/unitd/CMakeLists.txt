add_executable(unitd
  askconsole.c
  early.c
  service/instance.c
  service/service.c
  signal.c
  state.c
  system.c
  ubus.c
  unit/queue.c
  unit/service.c
  unit/unit.c
  unitd.c
  utils.c
)
set_property(TARGET unitd PROPERTY COMPILE_FLAGS "-std=c99 -Wall -Dtypeof=__typeof__ -D_GNU_SOURCE ${JSON_C_CFLAGS_OTHER}")
set_property(TARGET unitd PROPERTY LINK_FLAGS "${JSON_C_LDFLAGS_OTHER}")
set_property(TARGET unitd PROPERTY INCLUDE_DIRECTORIES ${JSON_C_INCLUDE_DIR})
target_link_libraries(unitd ubox ubus blobmsg_json ${JSON_C_LIBRARIES})

install(TARGETS unitd RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/unitd)
