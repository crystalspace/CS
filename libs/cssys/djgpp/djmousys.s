//
//  DOS	support	for Crystal Space 3D library
//  Copyright (C) 1998 by Jorrit Tyberghein
//  Written by Andrew Zabolotny	<bit@eltech.ru>
//
//  This library is free software; you can redistribute	it and/or
//  modify it under the	terms of the GNU Library General Public
//  License as published by the	Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed	in the hope that it will be useful,
//  but	WITHOUT	ANY WARRANTY; without even the implied warranty	of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR	PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  You	should have received a copy of the GNU Library General Public
//  License along with this library; if	not, write to the Free
//  Software Foundation, Inc., 675 Mass	Ave, Cambridge,	MA 02139, USA.
//

		.file	"djkeysys.s"
// externals (optional):
		.extern	___djgpp_base_address
		.extern	___djgpp_ds_alias
// public functions and	variables:
		.global	_install__12MouseHandler
		.global	_uninstall__12MouseHandler

		.data

		.align	4

work_rm_regs:
work_rm_edi:	.long	0
work_rm_esi:	.long	0
work_rm_ebp:	.long	0
work_rm_res:	.long	0
work_rm_ebx:	.long	0
work_rm_edx:	.long	0
work_rm_ecx:	.long	0
work_rm_eax:	.long	0
work_rm_flags:	.word	0
work_rm_es:	.word	0
work_rm_ds:	.word	0
work_rm_fs:	.word	0
work_rm_gs:	.word	0
work_rm_ip:	.word	0
work_rm_cs:	.word	0
work_rm_sp:	.word	0
work_rm_ss:	.word	0

__locked_data_start:

mouse_rm_regs:
mouse_rm_edi:	.long	0
mouse_rm_esi:	.long	0
mouse_rm_ebp:	.long	0
mouse_rm_res:	.long	0
mouse_rm_ebx:	.long	0
mouse_rm_edx:	.long	0
mouse_rm_ecx:	.long	0
mouse_rm_eax:	.long	0
mouse_rm_flags:	.word	0
mouse_rm_es:	.word	0
mouse_rm_ds:	.word	0
mouse_rm_fs:	.word	0
mouse_rm_gs:	.word	0
mouse_rm_ip:	.word	0
mouse_rm_cs:	.word	0
mouse_rm_sp:	.word	0
mouse_rm_ss:	.word	0

__locked_data_end:

		.text

__locked_code_start:

//----------------------------------------------------------------------------
// Description:
//   This procedure will be called every time a	mouse event happens
// Prototype:
//   static void mouse_handler ()
//----------------------------------------------------------------------------
		.align	4
mouse_handler:	pushl	%eax
		pushl	%ebx
		pushl	%ecx
		pushw	%ds

#if 0
// speaker click
movb $0xb6, %al
outb %al, $0x43
movb $0x08, %al
outb %al, $0x42
outb %al, $0x42
inb  $0x61, %al
orb  $3, %al
outb %al, $0x61
movl $0x00100000, %ecx
1:
decl %ecx
jnz 1b
inb  $0x61, %al
andb $0xfc, %al
outb %al, $0x61
#endif

		movl	(%esi),	%eax		// get return cs:ip
		es
		movl	%eax, 0x2a (%edi)	// put into current realmode cs:ip
		es
		addw	$4, 0x2e (%edi)		// simulate retf

		movw	%cs:___djgpp_ds_alias, %ds

		movl	_event_queue_head, %ebx

		testb	$0x01, mouse_rm_eax
		jz	1f
		leal	(%ebx,%ebx,2), %ecx
		shll	$2, %ecx
		movw	$2, _event_queue (%ecx)
		movb	$0, _event_queue+2 (%ecx)
		movzwl	mouse_rm_ecx, %eax
		movl	%eax, _event_queue+4 (%ecx)
		movzwl	mouse_rm_edx, %eax
		movl	%eax, _event_queue+8 (%ecx)
		incl	%ebx
		andb	$0x3f, %bl

1:		movb	mouse_rm_eax, %al
		testb	$0x06, %al
		jz	2f
		testb	$0x02, %al
		setnzb	%ah
		movb	$1, %al
		jmp	button_down

2:		testb	$0x18, %al
		jz	3f
		testb	$0x08, %al
		setnzb	%ah
		movb	$2, %al
		jmp	button_down

3:		testb	$0x60, %al
		jz	4f
		testb	$0x20, %al
		setnzb	%ah
		movb	$3, %al

button_down:	leal	(%ebx,%ebx,2), %ecx
		shll	$2, %ecx
		movw	$2, _event_queue (%ecx)
		movw	%ax, _event_queue+2 (%ecx)
		movzwl	mouse_rm_ecx, %eax
		movl	%eax, _event_queue+4 (%ecx)
		movzwl	mouse_rm_edx, %eax
		movl	%eax, _event_queue+8 (%ecx)
		incl	%ebx
		andb	$0x3f, %bl

4:		movl	%ebx, _event_queue_head

		popw	%ds
		popl	%ecx
		popl	%ebx
		popl	%eax
		iret

__locked_code_end:

//----------------------------------------------------------------------------
// Description:
//   Initializes the mouse handler and hooks the events	handler
//   Returns -1	on failure, zero on success
// Prototype:
//   int MouseHandler::install(void);
//----------------------------------------------------------------------------
		.align	4
_install__12MouseHandler:
		pushl	%esi
		pushl	%edi
		pushl	%ebx

// First, we need to lock the handler and memory it touches, so
// it doesn't get swapped out to disk.
		leal	__locked_data_start, %ecx
		leal	__locked_data_end, %edi
		call	lock_region
		jc	init_error
		leal	__locked_code_start, %ecx
		leal	__locked_code_end, %edi
		call	lock_region
		jc	init_error

// Allocate a callback
		pushw	%ds
		pushw	%es
		pushw	%cs
		popw	%ds
		leal	mouse_handler, %esi
		movw	___djgpp_ds_alias, %es
		leal	mouse_rm_regs, %edi
		movl	$0x303,	%eax
		int	$0x31
		popw	%es
		popw	%ds
		jc	init_error

// Call	int33h to set our mouse	handler
		movw	$0x000c, work_rm_eax
		movw	$0x007f, work_rm_ecx
		movl	%edx, work_rm_edx
		movw	%cx, work_rm_es
		xorl	%eax, %eax
		movw	%ax, work_rm_sp
		movw	%ax, work_rm_ss
		movw	%ax, work_rm_flags
		pushw	%es
		movw	___djgpp_ds_alias, %es
		leal	work_rm_regs, %edi
		movl	$0x0300, %eax
		movl	$0x0033, %ebx
		xorl	%ecx, %ecx
		int	$0x31
		popw	%es

// Actually we would have to unlock the	locked region on failure
// here. But since most	programs would exit with an error message
// in such case, there's no need to worry.
init_error:
// This	sets EAX to -1 if CF is	set and	to 0 atherwise
		movl	$0, %eax
		sbbl	$0, %eax

		popl	%ebx
		popl	%edi
		popl	%esi
		ret

//----------------------------------------------------------------------------
// Description:
//   Shuts the mouse handler down.
// Prototype:
//   void MouseHandler::uninstall(void);
//----------------------------------------------------------------------------
		.align 4
_uninstall__12MouseHandler:
		pushl	%esi
		pushl	%edi
		pushl	%ebx

// Call	int33h to remove our mouse handler
		movw	$0x000c, work_rm_eax
		movw	$0x0000, work_rm_ecx
		xorl	%eax, %eax
		movl	%eax, work_rm_edx
		movw	%ax, work_rm_es
		movw	%ax, work_rm_sp
		movw	%ax, work_rm_ss
		movw	%ax, work_rm_flags
		pushw	%es
		movw	___djgpp_ds_alias, %es
		leal	work_rm_regs, %edi
		movl	$0x0300, %eax
		movl	$0x0033, %ebx
		xorl	%ecx, %ecx
		int	$0x31
		popw	%es

// Unlock the region we	locked at initialization
		leal	__locked_data_start, %ecx
		leal	__locked_data_end, %edi
		call	unlock_region
		leal	__locked_code_start, %ecx
		leal	__locked_code_end, %edi
		call	unlock_region

		popl	%ebx
		popl	%edi
		popl	%esi
		ret

//----------------------------------------------------------------------------
// Description:
//   Lock a linear address space region
// Parameters:
//   ecx = start address
//   edi = final address
//----------------------------------------------------------------------------
lock_region:
		subl	%ecx, %edi
		addl	___djgpp_base_address, %ecx
		shldl	$16, %ecx, %ebx		// ecx -> bx:cx
		shldl	$16, %edi, %esi		// edi -> si:di
		movw	$0x0600, %ax		// lock	linear region
		int	$0x31
		ret

//----------------------------------------------------------------------------
// Description:
//   Unlock a linear address space region
// Parameters:
//   ecx = start address
//   edi = final address
//----------------------------------------------------------------------------
unlock_region:
		subl	%ecx, %edi
		addl	___djgpp_base_address, %ecx
		shldl	$16, %ecx, %ebx
		shldl	$16, %edi, %esi
		movw	$0x0601, %ax		// unlock linear region
		int	$0x31
		ret
