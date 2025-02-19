/*
  NoirVisor - Hardware-Accelerated Hypervisor solution

  Copyright 2018-2019, Zero Tang. All rights reserved.

  This file is the central HyperVisor of NoirVisor.

  This program is distributed in the hope that it will be useful, but 
  without any warranty (no matter implied warranty or merchantability
  or fitness for a particular purpose, etc.).

  File Location: /include/noirhvm.h
*/

#include "nvdef.h"
#if defined(_vt_drv) || defined(_vt_exit) || defined(_vt_ept) || defined(_vt_nvcpu)
#include "vt_hvm.h"
#elif defined(_svm_drv) || defined(_svm_exit)
#include "svm_hvm.h"
#endif

//Known Processor Manufacturer Encoding
#define intel_processor			0
#define amd_processor			1
#define unknown_processor		0xff

//Processor State of Virtualization
#define noir_virt_off			0
#define noir_virt_on			1
#define noir_virt_trans			2
#define noir_virt_nesting		3

//Stack Size = 64KB
#define nvc_stack_size			0x10000

typedef struct _noir_hypervisor
{
#if defined(_vt_drv) || defined(_vt_exit) || defined(_vt_ept)
	noir_vt_vcpu_p virtual_cpu;
	noir_vt_hvm_p relative_hvm;
#elif defined(_svm_drv) || defined(_svm_exit) || defined(_svm_npt)
	noir_svm_vcpu_p virtual_cpu;
	noir_svm_hvm_p relative_hvm;
#else
	void* virtual_cpu;
	void* relative_hvm;
#endif
	struct
	{
		ulong_ptr base;
		u32 size;
	}hv_image;
	u32 cpu_count;
	char vendor_string[13];
	u8 cpu_manuf;
	//Use a 64-bit notation for an 8-byte alignment.
	u64 reserved[0x10];	//Reserve 128 bytes for Relative HVM.
}noir_hypervisor,*noir_hypervisor_p;

#if !defined(_central_hvm)
typedef struct _noir_hook_page
{
	memory_descriptor orig;
	memory_descriptor hook;
	void* pte_descriptor;
	struct _noir_hook_page* next;
}noir_hook_page,*noir_hook_page_p;
extern noir_hook_page_p noir_hook_pages;
#endif

#if defined(_central_hvm)
//Functions from VT Core.
bool nvc_is_vt_supported();
bool nvc_is_ept_supported();
bool nvc_is_vmcs_shadowing_supported();
bool nvc_vt_subvert_system(noir_hypervisor_p hvm);
void nvc_vt_restore_system(noir_hypervisor_p hvm);
//Functions from SVM Core.
bool nvc_is_svm_supported();
bool nvc_is_npt_supported();
bool nvc_is_acnested_svm_supported();
bool nvc_svm_subvert_system(noir_hypervisor_p hvm);
void nvc_svm_restore_system(noir_hypervisor_p hvm);
//Central Hypervisor Structure.
void nvc_store_image_info(ulong_ptr* base,u32* size);
noir_hypervisor_p hvm_p=null;
ulong_ptr system_cr3=0;
ulong_ptr orig_system_call=0;
char virtual_vstr[13]="AuthenticAMD\0";
char virtual_nstr[49]="AMD Ryzen 7 1700 Eight-Core Processor\0";
#else
extern noir_hypervisor_p hvm_p;
extern ulong_ptr system_cr3;
extern ulong_ptr orig_system_call;
#endif
extern void noir_system_call();