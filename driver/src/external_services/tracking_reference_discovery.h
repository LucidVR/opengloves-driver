#pragma once

#include <functional>
#include <map>

#include "openvr_driver.h"

struct TrackingReferenceResult {
  uint32_t controller_id;
  vr::ETrackedControllerRole role;
};

class TrackingReferenceDiscovery {
 public:
  static TrackingReferenceDiscovery& GetInstance() {
    static TrackingReferenceDiscovery instance;

    return instance;
  };

  void AddCallback(std::function<void(const TrackingReferenceResult&)> callback);

  void Stop();

 private:
  TrackingReferenceDiscovery();

  bool is_initialised_;

 public:
  TrackingReferenceDiscovery(const TrackingReferenceDiscovery&) = delete;
  TrackingReferenceDiscovery& operator=(const TrackingReferenceDiscovery&) = delete;

 private:
  std::map<vr::ETrackedControllerRole, TrackingReferenceResult> tracking_references_discovered_;

  std::vector<std::function<void(const TrackingReferenceResult&)>> callbacks_;
};