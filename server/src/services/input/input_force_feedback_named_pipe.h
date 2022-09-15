#pragma once

#include <functional>
#include <memory>

#include "opengloves_interface.h"

struct ForceFeedbackCurlData {
  short thumb;
  short index;
  short middle;
  short ring;
  short pinky;
};

class InputForceFeedbackNamedPipe {
 public:
  InputForceFeedbackNamedPipe(og::Hand hand, std::function<void(const ForceFeedbackCurlData&)> on_data_callback);

  ~InputForceFeedbackNamedPipe();
 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;
};