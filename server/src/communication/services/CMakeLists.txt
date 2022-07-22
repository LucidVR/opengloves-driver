add_library(communication_services STATIC
        communication_service.h
        serial/service_serial.h
        bluetooth/service_bluetooth.h
        )

target_include_directories("${PROJECT_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(
        bluetooth/service_bluetooth_linux.h
        bluetooth/service_bluetooth_win.h

        serial/service_serial_linux.h
        serial/service_serial_win.h
)