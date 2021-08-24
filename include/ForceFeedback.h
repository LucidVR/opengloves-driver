#pragma once

#include "openvr_driver.h"

#include <memory>
#include <functional>

#include "Communication/CommunicationObjects.h"
#include "Util/NamedPipe.h"

class FFBListener {
 public:
  FFBListener(std::function<void(VRFFBData_t)> callback, vr::ETrackedControllerRole role);
  void Start();
  void Stop();

 private:
  std::function<void(VRFFBData_t)> m_callback;
  vr::ETrackedControllerRole m_role;

  std::unique_ptr<NamedPipeUtil> m_pipe;
};