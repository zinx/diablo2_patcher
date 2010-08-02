.bss
gleaned_node_ret:
	.align 4
	.space 4*32*2
gleaned_cnt:
	.align 4
	.space 4

.text
.globl _patch_handler
	.def    _patch_handler
		.scl    2
		.type   32
	.endef
_patch_handler:
	/* Stack here: node, return address, arguments */
	pushal
	movl 4*8(%esp), %ebx /* struct pentry *node */
	pushl 8(%ebx) /* node->data */
	pushl %esp /* ctx */
	movl (%ebx), %eax
	testl %eax, %eax
	jz 1f
	call *%eax /* call node->entry */
1:
	addl 8, %esp /* eat args */

	/* Save node & orig return address in gleaned_node_ret array */
	movl (gleaned_cnt), %ecx
	lea gleaned_node_ret(,%ecx,8), %eax
	movl %ebx, (%eax) /* save node in gleaned_node_ret[gleaned_cnt*2] */
	movl 4*8+4(%esp), %edx
	movl %edx, 4(%eax) /* save return address in gleaned_node_ret[gleaned_cnt*2+1] */
	incl %ecx
	movl %ecx, (gleaned_cnt)

	/* Patch the return address so the function reenters this handler when done */
	movl handle_return, %eax
	movl %eax, 4*8+4(%esp) /* overwrite return address */

	/* Call original function after restoring regs, by way of ret_code */
	lea 12(%edx), %ebx /* node->ret_code */
	movl %ebx, 4*8(%esp) /* overwrite node with ret_code addr */
	popal
	ret

	/* Function return */
handle_return:
	pushl %eax /* to be overwritten with original return address */
	pushal

	movl (gleaned_cnt), %ecx
	decl %ecx
	movl %ecx, (gleaned_cnt)

	lea gleaned_node_ret(,%ecx,8), %ebx /* &gleaned_node_ret[gleaned_cnt*2] */
	movl 4(%ebx), %eax /* reload saved return address */
	movl %eax, 4*8(%esp) /* restore original return address */

	movl (%ebx), %ebx /* node */
	pushl 8(%ebx) /* node->data */
	pushl %esp /* ctx */
	movl 4(%ebx), %eax /* node->exit */
	testl %eax, %eax
	jz 3f
	call *%eax /* call node->exit */
3:
	addl 8, %esp /* eat args */
	popal
	ret /* return to original address */
