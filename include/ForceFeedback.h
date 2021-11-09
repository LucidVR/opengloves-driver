#pragma once

#include <functional>
#include <memory>

#include "Encode/EncodingManager.h"
#include "Util/NamedPipeListener.h"
#include "openvr_driver.h"

class FFBListener {
 public:
  FFBListener(std::function<void(VRFFBData)> callback, vr::ETrackedControllerRole role);
  void Start();
  void Stop() const;

 private:
  std::function<void(VRFFBData)> _callback;
  vr::ETrackedControllerRole _role;

  std::unique_ptr<NamedPipeListener<VRFFBData>> _pipe;
};