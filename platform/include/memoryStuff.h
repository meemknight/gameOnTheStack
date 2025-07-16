#pragma once
#include <Windows.h>
#include <iostream>
#include <intrin.h>

inline size_t constexpr KB(size_t x) { return x * 1024ull; }
inline size_t constexpr MB(size_t x) { return KB(x) * 1024ull; }
inline size_t constexpr GB(size_t x) { return MB(x) * 1024ull; }
inline size_t constexpr TB(size_t x) { return GB(x) * 1024ull; }

inline float constexpr BYTES_TO_KB(size_t x) { return x / 1024.f; }
inline float constexpr BYTES_TO_MB(size_t x) { return BYTES_TO_KB(x) / 1024.f; }
inline float constexpr BYTES_TO_GB(size_t x) { return BYTES_TO_MB(x) / 1024.f; }

inline void printStackUsage()
{
	// This works on x64 (GS segment) and x86 (FS segment)
#ifdef _M_X64
	NT_TIB *tib = (NT_TIB *)NtCurrentTeb();
#else
	NT_TIB *tib = (NT_TIB *)__readfsdword(0x18);
#endif

	void *stackBase = tib->StackBase;
	void *stackLimit = tib->StackLimit;

	// Get current stack pointer
	void *currentStackPtr = _AddressOfReturnAddress();

	SIZE_T used = (SIZE_T)stackBase - (SIZE_T)currentStackPtr;
	SIZE_T total = (SIZE_T)stackBase - (SIZE_T)stackLimit;

	//std::cout << "Stack used:  " << used << " bytes\n";
	//std::cout << "Stack total: " << total << " bytes\n\n";

	std::cout << "Stack used:  " << BYTES_TO_MB(used) * 100 << "%\n";
	//std::cout << "Stack total: " << BYTES_TO_MB(total) << " MB\n\n";


}

inline void forceStackCommit()
{
	const SIZE_T pageSize = 0x1000; // 4 KB pages
#ifdef _M_X64
	NT_TIB *tib = (NT_TIB *)NtCurrentTeb();
#else
	NT_TIB *tib = (NT_TIB *)__readfsdword(0x18);
#endif

	char *stackLimit = (char *)tib->StackLimit;
	char *stackBase = (char *)tib->StackBase;

	// Stack grows down, so walk from base downward
	for (char *p = stackBase - pageSize; p >= stackLimit; p -= pageSize)
	{
		volatile char touch = *p;
		(void)touch; // prevent optimization
	}
}


inline uint16_t packRGB565(unsigned char r, unsigned char g, unsigned char b)
{
	return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

#define SCREEN_BUFFER_MEMORY KB(290)


