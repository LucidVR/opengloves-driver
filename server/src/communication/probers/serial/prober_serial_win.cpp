#include "serial/prober_serial_win.h"

#include "services/serial/service_serial.h"

// must be in this order

// clang-format off
#include <Windows.h>
#include <SetupAPI.h>
// clang-format on

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

SerialCommunicationProber::SerialCommunicationProber(const std::vector<SerialProberSearchParam>& params) {
  params_ = params;
}

// tries to connect to a serial port, if the serial port is open then someone (probably us) is already connected to it. Prevents us rediscovering a
// previously discovered device
static bool SerialDeviceIsConnectable(const std::string& port_name) {
  HANDLE handle = CreateFile(port_name.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (handle == INVALID_HANDLE_VALUE) {
    // handle is not connectable
    return false;
  }

  // we connected to the handle. close so we can connect again properly.
  CloseHandle(handle);

  return true;
}

int SerialCommunicationProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) {
  DWORD dwSize = 0;
  DWORD Error = 0;

  HDEVINFO device_info_set = SetupDiGetClassDevs(NULL, "USB", NULL, DIGCF_ALLCLASSES | DIGCF_PRESENT);
  if (device_info_set == INVALID_HANDLE_VALUE) return -GetLastError();

  SP_DEVINFO_DATA device_info_data;
  ZeroMemory(&device_info_data, sizeof(SP_DEVINFO_DATA));
  device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

  DWORD device_index = 0;
  int found_devices = 0;
  while (SetupDiEnumDeviceInfo(device_info_set, device_index, &device_info_data)) {
    DEVPROPTYPE property_type;
    char property_buff[1024] = {0};
    if (SetupDiGetDeviceRegistryProperty(
            device_info_set, &device_info_data, SPDRP_HARDWAREID, &property_type, (BYTE*)property_buff, sizeof(property_buff), &dwSize)) {
      // extract pid and vid from the property and check if it's an available device
      std::stringstream fmt;
      SerialProberSearchParam search;
      fmt << "VID_" << search.vid << ".PID_" << search.pid;
      if (!(std::find(params_.begin(), params_.end(), search) != params_.end())) continue;

      HKEY device_registery_key = SetupDiOpenDevRegKey(device_info_set, &device_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
      if (device_registery_key == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
        break;
      }

      char port_name[20];
      DWORD port_name_size = sizeof(port_name);
      DWORD type = 0;

      if ((RegQueryValueEx(device_registery_key, "PortName", NULL, &type, (LPBYTE)port_name, &port_name_size) == ERROR_SUCCESS) && (type == REG_SZ)) {
        if (SerialDeviceIsConnectable(port_name)) {
          found_devices++;

          out_devices.emplace_back(std::make_unique<SerialCommunicationService>(port_name));
        }

        RegCloseKey(device_registery_key);
      }
    }
    device_index++;
  }

  if (device_info_set) {
    SetupDiDestroyDeviceInfoList(device_info_set);
  }

  if (Error) return -Error;

  return found_devices;
}

std::string SerialCommunicationProber::GetName() {
  return "serial";
}