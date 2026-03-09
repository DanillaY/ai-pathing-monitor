.global module_hook_function
.type module_hook_function, @function

.extern set_new_variable

module_hook_function:
    push %rax
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    pushfq
    
    mov %rbx, %rax
    call set_new_variable
    
    popfq
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rax

    jz     .jump_to_target
    jmp    .continue_flow

    .jump_to_target:
        movabs $0x019a81b7, %r9
        jmp    *%r9

    .continue_flow:
        movabs $0x019a8012, %r9
        jmp    *%r9

.size module_hook_function, .-module_hook_function
