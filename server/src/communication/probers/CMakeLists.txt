add_library(communication_probers STATIC
        communication_prober.h

        prober_serial.h
        prober_bluetooth.h
        )
set_target_properties(communication_probers PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
    target_sources(communication_probers
            PRIVATE
            prober_serial_win.h
            prober_serial_win.cpp

            prober_bluetooth_win.h
            prober_bluetooth_win.cpp
            )

    target_link_libraries(communication_probers PRIVATE setupapi wsock32 ws2_32 bthprops)
elseif (UNIX AND NOT APPLE)
    target_sources(communication_probers
            PRIVATE
            prober_serial_linux.h
            prober_serial_linux.cpp

            prober_bluetooth_linux.h
            prober_bluetooth_linux.cpp
            )
endif ()

target_link_libraries(communication_probers PRIVATE communication_services)
target_link_libraries(communication_probers PUBLIC opengloves_interface-includes)