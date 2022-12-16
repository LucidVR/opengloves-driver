add_library(communication_services STATIC
        communication_service.h
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
elseif (UNIX AND NOT APPLE)
    target_sources(communication_services
            PRIVATE
            service_serial_linux.h
            service_serial_linux.cpp

            service_bluetooth_linux.h
            service_bluetooth_linux.cpp
            )

    find_package(bluetooth MODULE)

    target_link_libraries(communication_services PRIVATE bluetooth)
endif ()


target_link_libraries(communication_services PUBLIC server-includes opengloves_interface-includes)