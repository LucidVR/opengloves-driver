#pragma once

#include <array>
#include <functional>
#include <string>

namespace og {
  enum Hand { kHandLeft, kHandRight };

  // config structs
  struct BluetoothConfiguration {
    std::string name;
  };

  struct SerialConfiguration {
    std::string port_name;
  };

  struct EncodingConfiguration {
    unsigned int max_analog_value;
  };

  struct LegacyConfiguration {
    Hand hand;

    SerialConfiguration serial_configuration;
    BluetoothConfiguration bluetooth_configuration;

    EncodingConfiguration encoding_configuration;
  };

  // IO structs
  struct Button {
    float value;
    bool pressed;
  };

  struct Joystick {
    float x;
    float y;
    bool pressed;
  };

  struct Gesture {
    bool activated;
  };

  struct InputInfoData {
    int firmware_version;
  };

  // input data from device to server about buttons, joysticks, etc.
  struct InputPeripheralData {
    float flexion[5][4];
    float splay[5];

    Button trigger;
    Button A;
    Button B;
    Button menu;
    Button calibrate;

    Joystick joystick;

    Gesture grab;
    Gesture pinch;
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

  struct OutputFetchInfoData {};

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

  class Device {
   public:
  };

  class Server {
   public:
    /**
     * Used to provide legacy configuration values (explicitly setting comm ports, bluetooth name, encoding, etc.)
     * Not needed for newer versions of the firmware.
     * Not calling this before starting to probe for devices is fine, but means that any devices running firmware
     * where we can't get data from them will be dropped.
     */
    void SetLegacyConfiguration(const LegacyConfiguration& configuration);

    /***
     * Start looking for devices. The callback will be called for every new device found.
     */
    int StartProber(std::function<void(Device* device)>);

    int StopProber();

    ~Server();

   private:
    LegacyConfiguration legacy_configuration_;
  };

  enum LoggerLevel { kLoggerLevel_Info, kLoggerLevel_Warning, kLoggerLevel_Error };

  class Logger {
   public:
    static Logger& GetInstance() {
      static Logger instance;

      return instance;
    };

    void SubscribeToLogger(std::function<void(const std::string& message, LoggerLevel level)>& callback) {
      callbacks_.emplace_back(callback);
    }

    template <typename... Args>
    void Log(LoggerLevel level, const char* format, Args... args) {
      const std::string message = StringFormat(format, args...);

      for (auto& callback : callbacks_) {
        callback(message, level);
      }
    }

   private:
    Logger(){};

   public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

   private:
    std::vector<std::function<void(const std::string& message, LoggerLevel level)>> callbacks_;

    template <typename... Args>
    static std::string StringFormat(const std::string& format, Args... args) {
      int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;
      if (size_s <= 0) {
        return "unknown (bad formatting)";
      }
      auto size = static_cast<size_t>(size_s);
      auto buf = std::make_unique<char[]>(size);
      std::snprintf(buf.get(), size, format.c_str(), args...);
      return std::string(buf.get(), buf.get() + size - 1);
    }
  };
}  // namespace og