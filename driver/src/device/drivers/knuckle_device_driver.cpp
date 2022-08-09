#include "knuckle_device_driver.h"

KnuckleDeviceDriver::KnuckleDeviceDriver(const std::string& serial_number) {
  serial_number_ = serial_number;
}

void KnuckleDeviceDriver::SetupProperties(vr::PropertyContainerHandle_t& props) {
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, 2000000000);
  vr::VRProperties()->SetInt32Property(props, )
}

void KnuckleDeviceDriver::SetupComponents(vr::PropertyContainerHandle_t& props) {}

void KnuckleDeviceDriver::HandleInput(const og::InputPeripheralData &data) {

}