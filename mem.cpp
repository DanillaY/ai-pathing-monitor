
#include <cstdint>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <unistd.h>

#include "shared.h"

extern "C" void module_hook_function();

extern "C" void set_new_variable(uintptr_t new_variable) {
    if(new_variable != 0x0 && pathing_head_addr != new_variable) {
        pathing_head_addr = new_variable;
    }
}

bool is_mapped(uintptr_t addr)
{
    std::ifstream maps("/proc/self/maps");
    std::string line;

    while (std::getline(maps, line))
    {
        uintptr_t start, end;

        if (sscanf(line.c_str(), "%lx-%lx", &start, &end) == 2)
        {
            if (addr >= start && addr < end)
            {
                return true;
            }
        }
    }

    return false;
}


void patch_for_code_cave_jump(uintptr_t patch_addr, uintptr_t cave_addr, void (*target_func)()) {
    uint8_t stub[] = {
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,
        0xFF, 0xE0
    };

    *reinterpret_cast<uint64_t*>(stub + 2) = reinterpret_cast<uint64_t>(target_func);
    memcpy(reinterpret_cast<void*>(cave_addr), stub, sizeof(stub));

    int32_t rel = static_cast<int32_t>(cave_addr - (patch_addr + 5));
    uint8_t jmp[5] = {0xE9};
    memcpy(jmp + 1, &rel, 4);
    memcpy(reinterpret_cast<void*>(patch_addr), jmp, sizeof(jmp));
}

void make_text_writable_and_executable() {
    const long page_size = sysconf(_SC_PAGESIZE);

    uintptr_t text_start = 0x400000;
    uintptr_t text_size = 0x1d3d000 - 0x0400000;
    uintptr_t text_end = text_start + text_size;

    text_start &= ~(page_size - 1);
    text_end = (text_end + page_size - 1) & ~(page_size - 1);

    std::cout << "Aligned text_start: " << std::hex << text_start << std::endl;
    std::cout << "Aligned text_end: " << std::hex << text_end << std::endl;

    for (uintptr_t addr = text_start; addr < text_end; addr += page_size) {
        uintptr_t size = std::min(static_cast<uintptr_t>(page_size), text_end - addr);

        if (mprotect(reinterpret_cast<void*>(addr), size, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
            perror("mprotect failed");
            std::cout << "Failed to protect address range: " << "0x" << std::hex << addr << " to 0x" << std::hex << addr + size << std::endl;
            return;
        }
    }
}