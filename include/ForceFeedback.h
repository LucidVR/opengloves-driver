#pragma once

#include <memory>
#include <functional>

#include "openvr_driver.h"
#include "Util/NamedPipe.h"

struct VRFFBData_t {
  VRFFBData_t(short thumbCurl, short indexCurl, short middleCurl, short ringCurl, short pinkyCurl)
      : thumbCurl(thumbCurl), indexCurl(indexCurl), middleCurl(middleCurl), ringCurl(ringCurl), pinkyCurl(pinkyCurl){};

  short thumbCurl;
  short indexCurl;
  short middleCurl;
  short ringCurl;
  short pinkyCurl;
};

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