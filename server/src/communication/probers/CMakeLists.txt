add_library(communication_probers STATIC
        communication_prober.h

        serial/prober_serial.h
        bluetooth/prober_bluetooth.h
        )
set_target_properties(communication_probers PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
    target_sources(communication_probers
            PRIVATE
            serial/prober_serial_win.h
            serial/prober_serial_win.cpp

            bluetooth/prober_bluetooth_win.h
            bluetooth/prober_bluetooth_win.cpp
            )

    target_link_libraries(communication_probers PRIVATE setupapi wsock32 ws2_32 bthprops)
elseif (UNIX AND NOT APPLE)
    target_sources(communication_probers
            PRIVATE
            serial/prober_serial_linux.h
            serial/prober_serial_linux.cpp

            bluetooth/prober_bluetooth_linux.h
            bluetooth/prober_bluetooth_linux.cpp
            )
endif ()

target_link_libraries(communication_probers PRIVATE communication_services)
target_link_libraries(communication_probers PUBLIC communication-includes opengloves_server-includes)