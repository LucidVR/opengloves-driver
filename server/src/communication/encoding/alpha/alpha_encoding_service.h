#pragma once

#include <map>

#include "encoding/encoding_service.h"
#include "opengloves_interface.h"

enum AlphaEncodingKey {
  kAlphaEncodingKey_ThumbCurl,
  kAlphaEncodingKey_ThumbSplay,

  kAlphaEncodingKey_IndexCurl,
  kAlphaEncodingKey_IndexSplay,

  kAlphaEncodingKey_MiddleCurl,
  kAlphaEncodingKey_MiddleSplay,

  kAlphaEncodingKey_RingCurl,
  kAlphaEncodingKey_RingSplay,

  kAlphaEncodingKey_PinkyCurl,
  kAlphaEncodingKey_PinkySplay,

  kAlphaEncodingKey_ThumbJoint0,
  kAlphaEncodingKey_ThumbJoint1,
  kAlphaEncodingKey_ThumbJoint2,
  kAlphaEncodingKey_ThumbJoint3,

  kAlphaEncodingKey_IndexJoint0,
  kAlphaEncodingKey_IndexJoint1,
  kAlphaEncodingKey_IndexJoint2,
  kAlphaEncodingKey_IndexJoint3,

  kAlphaEncodingKey_MiddleJoint0,
  kAlphaEncodingKey_MiddleJoint1,
  kAlphaEncodingKey_MiddleJoint2,
  kAlphaEncodingKey_MiddleJoint3,

  kAlphaEncodingKey_RingJoint0,
  kAlphaEncodingKey_RingJoint1,
  kAlphaEncodingKey_RingJoint2,
  kAlphaEncodingKey_RingJoint3,

  kAlphaEncodingKey_PinkyJoint0,
  kAlphaEncodingKey_PinkyJoint1,
  kAlphaEncodingKey_PinkyJoint2,
  kAlphaEncodingKey_PinkyJoint3,

  kAlphaEncodingKey_MainJoystick_X,
  kAlphaEncodingKey_MainJoystick_Y,
  kAlphaEncodingKey_MainJoystick_Click,

  kAlphaEncodingKey_Trigger_Value,
  kAlphaEncodingKey_Trigger_Click,

  kAlphaEncodingKey_A_Click,
  kAlphaEncodingKey_B_Click,

  kAlphaEncodingKey_Grab_Gesture,
  kAlphaEncodingKey_Pinch_Gesture,

  kAlphaEncodingKey_Menu_Click,
  kAlphaEncodingKey_Calibration_Click,

  kAlphaEncodingKey_Info,
  kAlphaEncodingKey_Info_FWVersion,
  kAlphaEncodingKey_Info_DeviceType,
  kAlphaEncodingKey_Info_Hand,

  kAlphaEncodingKey_Info_StartStreaming,
  kAlphaEncodingKey_Info_StopStreaming,

  kAlphaEncodingKey_OutHapticFrequency,
  kAlphaEncodingKey_OutHapticDuration,
  kAlphaEncodingKey_OutHapticAmplitude,

  kAlphaEncodingKey_Max
};

class AlphaEncodingService : public IEncodingService {
 public:
  AlphaEncodingService(const og::EncodingConfiguration& encoding_configuration);

  og::Input DecodePacket(const std::string& buff) override;
  std::string EncodePacket(const og::Output& output) override;

 private:
  og::InputPeripheralData DecodePeripheralPacket(const std::map<AlphaEncodingKey, std::string>& input_map);
  og::InputInfoData DecodeInfoPacket(const std::map<AlphaEncodingKey, std::string>& input_map);

  og::EncodingConfiguration configuration_;
};