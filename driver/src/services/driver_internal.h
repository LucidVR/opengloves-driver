// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include <functional>
#include <map>
#include <memory>

#include "openvr_driver.h"

struct TrackingReferenceResult {
  uint32_t controller_id;
  vr::ETrackedControllerRole role;
};

class DriverInternalServer {
 public:
  static DriverInternalServer& GetInstance() {
    static DriverInternalServer instance;

    return instance;
  };

  void AddTrackingReferenceRequestCallback(std::function<void(const TrackingReferenceResult&)> callback);

  void Stop();

  ~DriverInternalServer();

 private:
  DriverInternalServer();

 public:
  DriverInternalServer(const DriverInternalServer&) = delete;
  DriverInternalServer& operator=(const DriverInternalServer&) = delete;

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  std::map<vr::ETrackedControllerRole, TrackingReferenceResult> tracking_references_discovered_;
  std::vector<std::function<void(const TrackingReferenceResult&)>> tracking_reference_callbacks_;
};