# Copyright (c) 2023 LucidVR
# SPDX-License-Identifier: MIT

add_library(encoding_services STATIC
        encoding_service.h
        alpha_encoding_service.h
        alpha_encoding_service.cpp
        )

target_link_libraries(encoding_services PUBLIC opengloves_interface-includes)