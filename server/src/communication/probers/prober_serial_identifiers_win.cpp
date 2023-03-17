// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "prober_serial_identifiers_win.h"

#include "communication/services/service_serial.h"
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

#include "prober_serial_connectable.h"
#include "win/win_util.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

SerialIdentifierProber::SerialIdentifierProber(SerialProberIdentifier identifier) {
  // convert serial pid vid struct to a string we can easily search for
  identifier_ = "VID_" + identifier.vid + "&PID_" + identifier.pid;
}

bool SerialIdentifierProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) {
  DWORD dwSize = 0;
  DWORD Error = 0;

  HDEVINFO device_info_set = SetupDiGetClassDevs(NULL, "USB", NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
  if (device_info_set == INVALID_HANDLE_VALUE) {
    logger.Log(og::kLoggerLevel_Error, "Failed to set up serial prober: %s", GetLastErrorAsString().c_str());

    return false;
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

      // if we can't find a matching identifier, skip this device
      if (strproperty_buff.find(identifier_) == std::string::npos) continue;

      HKEY device_registery_key = SetupDiOpenDevRegKey(device_info_set, &device_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
      if (device_registery_key == INVALID_HANDLE_VALUE) {
        logger.Log(og::kLoggerLevel_Error, "Failed to open device registry key: %s", GetLastErrorAsString().c_str());
        break;
      }

      char port_name[20];
      DWORD port_name_size = sizeof(port_name);
      DWORD type = 0;

      if ((RegQueryValueEx(device_registery_key, "PortName", nullptr, &type, (LPBYTE)port_name, &port_name_size) == ERROR_SUCCESS) &&
          (type == REG_SZ)) {
        std::string port = std::string(R"(\\.\)") + port_name;

        SerialPortProber port_prober({port});
        port_prober.InquireDevices(out_devices);

        found_devices++;
        RegCloseKey(device_registery_key);
      }
    }
  }

  if (device_info_set) {
    SetupDiDestroyDeviceInfoList(device_info_set);
  }

  return found_devices > 0;
}