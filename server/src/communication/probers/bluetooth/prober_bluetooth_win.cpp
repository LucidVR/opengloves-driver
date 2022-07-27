
// clang-format off
#include <WinSock2.h>
#include <bluetoothapis.h>

#include <Windows.h>
#include <ws2bth.h>
// clang-format on

#include "prober_bluetooth_win.h"

#include <string>
#include <vector>

#include "opengloves_interface.h"
#include "services/bluetooth/service_bluetooth.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

BluetoothCommunicationProber::BluetoothCommunicationProber(std::vector<std::string> wanted_devices) {
  wanted_devices_ = wanted_devices;
}

static bool BluetoothDeviceIsConnectable(const BTH_ADDR& bt_address) {
  SOCKET sock = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

  SOCKADDR_BTH sock_address = {.addressFamily = AF_BTH, .btAddr = bt_address, .serviceClassId = RFCOMM_PROTOCOL_UUID, .port = 0};

  if (connect(sock, reinterpret_cast<const SOCKADDR*>(&sock_address), sizeof sock_address) != 0) {
    return false;
  }

  if (shutdown(sock, SD_BOTH) == SOCKET_ERROR) {
    logger.Log(og::kLoggerLevel_Error, "Bluetooth prober failed to disconnect from device when trying to discover it! %llu", bt_address);
  }

  return true;
}

int BluetoothCommunicationProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) {
  constexpr BLUETOOTH_DEVICE_SEARCH_PARAMS bt_dev_sp = {
      sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),  // size of object
      1,                                       // return authenticated devices
      0,                                       // return remembered devices
      1,                                       // return unknown devices
      1,                                       // return connected devices
      1,                                       // issue in query
      2,                                       // timeout multiplier. Multiply this value by 1.28 seconds to get timeout.
      nullptr                                  // radio handler
  };

  BLUETOOTH_DEVICE_INFO bt_dev_info = {sizeof(BLUETOOTH_DEVICE_INFO), {0}};                  // default
  const HBLUETOOTH_DEVICE_FIND bt_dev = BluetoothFindFirstDevice(&bt_dev_sp, &bt_dev_info);  // returns first BT device connected to this machine

  if (bt_dev == nullptr) {
    logger.Log(kLoggerLevel_Info, "Could not find any bluetooth devices");
    return 0;
  }

  int dev_found = 0;

  do {
    for (const auto& find_device_name : wanted_devices_) {
      const auto wfind_device_name = std::wstring(find_device_name.begin(), find_device_name.end());
      const WCHAR* wcharfind_device_name = const_cast<WCHAR*>(wfind_device_name.c_str());

      if (wcscmp(bt_dev_info.szName, wcharfind_device_name) == 0) {
        if (bt_dev_info.fAuthenticated) {
          BTH_ADDR dev_bt_address = bt_dev_info.Address.ullLong;

          if (BluetoothDeviceIsConnectable(dev_bt_address)) {
            logger.Log(kLoggerLevel_Info, "Discovered a new device: %s", find_device_name.c_str());
            out_devices.emplace_back(std::make_unique<BluetoothCommunicationService>(dev_bt_address));

            dev_found++;
          }
        } else {
          // todo: maybe we can automatically try to pair with them?
          logger.Log(kLoggerLevel_Warning, "A device was found but was not authenticated. Please pair with it first.");
        }
      }
    }

  } while (BluetoothFindNextDevice(bt_dev, &bt_dev_info));

  return dev_found;
}