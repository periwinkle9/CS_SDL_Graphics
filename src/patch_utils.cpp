#include "patch_utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace patcher
{

bool verifyBytes(dword address, const byte expectedBytes[], unsigned size)
{
	// Let's assume we have read permissions because I don't want to deal with the case where that's not true
	// (and that case is probably not happening anyways)
	const byte* memPtr = reinterpret_cast<const byte*>(address);
	for (unsigned i = 0; i < size; ++i)
		if (memPtr[i] != expectedBytes[i])
			return false;
	return true;
}

void patchBytes(dword address, const byte bytes[], unsigned size)
{
	void* addr = reinterpret_cast<void*>(address);
	WriteProcessMemory(GetCurrentProcess(), addr, static_cast<const void*>(bytes), size, NULL);
}

void writeNOPs(dword address, unsigned numBytes)
{
	// I could do this more simply with VirtualProtect + FillMemory, but let's not bother with that
	const byte NOPs[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
	
	for (unsigned size = sizeof NOPs; size > 0; size /= 2)
	{
		while (numBytes >= size)
		{
			patchBytes(address, NOPs, size);
			numBytes -= size;
			address += size;
		}
	}
}

}
