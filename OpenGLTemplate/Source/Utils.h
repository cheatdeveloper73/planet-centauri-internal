#pragma once
#include <Windows.h>
#include <vector>

#include "fnv.h"

HWND tmp_window = NULL;
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE; // skip to next window

	tmp_window = handle;
	return FALSE; // window found abort search
}

HWND GetProcessWindow()
{
	tmp_window = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return tmp_window;
}

__forceinline auto pattern_to_byte(const char* pattern)
{

	auto bytes = std::vector<int>{};
	auto start = const_cast<char*>(pattern);
	auto end = const_cast<char*>(pattern) + strlen(pattern);

	for (auto current = start; current < end; ++current)
	{

		if (*current == xi(63))
		{
			++current;

			if (*current == xi(63))
				++current;

			bytes.push_back(xi(-1));
		}
		else
			bytes.push_back(strtoul(current, &current, xi(16)));

	}

	return bytes;

}

__forceinline uintptr_t find_signature(uintptr_t module_base, const char* signature, uintptr_t section)
{

	auto dos_headers = reinterpret_cast<IMAGE_DOS_HEADER*>(module_base);
	auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(module_base + dos_headers->e_lfanew);

	auto section_header = IMAGE_FIRST_SECTION(nt_headers);
	uintptr_t section_base = xi(0);
	DWORD section_size = xi(0);

	for (int i = xi(0); i < nt_headers->FileHeader.NumberOfSections; i++, section_header++)
	{

		if (RUNTIME_HASH(reinterpret_cast<const char*>(section_header->Name)) == section)
		{

			section_base = module_base + section_header->VirtualAddress;
			section_size = section_header->Misc.VirtualSize;
			break;

		}

	}

	if (!section_base || !section_size)
		return xi(NULL);

	auto buffer = new BYTE[section_size];
	std::memcpy(buffer, reinterpret_cast<void*>(section_base), section_size);

	auto pattern_bytes = pattern_to_byte(signature);
	auto pattern_length = pattern_bytes.size();
	auto pattern_data = pattern_bytes.data();

	for (DWORD64 i = xi(0); i < section_size - pattern_length; i++)
	{

		auto found = xi(1);

		for (DWORD64 j = xi(0); j < pattern_length; j++)
		{

			if (pattern_data[j] != xi(-1) && pattern_data[j] != buffer[i + j])
			{
				found = xi(0);
				break;
			}

		}

		if (found)
			return section_base + i;

	}

	delete[] buffer;

	return xi(NULL);

}