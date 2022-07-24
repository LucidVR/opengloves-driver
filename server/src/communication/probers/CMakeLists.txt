add_library(communication_probers STATIC
        communication_prober.h
        prober_manager.cpp

        serial/prober_serial.h
        serial/prober_serial_linux.h
        serial/prober_serial_linux.cpp
        serial/prober_serial_win.h
        serial/prober_serial_win.cpp

        bluetooth/prober_bluetooth.h
        bluetooth/prober_bluetooth_linux.h
        bluetooth/prober_bluetooth_linux.cpp
        bluetooth/prober_bluetooth_win.h
        bluetooth/prober_bluetooth_win.cpp
        )
set_target_properties(communication_probers PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(communication_probers PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(communication_probers PRIVATE communication_services)
target_link_libraries(communication_probers PUBLIC communication-includes opengloves_server-includes)