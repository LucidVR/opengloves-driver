#pragma once
#include "openvr_driver.h"

class IHookReceiver {
 public:
  virtual void TrackedDeviceAdded(const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, vr::ITrackedDeviceServerDriver *pDriver){};

  virtual void CreateSkeletonComponent(
      vr::PropertyContainerHandle_t ulContainer,
      const char *pchName,
      const char *pchSkeletonPath,
      const char *pchBasePosePath,
      vr::EVRSkeletalTrackingLevel eSkeletalTrackingLevel,
      const vr::VRBoneTransform_t *pGripLimitTransforms,
      uint32_t unGripLimitTransformCount,
      vr::VRInputComponentHandle_t *pHandle){};

  virtual void UpdateSkeletonComponent(
      vr::VRInputComponentHandle_t ulComponent, vr::EVRSkeletalMotionRange eMotionRange, const vr::VRBoneTransform_t *pTransforms, uint32_t unTransformCount){};

  virtual void CreateBooleanComponent(vr::PropertyContainerHandle_t ulContainer, const char *pchName, vr::VRInputComponentHandle_t *pHandle){};
  virtual void UpdateBooleanComponent(vr::VRInputComponentHandle_t ulComponent, bool bNewValue, double fTimeOffset){};
};