// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "driver_external.h"

#include "crow.h"
#include "device/configuration/device_configuration.h"
#include "nlohmann/json.hpp"
#include "opengloves_interface.h"
#include "webserver_logging.h"

static og::Logger& logger = og::Logger::GetInstance();

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

      nlohmann::ordered_map<std::string, std::variant<bool, std::string>> bt_serial_configuration = GetBluetoothSerialConfigurationMap();
      for (auto& [key, value] : bt_serial_configuration) {
        std::visit([&](auto&& v) { json[k_btserial_communication_settings_section][key] = v; }, value);
      }

      nlohmann::ordered_map<std::string, std::variant<bool, std::string>> serial_configuration = GetSerialConfigurationMap();
      for (auto& [key, value] : serial_configuration) {
        std::visit([&](auto&& v) { json[k_serial_communication_settings_section][key] = v; }, value);
      }

      nlohmann::ordered_map<std::string, std::variant<bool>> namedpipe_configuration = GetNamedPipeConfigurationMap();
      for (auto& [key, value] : namedpipe_configuration) {
        std::visit([&](auto&& v) { json[k_namedpipe_communication_settings_section][key] = v; }, value);
      }

      nlohmann::ordered_map<std::string, std::variant<int>> alpha_encoding_configuration = GetAlphaEncodingConfigurationMap();
      for (auto& [key, value] : alpha_encoding_configuration) {
        std::visit([&](auto&& v) { json[k_alpha_encoding_settings_section][key] = v; }, value);
      }

      nlohmann::ordered_map<std::string, std::variant<float, bool>> pose_configuration = GetPoseConfigurationMap();
      for (auto& [key, value] : pose_configuration) {
        std::visit([&](auto&& v) { json[k_pose_settings_section][key] = v; }, value);
      }

      return crow::response{json.dump()};
    });

    CROW_ROUTE(app_, "/settings").methods("POST"_method)([&](const crow::request& req) {
      vr::CVRSettingHelper settings_helper(vr::VRSettings());

      std::vector<std::string> sections_set;

      const nlohmann::json data = nlohmann::json::parse(req.body);

      for (const auto& section : data.items()) {
        for (const auto& option : section.value().items()) {
          const std::string& key = option.key();
          if (key.find("__") != std::string::npos) continue;

          const nlohmann::json& value = option.value();

          try {
            vr::EVRSettingsError settings_error;
            switch (value.type()) {
              case nlohmann::json::value_t::string:
                settings_helper.SetString(section.key().c_str(), key.c_str(), value.get<std::string>().c_str(), &settings_error);
                break;
              case nlohmann::json::value_t::boolean:
                settings_helper.SetBool(section.key().c_str(), key.c_str(), value.get<bool>(), &settings_error);
                break;
              case nlohmann::json::value_t::number_float:
                settings_helper.SetFloat(section.key().c_str(), key.c_str(), value.get<float>(), &settings_error);
                break;
              case nlohmann::json::value_t::number_unsigned:
              case nlohmann::json::value_t::number_integer:
                vr::VRSettings()->SetInt32(section.key().c_str(), key.c_str(), value.get<int>(), &settings_error);
                break;
              default: {
                std::stringstream ss;
                ss << "Error finding configuration property value of " << section.key() << "." << key << ", value: " << value.type_name()
                   << std::endl;
                logger.Log(og::kLoggerLevel_Warning, "%s. Skipping...", ss.str().c_str());
                continue;
              }
            }

            if (settings_error != vr::VRSettingsError_None) {
              logger.Log(og::kLoggerLevel_Error, "OpenVR settings failed with code: %i", settings_error);
              return crow::response(400, "Failed to set OpenVR configuration. Failure code: " + std::to_string(settings_error));
            }

          } catch (nlohmann::json::exception& e) {
            logger.Log(og::kLoggerLevel_Error, e.what());
            return crow::response(400, e.what());
          }
        }
      }

      // return list of sections updated
      const nlohmann::json response = sections_set;
      return crow::response(200, response.dump());
    });

    CROW_ROUTE(app_, "/settings").methods("DELETE"_method)([&](const crow::request& req) {
      vr::EVRSettingsError err;
      vr::VRSettings()->RemoveSection(k_driver_settings_section, &err);
      vr::VRSettings()->RemoveSection(k_serial_communication_settings_section, &err);
      vr::VRSettings()->RemoveSection(k_btserial_communication_settings_section, &err);
      vr::VRSettings()->RemoveSection(k_pose_settings_section, &err);
      vr::VRSettings()->RemoveSection(k_alpha_encoding_settings_section, &err);

      return crow::response(200);
    });

    CROW_ROUTE(app_, "/functions/<string>/<string>")
        .methods("POST"_method)([&](const crow::request& req, const std::string& function_name, const std::string& role) {
          if (!crow::json::load(req.body)) return crow::response(crow::status::BAD_REQUEST);

          std::string function_callback_name = function_name + "/" + role;

          if (!request_callbacks_.contains(function_callback_name)) {
            return crow::response(404);
          }

          try {
            if (!request_callbacks_.at(function_callback_name)(req.body)) {
              return crow::response(400);
            }
          } catch (const std::exception& e) {
            logger.Log(og::kLoggerLevel_Error, "failed to call function %s: %s", function_name.c_str(), e.what());

            return crow::response(500, e.what());
          } catch (...) {
            logger.Log(og::kLoggerLevel_Error, "fatal error calling function: %s", function_name.c_str());
            return crow::response(500);
          }

          return crow::response(200);
        });

    server_ = app_.port(52060).multithreaded().run_async();
  }

  void RegisterFunctionCallback(const std::string& path, const std::function<bool(const std::string& body)>& callback) {
    request_callbacks_[path] = {callback};
  }

  void RemoveFunctionCallback(const std::string& path) {
    request_callbacks_.erase(path);
  }

  void Stop() {
    app_.stop();
  }

  ~Impl() {
    Stop();
  }

 private:
  crow::SimpleApp app_;
  std::future<void> server_;

  std::map<std::string, std::function<bool(const std::string& body)>> request_callbacks_;
};

DriverExternalServer::DriverExternalServer() {
  pImpl_ = std::make_unique<Impl>();
}

void DriverExternalServer::RegisterFunctionCallback(const std::string& path, const std::function<bool(const std::string& body)>& callback) {
  pImpl_->RegisterFunctionCallback(path, callback);
}

void DriverExternalServer::RemoveFunctionCallback(const std::string& path) {
  pImpl_->RemoveFunctionCallback(path);
}

void DriverExternalServer::Stop() {
  pImpl_->Stop();
}

DriverExternalServer::~DriverExternalServer() = default;