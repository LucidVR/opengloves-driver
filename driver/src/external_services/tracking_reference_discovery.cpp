#include "tracking_reference_discovery.h"

#include <functional>

#include "grpc/grpc.h"
#include "grpcpp/server_builder.h"
#include "protos/driver/driver_input.grpc.pb.h"
#include "protos/driver/driver_input.pb.h"
#include "util/driver_log.h"

class TrackingReferenceDiscoveryService final : public driver_input::DriverInput::Service {
 public:
  explicit TrackingReferenceDiscoveryService(std::function<void(const TrackingReferenceResult& result)> tracking_reference_found_callback) {
    tracking_reference_found_callback_ = tracking_reference_found_callback;
  }

  grpc::Status TrackingReferenceFound(
      grpc::ServerContext* context, const driver_input::TrackingReferenceInfo* info, driver_input::TrackingReferenceResponse* response) override {
    DriverLog(
        "Tracking reference found for %s with id: %i",
        info->hand() == driver_input::TrackingReferenceInfo_Hand_RIGHT ? "right" : "left",
        info->openvr_id());

    TrackingReferenceResult result{
        info->openvr_id(),
        info->hand() == driver_input::TrackingReferenceInfo_Hand_RIGHT ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand};

    tracking_reference_found_callback_(result);

    return grpc::Status::OK;
  }

 private:
  std::function<void(const TrackingReferenceResult& result)> tracking_reference_found_callback_;
};

static std::unique_ptr<TrackingReferenceDiscoveryService> service;
static std::unique_ptr<grpc::Server> server;

TrackingReferenceDiscovery::TrackingReferenceDiscovery() {
  service = std::make_unique<TrackingReferenceDiscoveryService>(
      [&](const TrackingReferenceResult& result) { tracking_references_discovered_.insert_or_assign(result.role, result); });

  grpc::ServerBuilder builder;
  builder.AddListeningPort("0.0.0.0:52050", grpc::InsecureServerCredentials());

  builder.RegisterService(service.get());

  server = builder.BuildAndStart();

  is_initialised_ = true;
}

bool TrackingReferenceDiscovery::GetTrackingReferenceForRole(vr::ETrackedControllerRole role, TrackingReferenceResult& out_result) {
  if (!is_initialised_) {
    DriverLog("Cannot get tracking references as discovery failed to initialise");
    return false;
  }

  if (tracking_references_discovered_.find(role) == tracking_references_discovered_.end()) return false;

  out_result = tracking_references_discovered_.at(role);

  return true;
}

void TrackingReferenceDiscovery::Stop() {
  is_initialised_ = false;
  server->Shutdown();

  server.reset();
  service.reset();
}