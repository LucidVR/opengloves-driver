#pragma once

#include "openvr_driver.h"

class IHookReceiver;

void InjectHooks(IHookReceiver *hookReceiver, vr::IVRDriverContext *pDriverContext);
void DisableHooks();