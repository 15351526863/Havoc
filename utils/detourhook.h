#pragma once
// used: format
#include <format>

// used: minhook hooking library
#include "../dependencies/minhook/minhook.h"

/*
 * detour hooking implementation using minhook
 * @credits: https://github.com/TsudaKageyu/minhook
 */
class CDetourHook
{
public:
	CDetourHook() = default;

	explicit CDetourHook(void* pFunction, void* pDetour)
		: pBaseFn(pFunction), pReplaceFn(pDetour) { }

	/* first hook setup */
	bool Create(void* pFunction, void* pDetour)
	{
		pBaseFn = pFunction;

		if (pBaseFn == nullptr)
			return false;

		pReplaceFn = pDetour;

		if (pReplaceFn == nullptr)
			return false;

		const MH_STATUS status = MH_CreateHook(pBaseFn, pReplaceFn, &pOriginalFn);

		if (status != MH_OK) {
			printf("failed to create hook function\n");
		}
		else {
			printf("creating hook status is ok, base: %i, replace: %i, original: %i\n", pBaseFn, pReplaceFn, pOriginalFn);
		}
			//throw std::runtime_error(std::vformat(_("failed to create hook function, status: {}\nbase function -> {:#08X}"), std::make_format_args(MH_StatusToString(status), reinterpret_cast<std::uintptr_t>(pBaseFn))));

		if (!this->Replace())
			return false;

		return true;
	}

	/* replace function to our modified */
	bool Replace()
	{
		// check is hook be created
		if (pBaseFn == nullptr)
			return false;

		// check is function not already hooked
		if (bIsHooked)
			return false;

		const MH_STATUS status = MH_EnableHook(pBaseFn);

		if (status != MH_OK)
		{
			printf("failed to enable hook function\n");
		}
			//throw std::runtime_error(std::vformat(_("failed to enable hook function, status: {}\nbase function -> {:#08X} address"), std::make_format_args(MH_StatusToString(status), reinterpret_cast<std::uintptr_t>(pBaseFn))));

		// switch hook state
		bIsHooked = true;
		return true;
	}

	/* remove hook fully */
	bool Remove()
	{
		// restore it at first
		if (!this->Restore())
			return false;

		const MH_STATUS status = MH_RemoveHook(pBaseFn);

		if (status != MH_OK) {
			printf("failed to remove hook\n");
		}
		// throw std::runtime_error(std::vformat(_("failed to remove hook, status: {}\n base function -> {:#08X} address"), std::make_format_args(MH_StatusToString(status), reinterpret_cast<std::uintptr_t>(pBaseFn))));

		return true;
	}

	/* replace swaped modified function back to original */
	bool Restore()
	{
		// check is function hooked
		if (!bIsHooked)
			return false;

		const MH_STATUS status = MH_DisableHook(pBaseFn);

		// 
		if (status != MH_OK) {
			printf("failed to restore hook\n");
		}
		// throw std::runtime_error(std::vformat(_("failed to restore hook, status: {}\n base function -> {:#08X} address"), std::make_format_args(MH_StatusToString(status), reinterpret_cast<std::uintptr_t>(pBaseFn))));

		// switch hook state
		bIsHooked = false;
		return true;
	}

	/* get original function pointer (not a call!) */
	template <typename Fn>
	Fn GetOriginal()
	{
		return static_cast<Fn>(pOriginalFn);
	}

	/* returns hook state */
	inline bool IsHooked() const
	{
		return bIsHooked;
	}

private:
	// Values
	/* hook state */
	bool bIsHooked = false;
	/* function base address */
	void* pBaseFn = nullptr;
	/* in future that is being modified and replace original */
	void* pReplaceFn = nullptr;
	/* saved function to get available restore hook later */
	void* pOriginalFn = nullptr;
};
