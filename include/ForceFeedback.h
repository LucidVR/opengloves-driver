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
  void Stop();

 private:
  std::function<void(VRFFBData)> m_callback;
  vr::ETrackedControllerRole m_role;

  std::unique_ptr<NamedPipeListener<VRFFBData>> m_pipe;
};