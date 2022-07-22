add_library(communication_probers STATIC
        communication_prober.h
        bluetooth/prober_bluetooth.h
        serial/prober_serial.h
        )

target_include_directories("${PROJECT_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries("${PROJECT_NAME}" communication_services)

target_sources("${PROJECT_NAME}"
        serial/prober_serial_linux.h
        serial/prober_serial_linux.cpp
        serial/prober_serial_win.h
        serial/prober_serial_win.cpp

        bluetooth/prober_bluetooth_linux.h
        bluetooth/prober_bluetooth_linux.cpp
        bluetooth/prober_bluetooth_win.h
        bluetooth/prober_bluetooth_win.cpp
        )