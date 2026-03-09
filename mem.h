#include <stdint.h>
#include <stdbool.h>

#pragma once

void set_new_variable(uintptr_t new_variable);

bool is_mapped(uintptr_t addr);

#ifdef __cplusplus
extern "C" {
#endif

void module_hook_function();

#ifdef __cplusplus
}
#endif

void patch_for_code_cave_jump(uintptr_t patch_addr, uintptr_t cave_addr, void (*target_func)());

void make_text_writable_and_executable();