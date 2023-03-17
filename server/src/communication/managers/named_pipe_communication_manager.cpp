// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "named_pipe_communication_manager.h"

#include <regex>
#include <utility>

#include "named_pipe/named_pipe_win.h"
#include "opengloves_interface.h"

static og::Logger& logger = og::Logger::GetInstance();

namespace NamedPipeInputDataVersion {
  struct v1 {
    const std::array<std::array<float, 4>, 5> flexion;
    const std::array<float, 5> splay = {-2.0f, -2.0f, -2.0f, -2.0f, -2.0f};
    const float joy_x;
    const float joy_y;
    const bool joy_button;
    const bool trigger_button;
    const bool a_button;
    const bool b_button;
    const bool grab;
    const bool pinch;
    const bool menu;
    const bool calibrate;
  };

  struct v2 {
    std::array<std::array<float, 4>, 5> flexion;
    std::array<float, 5> splay;
    float joy_x;
    float joy_y;
    bool joy_button;
    bool trigger_button;
    bool a_button;
    bool b_button;
    bool grab;
    bool pinch;
    bool menu;
    bool calibrate;

    // new
    float trigger_value;
  };
}  // namespace NamedPipeInputDataVersion

struct NamedPipeInputData : public NamedPipeInputDataVersion::v2 {
  NamedPipeInputData() : NamedPipeInputDataVersion::v2(){};

  NamedPipeInputData(const NamedPipeInputDataVersion::v1& data) {
    flexion = data.flexion;
    splay = data.splay;
    joy_x = data.joy_x;
    joy_y = data.joy_y;
    joy_button = data.joy_button;
    trigger_button = data.trigger_button;
    trigger_value = data.trigger_button;
    a_button = data.a_button;
    b_button = data.b_button;
    grab = data.grab;
    pinch = data.pinch;
    menu = data.menu;
    calibrate = data.calibrate;
  }

  NamedPipeInputData(const NamedPipeInputDataVersion::v2& data) {
    flexion = data.flexion;
    splay = data.splay;
    joy_x = data.joy_x;
    joy_y = data.joy_y;
    joy_button = data.joy_button;
    trigger_button = data.trigger_button;
    trigger_value = data.trigger_value;
    a_button = data.a_button;
    b_button = data.b_button;
    grab = data.grab;
    pinch = data.pinch;
    menu = data.menu;
    calibrate = data.calibrate;
    trigger_value = data.trigger_value;
  }
};

class NamedPipeCommunicationManager::Impl {
 public:
  Impl(og::Hand hand, std::function<void()> on_client_connected_callback)
      : hand_(hand), on_client_connected_callback_(std::move(on_client_connected_callback)){};

  void StartListener(std::function<void(const NamedPipeInputData&)> on_data_callback) {
    on_data_callback_ = std::move(on_data_callback);

    std::string base_name = R"(\\.\pipe\vrapplication\input\glove\$version\)" + std::string(hand_ == og::kHandLeft ? "left" : "right");

    // v1
    named_pipes_.emplace_back(std::make_unique<NamedPipeListener<NamedPipeInputDataVersion::v1>>(
        std::regex_replace(base_name, std::regex("\\$version"), "v1"),
        [&](const NamedPipeListenerEvent& event) { OnEvent(event); },
        [&](NamedPipeInputDataVersion::v1* data) { on_data_callback_(static_cast<NamedPipeInputData>(*data)); }));
    // v2
    named_pipes_.emplace_back(std::make_unique<NamedPipeListener<NamedPipeInputDataVersion::v2>>(
        std::regex_replace(base_name, std::regex("\\$version"), "v2"),
        [&](const NamedPipeListenerEvent& event) { OnEvent(event); },
        [&](NamedPipeInputDataVersion::v2* data) { on_data_callback_(static_cast<NamedPipeInputData>(*data)); }));

    for (const auto& pipe : named_pipes_) {
      pipe->StartListening();
    }

    is_listening_ = true;
  }

  [[nodiscard]] bool IsListening() const {
    return is_listening_;
  }

 private:
  void OnEvent(const NamedPipeListenerEvent& event) {
    switch (event.type) {
      case NamedPipeListenerEventType::ClientConnected:
        if (client_registered_) return;
        on_client_connected_callback_();
        client_registered_ = true;
        break;
    }
  }

  bool is_listening_ = false;
  bool client_registered_ = false;
  og::Hand hand_;
  std::vector<std::unique_ptr<INamedPipeListener>> named_pipes_;
  std::function<void()> on_client_connected_callback_;
  std::function<void(const NamedPipeInputData&)> on_data_callback_;
};

NamedPipeCommunicationManager::NamedPipeCommunicationManager(og::Hand hand, std::function<void()> on_client_connected)
    : on_client_connected_(std::move(on_client_connected)), pImpl_(std::make_unique<Impl>(hand, [&]() { on_client_connected_(); })){};

void NamedPipeCommunicationManager::BeginListener(std::function<void(const og::Input&)> callback) {
  on_data_callback_ = std::move(callback);

  if (pImpl_->IsListening()) return;

  pImpl_->StartListener([&](const NamedPipeInputData& pipe_data) {
    logger.Log(og::kLoggerLevel_Info, "Received named pipe data");

    og::Input result{};
    result.type = og::kInputDataType_Peripheral;

    og::InputPeripheralData& data = result.data.peripheral;

    data.flexion = pipe_data.flexion;
    data.splay = pipe_data.splay;

    data.A.pressed = pipe_data.a_button;
    data.A.value = pipe_data.a_button;

    data.B.pressed = pipe_data.b_button;
    data.B.value = pipe_data.b_button;

    data.trigger.pressed = pipe_data.trigger_button;
    data.trigger.value = pipe_data.trigger_value;

    data.joystick.x = pipe_data.joy_x;
    data.joystick.y = pipe_data.joy_y;
    data.joystick.pressed = pipe_data.joy_button;

    data.calibrate.pressed = pipe_data.calibrate;
    data.calibrate.value = pipe_data.calibrate;

    data.grab.activated = pipe_data.grab;
    data.pinch.activated = pipe_data.pinch;

    data.menu.pressed = pipe_data.menu;
    data.menu.value = pipe_data.menu;

    on_data_callback_(result);
  });
}

void NamedPipeCommunicationManager::WriteOutput(const og::Output& output) {
  // not implemented yet
}

NamedPipeCommunicationManager::~NamedPipeCommunicationManager() = default;