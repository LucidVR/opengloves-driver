# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(communication_probers STATIC
        prober.h

        prober_serial_connectable.cpp prober_serial_connectable.h prober_bluetooth_connectable.cpp prober_bluetooth_connectable.h)
set_target_properties(communication_probers PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
    target_sources(communication_probers
            PRIVATE
            prober_serial_identifiers_win.h
            prober_serial_identifiers_win.cpp

            prober_bluetooth_identifiers_win.h
            prober_bluetooth_identifiers_win.cpp
            )

    target_link_libraries(communication_probers PRIVATE server_lib setupapi wsock32 ws2_32 bthprops)
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