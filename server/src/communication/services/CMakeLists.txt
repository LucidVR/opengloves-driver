project(communication_services)

add_library(communication_services STATIC
        communication_service.h
        serial/service_serial.h
        bluetooth/service_bluetooth.h

        bluetooth/service_bluetooth_linux.h
        bluetooth/service_bluetooth_win.h

        serial/service_serial_linux.h
        serial/service_serial_win.h
        serial/service_serial_win.cpp
        )

set_target_properties(communication_services PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(communication_services PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(communication_services communication-includes)