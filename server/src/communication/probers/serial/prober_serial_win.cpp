#include "communication/probers/serial/prober_serial_win.h"

#include "communication/services/serial/service_serial.h"
#include "opengloves_interface.h"

// must be in this order

// clang-format off
#include <Windows.h>
#include <SetupAPI.h>
// clang-format on

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace og;

static Logger& logger = Logger::GetInstance();

static std::string GetLastErrorAsString() {
  const DWORD errorMessageId = ::WSAGetLastError();
  if (errorMessageId == 0) return std::string();

  LPSTR messageBuffer = nullptr;
  const size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      errorMessageId,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&messageBuffer),
      0,
      nullptr);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

SerialCommunicationProber::SerialCommunicationProber(SerialProberConfiguration configuration) : configuration_(std::move(configuration)) {
  // convert serial pid vid struct to a string we can easily search for
  for (const auto& identifier : configuration_.identifiers) {
    std::string formatted_param = "VID_" + identifier.vid + "&PID_" + identifier.pid;
    identifiers_.emplace_back(formatted_param);
  }
}

og::CommunicationType SerialCommunicationProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) {
  DWORD dwSize = 0;
  DWORD Error = 0;

  HDEVINFO device_info_set = SetupDiGetClassDevs(NULL, "USB", NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
  if (device_info_set == INVALID_HANDLE_VALUE) {
    logger.Log(og::kLoggerLevel_Error, "Failed to set up serial prober: %s", GetLastErrorAsString().c_str());

    return og::kCommunicationType_Invalid;
  }

  SP_DEVINFO_DATA device_info_data;
  ZeroMemory(&device_info_data, sizeof(SP_DEVINFO_DATA));
  device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

  DWORD device_index = 0;
  int found_devices = 0;
  while (SetupDiEnumDeviceInfo(device_info_set, device_index, &device_info_data)) {
    device_index++;
    DEVPROPTYPE property_type;
    char property_buff[1024] = {0};
    if (SetupDiGetDeviceRegistryProperty(
            device_info_set, &device_info_data, SPDRP_HARDWAREID, &property_type, (BYTE*)property_buff, sizeof(property_buff), &dwSize)) {
      // extract pid and vid from the property and check if it's an available device

      std::string strproperty_buff = property_buff;

      if (std::find_if(
              identifiers_.begin(),  //
              identifiers_.end(),    //
              [&](const std::string& param) { return strproperty_buff.find(param) != std::string::npos; }) == identifiers_.end())
        continue;

      HKEY device_registery_key = SetupDiOpenDevRegKey(device_info_set, &device_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
      if (device_registery_key == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        break;
      }

      char port_name[20];
      DWORD port_name_size = sizeof(port_name);
      DWORD type = 0;

      if ((RegQueryValueEx(device_registery_key, "PortName", NULL, &type, (LPBYTE)port_name, &port_name_size) == ERROR_SUCCESS) && (type == REG_SZ)) {
        std::string port = std::string(R"(\\.\)") + port_name;

        og::DeviceSerialCommunicationConfiguration serial_configuration{port};
        std::unique_ptr<ICommunicationService> communication_service = std::make_unique<SerialCommunicationService>(serial_configuration);

        // this means that the device is probably opened by us (or someone else). Ignore it.
        if (!communication_service->IsConnected()) {
          logger.Log(
              kLoggerLevel_Warning,
              "A device was discovered on port: %s, but did not connect as it is already being used by another process (or is unreachable).",
              port.c_str());

          continue;
        }

        out_devices.emplace_back(std::move(communication_service));
        found_devices++;

        RegCloseKey(device_registery_key);
      }
    }
  }

  if (device_info_set) {
    SetupDiDestroyDeviceInfoList(device_info_set);
  }

  return og::CommunicationType::kCommunicationType_Serial;
}

std::string SerialCommunicationProber::GetName() {
  return "serial";
}