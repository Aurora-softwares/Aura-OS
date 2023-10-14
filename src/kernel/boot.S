#include "arch/x86_64/gdt.h"
#include "arch/x86_64/mmu.h"
#include "kernel.h"
#include "arch/x86_64/sizes.h"
#include "arch/x86_64/multiboot2.h"
#include "arch/x86_64/msr.h"

// Define the length and checksum for the Multiboot2 header
.SET HEADER_LENGTH, header_end - header_start
.SET CHECKSUM, -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 + HEADER_LENGTH)
.section .multiboot
header_start:
	.long MULTIBOOT2_HEADER_MAGIC		// Magic value to identify Multiboot2 header
	.long MULTIBOOT_ARCHITECTURE_I386	// Architecture tag (for x86, use I386)
	.long HEADER_LENGTH					// Length of the header, including this field
	.long CHECKSUM						// Checksum to validate the header content

// Additional Multiboot tags can go here

.short MULTIBOOT_HEADER_TAG_END			// End tag to indicate the end of the header
.short 0								// Flags for Multiboot header (none set)
.long 8									// Size of the header, including this end tag
header_end:

.code32

.section .bss
// Reserve space for various data structures (page tables, stacks, etc.)
.comm pml4, PML4_SIZE, PML4_ALIGNMENT
.comm low_pdpt, PDPT_SIZE, PDPT_ALIGNMENT
.comm high_pdpt, PDPT_SIZE, PDPT_ALIGNMENT
.comm low_page_directory_table, PAGE_DIRECTORY_SIZE, PAGE_DIRECTORY_ALIGNMENT
.comm high_page_directory_table, PAGE_DIRECTORY_SIZE, PAGE_DIRECTORY_ALIGNMENT
.comm tmp_stack, KERNEL_BOOT_STACK_SIZE, KERNEL_BOOT_STACK_ALIGNMENT
.section .data
.align GDT_TABLE_ALIGNMENT
gdt_table:
	.8byte GDT_FIRST_ENTRY		// GDT entry for the first segment (unused)
	.8byte GDT_KERNEL_ENTRY		// GDT entry for the kernel segment

gdt_table_end:
	.skip (GDT_TABLE_SIZE - (gdt_table_end - gdt_table))

gdt_ptr:
	.short GDT_TABLE_SIZE - 1	// Limit of the GDT table (size - 1)
	.long gdt_table				// Base address of the GDT table


.section .text
.global _start
.type _start, @function
_start:
	movl $tmp_stack + KERNEL_BOOT_STACK_SIZE, %esp	// Set up the kernel stack

	// Set up the Page Map Level 4 (PML4) with the physical and virtual address mappings
	movl $low_pdpt, %eax
	or $(MMU_PRESENT | MMU_WRITABLE), %eax
	movl %eax, pml4 + (PML4_ADDR_TO_ENTRY_INDEX(KERNEL_PHYSICAL_START) * PML4_ENTRY_SIZE)

	movl $high_pdpt, %eax
	or $(MMU_PRESENT | MMU_WRITABLE), %eax
	movl %eax, pml4 + (PML4_ADDR_TO_ENTRY_INDEX(KERNEL_VIRTUAL_START) * PML4_ENTRY_SIZE)

	// Set up the Page Directory Pointer Tables (PDPT) for both low and high mappings
	movl $low_page_directory_table, %eax
	or $(MMU_PRESENT | MMU_WRITABLE), %eax
	movl %eax, low_pdpt + (PDPT_ADDR_TO_ENTRY_INDEX(KERNEL_PHYSICAL_START) * PDPT_ENTRY_SIZE)

	movl $high_page_directory_table, %eax
	or $(MMU_PRESENT | MMU_WRITABLE), %eax
	movl %eax, high_pdpt + (PDPT_ADDR_TO_ENTRY_INDEX(KERNEL_VIRTUAL_START) * PDPT_ENTRY_SIZE)

	// Set up page directory entries for each 2 MB block of memory to be mapped
	mov $0, %ecx
	movl $_kernel_physical_end, %esi
	shrl $TWO_MEGABYTES_SHIFT, %esi
	addl $1, %esi

page_directory_table_loop:
	movl $TWO_MEGABYTES, %eax
	mul %ecx
	or $(MMU_PRESENT | MMU_WRITABLE | MMU_PDE_TWO_MB), %eax
	// Set the low_page_directory_table entry
	movl %eax, low_page_directory_table(, %ecx, PAGE_DIRECTORY_ENTRY_SIZE)
	// Set the high_page_directory_table entry
    movl %eax, high_page_directory_table(, %ecx, PAGE_DIRECTORY_ENTRY_SIZE)

	inc %ecx
	cmp %esi, %ecx
	jne page_directory_table_loop	// If not equal, redo the loop for the next block

	// Load the Page Map Level 4 (PML4) into CR3 register for enabling paging
	movl $pml4, %eax
	movl %eax, %cr3

	// Set various control registers for enabling long mode (64-bit)
	movl $KERNEL_CR4, %eax
	movl %eax, %cr4

	movl $MSR_EFER, %ecx
	rdmsr
	or $MSR_EFER_LME, %eax
	wrmsr

	movl $KERNEL_CR0, %eax
	movl %eax, %cr0

	// Load the Global Descriptor Table (GDT) using LGDT instruction
	lgdt gdt_ptr

	// Jump to 64-bit mode by using a long jump instruction (ljmp)
	ljmp $(KERNEL_GDT_ENTRY * GDT_ENTRY_SIZE), $_start64

	// Disable interrupts (CLI) and halt the system (HLT)
	cli
	hlt

.code64

.global _start64
.type _start64, @function
_start64:
	// Set up segment selectors to zero (DS, ES, FS, GS, SS) since they are unused
	movw $0, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss


	// Call the C function `kernel_init` (defined in kernel.c)
	call kernel_init

	// Should never reach here, but add an infinite loop for safety
	cli
	hlt
1:
	jmp 1b
