# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(communication_services STATIC
        communication_service.h

        service_bluetooth.h
        service_serial.h
        )
set_target_properties(communication_services PROPERTIES LINKER_LANGUAGE CXX)

if (WIN32)
    target_sources(communication_services
            PRIVATE
            service_serial_win.h
            service_serial_win.cpp

            service_bluetooth_win.h
            service_bluetooth_win.cpp
            )
endif ()

target_link_libraries(communication_services PUBLIC server-includes opengloves_interface-includes)