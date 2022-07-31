add_library(encoding_services STATIC
        encoding_service.h
        alpha_encoding/alpha_encoding_service.h
        alpha_encoding/alpha_encoding_service.cpp
        )

target_link_libraries(encoding_services PUBLIC communication-includes opengloves_server-includes)