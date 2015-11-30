add_executable(askfirst askfirst.c)

install(TARGETS askfirst RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/unitd)
