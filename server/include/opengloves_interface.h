// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>

namespace og {

  enum Hand { kHandLeft, kHandRight };

  enum EncodingType { kEncodingType_Alpha };
  enum CommunicationType { kCommunicationType_Serial, kCommunicationType_Bluetooth, kCommunicationType_Invalid };

  enum DeviceType {
    kDeviceType_lucidgloves,
  };

  struct DeviceAlphaEncodingConfiguration {
    unsigned int max_analog_value;
  };

  struct DeviceBluetoothCommunicationConfiguration {
    std::string name;  // must be set if auto probe is disabled
  };
  struct DeviceSerialCommunicationConfiguration {
    std::string port_name;  // must be set if auto probe is disabled
  };

  struct DeviceCommunicationConfiguration {
    DeviceSerialCommunicationConfiguration serial;
    DeviceBluetoothCommunicationConfiguration bluetooth;

    DeviceAlphaEncodingConfiguration encoding;
  };

  struct DeviceConfiguration {
    bool enabled;

    Hand hand;
    DeviceType type;
    DeviceCommunicationConfiguration communication;
  };

  struct BluetoothCommunicationConfiguration {
    bool enabled;
  };
  struct SerialCommunicationConfiguration {
    bool enabled;
  };
  struct NamedPipeCommunicationConfiguration {
    bool enabled;
  };

  struct CommunicationConfiguration {
    bool auto_probe;

    SerialCommunicationConfiguration serial;
    BluetoothCommunicationConfiguration bluetooth;
    NamedPipeCommunicationConfiguration named_pipe;
  };

  struct ServerConfiguration {
    CommunicationConfiguration communication;

    std::vector<DeviceConfiguration> devices;  // this doesn't need to be provided if auto probing is enabled
  };

  // IO structs
  struct Button {
    auto operator<=>(const Button&) const = default;
    float value;
    bool pressed;
  };

  struct Joystick {
    auto operator<=>(const Joystick&) const = default;
    float x;
    float y;
    bool pressed;
  };

  struct Gesture {
    auto operator<=>(const Gesture&) const = default;
    bool activated;
  };

  // input data from device to server about buttons, joysticks, etc.
  struct InputPeripheralData {
    auto operator<=>(const InputPeripheralData&) const = default;

    std::array<std::array<float, 4>, 5> flexion;
    std::array<float, 5> splay;

    Button trigger;
    Button A;
    Button B;
    Button menu;
    Button calibrate;

    Joystick joystick;

    Gesture grab;
    Gesture pinch;
  };

  struct InputInfoData {
    auto operator<=>(const InputInfoData&) const = default;

    Hand hand;
    DeviceType device_type;

    int firmware_version;
  };

  union InputData {
    InputInfoData info;
    InputPeripheralData peripheral;
  };

  enum InputDataType { kInputDataType_Invalid, kInputDataType_Info, kInputDataType_Peripheral };

  // input data from glove to server
  struct Input {
    InputData data;
    InputDataType type;
  };

  // force feedback output data from server to device
  struct OutputForceFeedbackData {
    int16_t thumb;
    int16_t index;
    int16_t middle;
    int16_t ring;
    int16_t pinky;
  };
  // haptic vibration output from server to device
  struct OutputHapticData {
    float duration;
    float frequency;
    float amplitude;
  };
  struct OutputFetchInfoData {
    bool start_streaming;
    bool get_info;
  };

  union OutputData {
    OutputFetchInfoData fetch_info;
    OutputHapticData haptic_data;
    OutputForceFeedbackData force_feedback_data;
  };
  enum OutputDataType { kOutputDataType_Empty, kOutputDataType_FetchInfo, kOutputDataType_Haptic, kOutputData_Type_ForceFeedback };

  // output data from driver to glove
  struct Output {
    OutputDataType type;
    OutputData data;
  };

  class IDevice {
   public:
    virtual DeviceConfiguration GetConfiguration() = 0;

    virtual void ListenForInput(std::function<void(const InputPeripheralData& data)> callback) = 0;

    virtual void Output(const Output& output) = 0;

    virtual ~IDevice() = default;
  };

  class IDeviceDiscoverer {
   public:
    virtual void StartDiscovery(std::function<void(std::unique_ptr<og::IDevice> device)> callback) = 0;

    virtual ~IDeviceDiscoverer() = default;
  };

  class Server {
   public:
    explicit Server(ServerConfiguration configuration);

    /***
     * Start looking for devices. The callback will be called for every new device found.
     */
    bool StartProber(std::function<void(std::unique_ptr<IDevice> device)> callback);

    /***
     * Stop looking for devices.
     * @return
     */
    bool StopProber();

    ~Server();

   private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
  };

  enum LoggerLevel { kLoggerLevel_Info, kLoggerLevel_Warning, kLoggerLevel_Error };

  class Logger {
   public:
    static Logger& GetInstance() {
      static Logger instance;

      return instance;
    };

    void SubscribeToLogger(std::function<void(const std::string& message, LoggerLevel level)> callback) {
      callbacks_.emplace_back(callback);
    }

    template <typename... Args>
    void Log(LoggerLevel level, const char* format, Args... args) {
      const std::string message = StringFormat(format, args...);

      if (last_message_ == message) return;
      last_message_ = message;

      for (auto& callback : callbacks_) {
        callback(message, level);
      }
    }

   private:
    Logger() = default;
    ;

   public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

   private:
    std::vector<std::function<void(const std::string& message, LoggerLevel level)>> callbacks_;

    std::string last_message_;

    template <typename... Args>
    static std::string StringFormat(const std::string& format, Args... args) {
      int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
      if (size_s <= 0) {
        return "unknown (bad formatting)";
      }
      auto size = static_cast<size_t>(size_s);
      auto buf = std::make_unique<char[]>(size);
      std::snprintf(buf.get(), size, format.c_str(), args...);
      return {buf.get(), buf.get() + size - 1};
    }
  };
}  // namespace og