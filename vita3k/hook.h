#pragma once
#include <host/state.h>

#define EXPORTDLL __declspec(dllexport)

static HostState * current_host;

struct HookContext {
    int thread_id;
    uint32_t regs[16];
};

typedef void (*HookFunc)(HookContext *ctx);

 extern "C" EXPORTDLL bool inited() {
    return current_host;
}

 extern "C" EXPORTDLL void add_trampoline(uint32_t addr, bool thumb, HookFunc on_enter) {
    current_host->kernel.debugger.remove_trampoline(current_host->mem, addr);
    current_host->kernel.debugger.add_trampoile(current_host->mem, addr, thumb, [=](CPUState &cpu, MemState &mem, Address lr) {
        HookContext ctx;
        CPUContext cpu_ctx = save_context(cpu);
        std::copy(std::begin(cpu_ctx.cpu_registers), std::end(cpu_ctx.cpu_registers), std::begin(ctx.regs));
        ctx.thread_id = cpu.thread_id;
        on_enter(&ctx);
        write_pc(cpu, lr);
        return true;
    });
}

 extern "C" EXPORTDLL void remove_trampoline(uint32_t addr) {
    current_host->kernel.debugger.remove_trampoline(current_host->mem, addr);
 }


 extern "C" EXPORTDLL void *devirtualize(uint32_t vaddr) {
    return &current_host->mem.memory[vaddr];
}

 extern "C" EXPORTDLL uint32_t virtualize(void *addr) {
    const uintptr_t base = reinterpret_cast<uintptr_t>(&current_host->mem.memory[0]);
    return reinterpret_cast<uintptr_t>(addr) - base;
}