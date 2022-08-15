#include "driver_external.h"

#include "crow.h"
#include "device/configuration/device_configuration.h"
#include "nlohmann/json.hpp"
#include "webserver_logging.h"

class DriverExternalServer::Impl {
 public:
  Impl() {
    DriverServerLog::GetInstance();

    CROW_ROUTE(app_, "/settings").methods("GET"_method)([&](const crow::request& req) {
      nlohmann::ordered_json json;

      nlohmann::ordered_map<std::string, std::variant<bool>> driver_configuration = GetDriverConfigurationMap();
      for (auto& [key, value] : driver_configuration) {
        std::visit([&](auto&& v) { json[k_driver_settings_section][key] = v; }, value);
      }

      nlohmann::ordered_map<std::string, std::variant<bool, std::string>> serial_configuration = GetSerialConfigurationMap();
      for (auto& [key, value] : serial_configuration) {
        std::visit([&](auto&& v) { json[k_serial_communication_settings_section][key] = v; }, value);
      }

      nlohmann::ordered_map<std::string, std::variant<bool, std::string>> bt_serial_configuration = GetBluetoothSerialConfigurationMap();
      for (auto& [key, value] : bt_serial_configuration) {
        std::visit([&](auto&& v) { json[k_btserial_communication_settings_section][key] = v; }, value);
      }

      nlohmann::ordered_map<std::string, std::variant<float, bool>> pose_configuration = GetPoseConfigurationMap();
      for (auto& [key, value] : pose_configuration) {
        std::visit([&](auto&& v) { json[k_pose_settings_section][key] = v; }, value);
      }

      return crow::response{json.dump()};
    });

    server_ = app_.port(52060).multithreaded().run_async();
  }

  void Stop() {
    app_.stop();
  }

  ~Impl() {
    Stop();
  }

 private:
  std::future<void> server_;

  crow::SimpleApp app_;
};

DriverExternalServer::DriverExternalServer() {
  pImpl_ = std::make_unique<Impl>();
}

void DriverExternalServer::Stop() {
  pImpl_->Stop();
}

DriverExternalServer::~DriverExternalServer() = default;