#include "Hooks/Hooking.h"

std::map<std::string, IHook *> IHook::hooks;

bool IHook::Exists(const std::string &name)
{
	return hooks.find(name) != hooks.end();
}

void IHook::Register(IHook *hook)
{
	hooks[hook->name] = hook;

	DriverLog("Registed: %s", hook->name.c_str());
}

void IHook::Unregister(IHook *hook)
{
	hooks.erase(hook->name);
}

void IHook::DestroyAll()
{
	for (auto &hook : hooks)
	{
		hook.second->Destroy();
	}
	hooks.clear();
}
