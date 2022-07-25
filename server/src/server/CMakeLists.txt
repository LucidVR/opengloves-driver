add_library(opengloves_server STATIC
        opengloves_server.cpp
        device_discovery.h
        device_discovery.cpp
        )

target_link_libraries(opengloves_server PRIVATE opengloves_server-includes communication_probers encoding_services)