find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenVR REQUIRED)

if(WIN32)
add_executable(opengloves_overlay WIN32 main.cpp)
else()
add_executable(opengloves_overlay main.cpp)
endif()

target_link_libraries(opengloves_overlay PRIVATE opengloves_protos OpenVR::OpenVR)