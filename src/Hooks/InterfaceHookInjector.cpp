#include "Hooks/InterfaceHookInjector.h"

#include "DriverLog.h"
#include "Hooks/HookReceiver.h"
#include "Hooks/Hooking.h"

static IHookReceiver *HookReceiver = nullptr;

static Hook<void *(*)(vr::IVRDriverContext *, const char *, vr::EVRInitError *)> GetGenericInterfaceHook("IVRDriverContext::GetGenericInterface");

static Hook<bool (*)(
    vr::IVRServerDriverHost *, const char *pchDeviceSerialNumber, vr::ETrackedDeviceClass eDeviceClass, vr::ITrackedDeviceServerDriver *pDriver)>
    TrackedDeviceAddedHook006("IVRServerDriverHost006::TrackedDeviceAdded");

static Hook<vr::EVRInputError (*)(
    vr::IVRDriverInput *,
    vr::PropertyContainerHandle_t,
    const char *,
    const char *,
    const char *,
    vr::EVRSkeletalTrackingLevel,
    const vr::VRBoneTransform_t *,
    uint32_t,
    vr::VRInputComponentHandle_t *)>
    CreateSkeletonComponentHook003("IVRDriverInput003::CreateSkeletonComponent");

static Hook<vr::EVRInputError (*)(
    vr::IVRDriverInput *,
    vr::VRInputComponentHandle_t,
    vr::EVRSkeletalMotionRange,
    const vr::VRBoneTransform_t *,
    uint32_t unTransformCount)>
    UpdateSkeletonComponentHook003("IVRDriverInput003::UpdateSkeletonComponent");

static bool DetourTrackedDeviceAdded006(
    vr::IVRServerDriverHost *_this,
    const char *pchDeviceSerialNumber,
    vr::ETrackedDeviceClass eDeviceClass,
    vr::ITrackedDeviceServerDriver *pDriver) {
  DriverLog("Hook: TrackedDeviceAdded called, Serial: %s", pchDeviceSerialNumber);

  HookReceiver->TrackedDeviceAdded(pchDeviceSerialNumber, eDeviceClass, pDriver);

  auto retval = TrackedDeviceAddedHook006.originalFunc(_this, pchDeviceSerialNumber, eDeviceClass, pDriver);
  return retval;
}

static vr::EVRInputError DetourCreateSkeletonComponent(
    vr::IVRDriverInput *_this,
    vr::PropertyContainerHandle_t ulContainer,
    const char *pchName,
    const char *pchSkeletonPath,
    const char *pchBasePosePath,
    vr::EVRSkeletalTrackingLevel eSkeletalTrackingLevel,
    const vr::VRBoneTransform_t *pGripLimitTransforms,
    uint32_t unGripLimitTransformCount,
    vr::VRInputComponentHandle_t *pHandle) {
  auto retval = CreateSkeletonComponentHook003.originalFunc(
      _this,
      ulContainer,
      pchName,
      pchSkeletonPath,
      pchBasePosePath,
      eSkeletalTrackingLevel,
      pGripLimitTransforms,
      unGripLimitTransformCount,
      pHandle);

  HookReceiver->CreateSkeletonComponent(
      ulContainer, pchName, pchSkeletonPath, pchBasePosePath, eSkeletalTrackingLevel, pGripLimitTransforms, unGripLimitTransformCount, pHandle);

  return retval;
}

static vr::EVRInputError DetourUpdateSkeletonComponent(
    vr::IVRDriverInput *_this,
    vr::VRInputComponentHandle_t ulComponent,
    vr::EVRSkeletalMotionRange eMotionRange,
    const vr::VRBoneTransform_t *pTransforms,
    uint32_t unTransformCount) {
  HookReceiver->UpdateSkeletonComponent(ulComponent, eMotionRange, pTransforms, unTransformCount);

  auto retval = UpdateSkeletonComponentHook003.originalFunc(_this, ulComponent, eMotionRange, pTransforms, unTransformCount);

  return retval;
}

static void *DetourGetGenericInterface(vr::IVRDriverContext *_this, const char *pchInterfaceVersion, vr::EVRInitError *peError) {
  // DebugDriverLog("ServerTrackedDeviceProvider::DetourGetGenericInterface(%s)", pchInterfaceVersion);
  auto originalInterface = GetGenericInterfaceHook.originalFunc(_this, pchInterfaceVersion, peError);

  std::string iface(pchInterfaceVersion);
  if (iface == "IVRDriverInput_003") {
    if (!IHook::Exists(CreateSkeletonComponentHook003.name)) {
      CreateSkeletonComponentHook003.CreateHookInObjectVTable(originalInterface, 0, &DetourCreateSkeletonComponent);
      IHook::Register(&CreateSkeletonComponentHook003);
    }
    if (!IHook::Exists(UpdateSkeletonComponentHook003.name)) {
      UpdateSkeletonComponentHook003.CreateHookInObjectVTable(originalInterface, 1, &DetourUpdateSkeletonComponent);
      IHook::Register(&UpdateSkeletonComponentHook003);
    }
  } else if (iface == "IVRServerDriverHost_006") {
    if (!IHook::Exists(TrackedDeviceAddedHook006.name)) {
      TrackedDeviceAddedHook006.CreateHookInObjectVTable(originalInterface, 0, &DetourTrackedDeviceAdded006);
      IHook::Register(&TrackedDeviceAddedHook006);
    }
  }

  return originalInterface;
}

void InjectHooks(IHookReceiver *hookReceiver, vr::IVRDriverContext *pDriverContext) {
  HookReceiver = hookReceiver;

  auto err = MH_Initialize();

  if (err == MH_OK) {
    GetGenericInterfaceHook.CreateHookInObjectVTable(pDriverContext, 0, &DetourGetGenericInterface);
    IHook::Register(&GetGenericInterfaceHook);
  } else {
    DriverLog("MH_Initialize error: %s", MH_StatusToString(err));
  }
}

void DisableHooks() {
  IHook::DestroyAll();
  MH_Uninitialize();
}