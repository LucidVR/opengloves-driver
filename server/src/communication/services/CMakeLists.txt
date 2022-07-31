add_library(communication_services STATIC
        communication_service.h

        bluetooth/service_bluetooth.h
        serial/service_serial.h
        )
set_target_properties(communication_services PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
    target_sources(communication_services
            PRIVATE
            serial/service_serial_win.h
            serial/service_serial_win.cpp

            bluetooth/service_bluetooth_win.h
            bluetooth/service_bluetooth_win.cpp
            )
elseif (UNIX AND NOT APPLE)
    target_sources(communication_services
            PRIVATE
            bluetooth/service_bluetooth_linux.h

            serial/service_serial_linux.h
            )
endif ()

target_link_libraries(communication_services PUBLIC communication-includes)
target_link_libraries(communication_services PRIVATE opengloves_server-includes)