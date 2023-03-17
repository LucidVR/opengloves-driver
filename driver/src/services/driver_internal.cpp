// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "driver_internal.h"

#include <future>

#include "crow.h"
#include "webserver_logging.h"

class DriverInternalServer::Impl {
 public:
  explicit Impl(std::function<crow::response(std::string path, const crow::request& req)> callback) {
    DriverServerLog::GetInstance();

    callback_ = callback;
    // set up the routes
    CROW_ROUTE(app_, "/tracking_reference").methods("POST"_method)([&](const crow::request& req) { return callback_("tracking_reference", req); });

    server_ = app_.port(52061).multithreaded().run_async();
  }

  void Stop() {
    app_.stop();
  }

  ~Impl() {
    Stop();
  }

 private:
  crow::SimpleApp app_;

  std::future<void> server_;

  std::function<crow::response(std::string path, const crow::request& req)> callback_;
};

DriverInternalServer::DriverInternalServer() {
  pImpl_ = std::make_unique<DriverInternalServer::Impl>([&](std::string path, const crow::request& req) {
    if (path == "tracking_reference") {
      auto body = crow::json::load(req.body);
      if (!body) return crow::response(crow::status::BAD_REQUEST);

      auto controller_role = static_cast<vr::ETrackedControllerRole>(body["openvr_role"].i());
      auto controller_id = static_cast<uint32_t>(body["openvr_id"].i());

      TrackingReferenceResult result{controller_id, controller_role};
      tracking_references_discovered_.insert_or_assign(controller_role, result);

      for (const auto& callback : tracking_reference_callbacks_) {
        callback(result);
      }

      return crow::response{"ok"};
    }

    return crow::response{crow::status::NOT_FOUND};
  });
}

void DriverInternalServer::AddTrackingReferenceRequestCallback(std::function<void(const TrackingReferenceResult&)> callback) {
  tracking_reference_callbacks_.emplace_back(callback);

  // make sure the callback has all the references we've already found
  for (const auto& reference : tracking_references_discovered_) {
    callback(reference.second);
  }
}

void DriverInternalServer::Stop() {
  pImpl_->Stop();
}

DriverInternalServer::~DriverInternalServer() = default;