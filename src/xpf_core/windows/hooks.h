/*
  NoirVisor - Hardware-Accelerated Hypervisor solution

  Copyright 2018-2019, Zero Tang. All rights reserved.

  This file is auxiliary to MSR-Hook facility to optimize compatibility.

  This program is distributed in the hope that it will be useful, but 
  without any warranty (no matter implied warranty or merchantability
  or fitness for a particular purpose, etc.).

  File Location: ./xpf_core/windows/hooks.h
*/

#include <ntddk.h>
#include <windef.h>

#if defined(_WIN64)
#define INDEX_OFFSET		0x15
#else
#define INDEX_OFFSET		0x1
#endif

#define NoirProtectedFileName	L"NoirVisor.sys"
#define NoirProtectedFileNameCch	13
#define NoirProtectedFileNameCb		NoirProtectedFileNameCch*2

#if defined(_WIN64)
#define NoirGetPageBase(va)		(PVOID)((ULONG64)va&0xfffffffffffff000)
#define HookLength				16
#define DetourLength			14
#else
#define NoirGetPageBase(va)		(PVOID)((ULONG)va&0xfffff000)
#define HookLength				5
#define DetourLength			5
#endif

typedef NTSTATUS (*NTSETINFORMATIONFILE)
(
 IN HANDLE FileHandle,
 OUT PIO_STATUS_BLOCK IoStatusBlock,
 IN PVOID FileInformation,
 IN ULONG Length,
 IN FILE_INFORMATION_CLASS FileInformationClass
);

typedef struct _MEMORY_DESCRIPTOR
{
	PVOID VirtualAddress;
	ULONG64 PhysicalAddress;
}MEMORY_DESCRIPTOR,*PMEMORY_DESCRIPTOR;

typedef struct _NOIR_HOOK_PAGE
{
	MEMORY_DESCRIPTOR OriginalPage;
	MEMORY_DESCRIPTOR HookedPage;
	PVOID Reserved;
	struct _NOIR_HOOK_PAGE* NextHook;
}NOIR_HOOK_PAGE,*PNOIR_HOOK_PAGE;

typedef struct _NOIR_PROTECTED_FILE_NAME
{
	ERESOURCE Lock;
	SIZE_T Length;
	SIZE_T MaximumLength;
	WCHAR FileName[1];
}NOIR_PROTECTED_FILE_NAME,*PNOIR_PROTECTED_FILE_NAME;

PVOID NoirAllocateContiguousMemory(IN ULONG Length);
ULONG64 NoirGetPhysicalAddress(IN PVOID VirtualAddress);
ULONG SizeOfCode(IN PVOID Code,IN ULONG Architecture);
ULONG GetPatchSize(IN PVOID Code,IN ULONG Length);

NTSETINFORMATIONFILE	NtSetInformationFile=NULL,Old_NtSetInformationFile=NULL;
PNOIR_PROTECTED_FILE_NAME NoirProtectedFile=NULL;
PNOIR_HOOK_PAGE noir_hook_pages=NULL;
ULONG IndexOf_NtOpenProcess=0x23;		//This is hard-code on Windows 7 x64.
ULONG ProtPID=0;
extern ULONG_PTR orig_system_call;

#define HookPages	noir_hook_pages