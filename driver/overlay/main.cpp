// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#ifdef WIN32
#include <Windows.h>
#endif

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <thread>

#include "nlohmann/json.hpp"
#include "openvr.h"
#include "restclient-cpp/restclient.h"

static std::string this_manufacturer = "LucidVR";

static std::atomic<bool> app_active = false;

enum ServerAddress {
  kServerAddress_DriverInternal,
};

std::map<ServerAddress, std::string> server_addresses_{{kServerAddress_DriverInternal, "http://localhost:52061"}};

void DiscoveryThread(vr::ETrackedControllerRole role, std::function<void(uint32_t id)> callback) {
  uint32_t last_found_id = -1;
  uint32_t found_id = -1;
  while (app_active) {
    for (uint32_t i = 1; i < vr::k_unMaxTrackedDeviceCount; i++) {
      char manufacturer[4096];
      vr::VRSystem()->GetStringTrackedDeviceProperty(i, vr::Prop_ManufacturerName_String, manufacturer, sizeof manufacturer);
      if (manufacturer == this_manufacturer) continue;

      const int32_t controller_hint = vr::VRSystem()->GetInt32TrackedDeviceProperty(i, vr::Prop_ControllerRoleHint_Int32);
      if (controller_hint == role) {
        found_id = i;
        break;
      }

      const int32_t controller_role = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(i);
      if (controller_role == role) {
        found_id = i;
      }
    }

    if (found_id != last_found_id) {
      last_found_id = found_id;

      callback(found_id);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

#ifdef WIN32
int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPWSTR nCmdLine, int nCmdShow)
#endif
#ifndef WIN32
    int main(int argc, char **argv)
#endif

{
  vr::EVRInitError err;
  VR_Init(&err, vr::VRApplication_Background);

  if (err != vr::VRInitError_None) {
    std::cout << "Failed to initialise background process. Error: %s" << err << std::endl;
    return 1;
  }

  std::thread left_controller_thread = std::thread(DiscoveryThread, vr::TrackedControllerRole_LeftHand, [&](uint32_t id) {
    nlohmann::json json;
    json["openvr_id"] = id;
    json["openvr_role"] = vr::TrackedControllerRole_LeftHand;

    RestClient::Response response =
        RestClient::post(server_addresses_.at(kServerAddress_DriverInternal) + "/tracking_reference", "application/json", json.dump());
  });

  std::thread right_controller_thread = std::thread(DiscoveryThread, vr::TrackedControllerRole_RightHand, [&](uint32_t id) {
    nlohmann::json json;
    json["openvr_id"] = id;
    json["openvr_role"] = vr::TrackedControllerRole_RightHand;

    RestClient::Response response =
        RestClient::post(server_addresses_.at(kServerAddress_DriverInternal) + "/tracking_reference", "application/json", json.dump());
  });

  app_active = true;
  while (app_active) {
    vr::VREvent_t event{};
    while (vr::VRSystem() && vr::VRSystem()->PollNextEvent(&event, sizeof event)) {
      switch (event.eventType) {
        case vr::VREvent_Quit:
          vr::VRSystem()->AcknowledgeQuit_Exiting();
          app_active = false;

          left_controller_thread.join();
          right_controller_thread.join();

          vr::VR_Shutdown();

          return 0;
      }
    }
    Sleep(200);
  }

  return 0;
}