//
//  DOS support for Crystal Space 3D library
//  Copyright (C) 1998 by Jorrit Tyberghein
//  Written by David N. Arnold <derek_arnold@fuse.net>
//  Written by Andrew Zabolotny <bit@eltech.ru>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Library General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this library; if not, write to the Free
//  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

		.file	"djkeysys.s"
// externals (optional):
		.extern	___djgpp_base_address
		.extern	___djgpp_ds_alias
		.extern	___djgpp_dos_sel
// public functions and	variables:
		.global	_install__15KeyboardHandler
		.global	_uninstall__15KeyboardHandler
		.global	_chain__15KeyboardHandleri
		.global	_event_queue
		.global	_event_queue_head
		.global	_event_queue_tail

		.data

		.align	4

__locked_data_start:

old_vector:
old_vector_ofs:	.long	0
old_vector_sel:	.word	0
chain_flag:	.long	1
irq1:		.byte	0

// C/C++ extern	variables
_event_queue_head:
		.long	0
_event_queue_tail:
		.long	0
_event_queue:
		.space	0x40*12,0

__locked_data_end:

		.text

__locked_code_start:

//----------------------------------------------------------------------------
// Description:
//   Keyboard (IRQ1) handler
//   ... will be called	every time a key is pressed/released
//----------------------------------------------------------------------------
		.align	4
handler_procedure:
		pushl	%eax
		pushl	%edx
		pushw	%ds

// Load	DS with	our data selector
		movw	%cs:___djgpp_ds_alias, %ds

// Read	the scancode from keyboard port	and put	it into	scancode buffer
		inb	$0x60, %al

// If we got a strange keycode,	chain to old keyboard handler
// This	is used	by OS/2	to detect idle DOS sessions
		cmpb	$0xE1, %al
		ja	handler_chain

		movl	_event_queue_head, %edx
		incl	%edx
		andl	$0x3f, %edx
		cmpl	_event_queue_tail, %edx
		je	buffer_full
		xchgl	_event_queue_head, %edx
		leal	(%edx, %edx, 2), %edx
		shll	$2, %edx
		movw	$1, _event_queue(%edx)
		movb	%al, _event_queue+2(%edx)

buffer_full:
// Chain if flag is set, otherwise do what's necessary and return
		cmpl	$0, chain_flag
		jne	handler_chain

// Acknowledge keyboard	and interrupt contollers
		inb	$0x61, %al
		movb	%al, %ah
		orb	$0x80, %al
		outb	%al, $0x61
		movb	%ah, %al
		outb	%al, $0x61
		movb	$0x20, %al
		outb	%al, $0x20

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
movl $0x00100000, %edx
1:
decl %edx
jnz 1b
inb  $0x61, %al
andb $0xfc, %al
outb %al, $0x61
#endif

		popw	%ds
		popl	%edx
		popl	%eax
		iret

		.align	4
handler_chain:	popw	%ds
		popl	%edx
		popl	%eax
		ljmp	%cs:(old_vector)

__locked_code_end:

//----------------------------------------------------------------------------
// Description:
//   Initializes the keyboard handler and hooks	the keyboard interrupt.
//   Returns -1	on failure, zero on success
// Prototype:
//   int KeyboardHandler::install(void);
//----------------------------------------------------------------------------
		.align	4
_install__15KeyboardHandler:
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

// Clear scancode buffer
		xorl	%eax, %eax
		movl	%eax, _event_queue_head
		movl	%eax, _event_queue_tail

// Compute IRQ0	interrupt number
		movw	$0x0400, %ax
		int	$0x31
		incb	%dh
		movb	%dh, irq1

// Now we need to save the old interrupt vector, so we can restore
// it later and	also to	know where to jump if chaining.
		movw	$0x0204, %ax		// get pm int vector
		movb	irq1, %bl
		int	$0x31
		movw	%cx, old_vector_sel
		movl	%edx, old_vector_ofs

// Make	sure we	chain after initialization.
		movl	$1, chain_flag

// Set the interrupt vector to point to	our handler.
		movw	%cs, %cx
		leal	handler_procedure, %edx
		movb	irq1, %bl
		movw	$0x0205, %ax		// set pm int vector
		int	$0x31

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
//   Shuts the keyboard	handler	down.
// Prototype:
//   void KeyboardHandler::uninstall(void);
//----------------------------------------------------------------------------
		.align 4
_uninstall__15KeyboardHandler:
		pushl	%esi
		pushl	%edi
		pushl	%ebx

// Restore the interrupt vector	to its previous	value
		movw	old_vector_sel,	%cx
		movl	old_vector_ofs,	%edx
		movb	irq1, %bl
		movw	$0x0205, %ax		// set pm int vector
		int	$0x31

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
//   Enable/disable passing control to old IRQ0	handler
// Prototype:
//   void KeyboardHandler::chain(int toggle);
//----------------------------------------------------------------------------
		.align	4
_chain__15KeyboardHandleri:
		cmpl	$0, 8(%esp)
		je	chain_off
chain_on:
// Set the chain_flag and clear	BIOS shift/ctrl/alt status bits:
		movl	$1, chain_flag

		pushw	%es
		movw	___djgpp_dos_sel, %es
		andb	$0xf0, %es:0x417
		andb	$0xf0, %es:0x496
		popw	%es
		jmp	chain_done
chain_off:
		movl	$0, chain_flag
chain_done:	ret

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
