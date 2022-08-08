#include "grpc_device_output.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include "protos/server/server_output.pb.h"
#include "protos/server/server_output.grpc.pb.h"


//class GRPCDeviceOutputImpl final : public server_output::DeviceOutput::Service {
// public:
//
//
//
//  grpc::Status GetDeviceInfos(grpc::ServerContext* context, const server_output::DeviceInfoRequest* request, server_output::DeviceInfoResponse* response) {
//
//  }
//};