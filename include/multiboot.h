#pragma once

#include "types.h"

typedef unsigned int uint32_t;

// GRUB 启动时, 将内存信息存储在 multiboot 结构
//

// GRUB 规范 的 multiboot 结构体
typedef struct {
	uint32_t flags;			// Multiboot 的版本信息
	uint32_t mem_lower;
	uint32_t mem_upper;

	uint32_t boot_device;		// 指出引导程序从哪个BIOS磁盘设备载入的OS映像
	uint32_t cmdline;		// 内核命令行
	uint32_t mods_count;		// boot 模块列表
	uint32_t mods_addr;
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
	uint32_t mmap_length;
	uint32_t mmap_addr;

	uint32_t drives_length; 	// 指出第一个驱动器结构的物理地址
	uint32_t drives_addr; 		// 指出第一个驱动器这个结构的大小
	uint32_t config_table; 		// ROM 配置表
	uint32_t boot_loader_name; 	// boot loader 的名字
	uint32_t apm_table; 	    	// APM 表
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint32_t vbe_mode;
	uint32_t vbe_interface_seg;
	uint32_t vbe_interface_off;
	uint32_t vbe_interface_len;
} __attribute__((packed)) multiboot;



typedef struct {
	uint32_t size; 		
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t length_low;
	uint32_t length_high;
	uint32_t type;
} __attribute__((packed)) mmap_entry;

extern multiboot *glb_mboot_ptr;

