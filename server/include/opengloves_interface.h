#pragma once

#include <functional>
#include <string>

namespace og {

  enum Hand { kHandLeft, kHandRight };

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

  class Device {};

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
        throw std::runtime_error("Error during formatting.");
      }
      auto size = static_cast<size_t>(size_s);
      auto buf = std::make_unique<char[]>(size);
      std::snprintf(buf.get(), size, format.c_str(), args...);
      return std::string(buf.get(), buf.get() + size - 1);
    }
  };
}  // namespace og