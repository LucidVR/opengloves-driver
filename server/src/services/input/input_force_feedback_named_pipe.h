// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

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

  void StartListener();

  ~InputForceFeedbackNamedPipe();
 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;
};