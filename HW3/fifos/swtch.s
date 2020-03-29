#https://github.com/mit-pdos/xv6-public/blob/master/swtch.S
# Context switch
#
#   void swtch(uint32_t **old_stack, unit32_t *new_stack);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

.globl swtch
swtch:
  #movl 4(%esp), %eax
  #movl 8(%esp), %edx

  # Save old callee-saved registers
  pushf                #Push the flags register to the stack
  pushl %eax
  pushl %ecx
  pushl %edx
  pushl %ebp
  pushl %ebx
  pushl %esi
  pushl %edi
  pushw %ds
  pushw %es
  pushw %fs
  pushw %gs 

  # Switch stacks
  movl %esp, 44(%esp)
  movl 48(%esp), %esp

  # Load new callee-saved registers
  popw %gs
  popw %fs
  popw %es
  popw %ds
  popl %edi
  popl %esi
  popl %ebx
  popl %ebp
  popl %edx
  popl %ecx
  popl %eax
  popf
  
  ret                #Jump to the new thread's eip
