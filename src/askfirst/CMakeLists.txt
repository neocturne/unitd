add_executable(askfirst askfirst.c)
set_property(TARGET askfirst PROPERTY COMPILE_FLAGS "-std=c99 -Wall")

install(TARGETS askfirst RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/unitd)
