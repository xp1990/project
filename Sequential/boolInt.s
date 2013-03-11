	.file	"boolInt.c"
	.text
	.p2align 4,,15
	.globl	f
	.type	f, @function
f:
.LFB0:
	.cfi_startproc
	cmpl	$1, %edi
	sbbl	%eax, %eax
	andb	$58, %al
	addl	$99, %eax
	ret
	.cfi_endproc
.LFE0:
	.size	f, .-f
	.p2align 4,,15
	.globl	g
	.type	g, @function
g:
.LFB1:
	.cfi_startproc
	cmpb	$1, %dil
	sbbl	%eax, %eax
	andb	$58, %al
	addl	$99, %eax
	ret
	.cfi_endproc
.LFE1:
	.size	g, .-g
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	rep
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.7.2-2ubuntu1) 4.7.2"
	.section	.note.GNU-stack,"",@progbits
