#include "service_bluetooth_win.h"

#include <Windows.h>
#include <ws2bth.h>

BluetoothCommunicationService::BluetoothCommunicationService(BTH_ADDR bt_address) {
  bt_address_ = bt_address;
}