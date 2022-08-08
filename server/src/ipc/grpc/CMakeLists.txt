find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(ipc_grpc STATIC grpc_device_output.h grpc_device_output.cpp)

target_link_libraries(ipc_grpc PRIVATE opengloves_protos)