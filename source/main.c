#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <wafel/dynamic.h>
#include <wafel/ios_dynamic.h>
#include <wafel/utils.h>
#include <wafel/patch.h>
#include <wafel/ios/svc.h>
#include <wafel/trampoline.h>

void (*crypto_printf)(const char*, ...) = (void*)0x040153d0;

u32 getKeyHook(int *key, int *size, u32 keyid, int r3, int (*crypto_get_key)(int*, int*, u32), u32 lr){
    debug_printf("GetKeyHook(%p,%p,%u), chain: %p, lr: %p\n", key, size, keyid, crypto_get_key, lr);
    u32 ret = crypto_get_key(key, size, keyid);
    debug_printf("crypto_get_key returned: %u\n", ret);
    return ret;
}

void print_state(trampoline_state *state){
    debug_printf("state %p: r0: %p, r1: %p, r2: %p, r3: %p, r4: %p, r5: %p, r6: %p, r7: %p, r8: %p, r9: %p, r10: %p, r11: %p, r12: %p, lr: %p\n", state,
            state->r[0],state->r[1],state->r[2],state->r[3],state->r[4],state->r[5],state->r[6],state->r[7],state->r[8],state->r[9],state->r[10],state->r[11], state->r[12], state->lr);
}

void print_start(){
    debug_printf("start hook\n");
}

void print_end(trampoline_state *state){
    crypto_printf("end hook crypto printf\n");
}

int mount_wrapper(char* dev, char *dir, char *mount_point, void *owner, int (*orgfunc)(char*, char*, char*, void*)){
    debug_printf("Calling MCP_MountWithSubdir(%s, %s, %s, %p) at %p\n", dev, dir, mount_point, owner, orgfunc);
    int ret = orgfunc(dev, dir, mount_point, owner);
    debug_printf("MCP_MountWithSubdir returned %X\n", ret);
    return ret;
}

int dev_wait_wrapper(char* dev, u32 timeout, int r2, int r3, int (*orgfunc)(char*, u32)){
    timeout = timeout * 2;
    debug_printf("Increase timout for %s to %u\n", dev, timeout);
    return orgfunc(dev, timeout);
}

void print_thumb_state(trampoline_t_state *state){
    debug_printf("THUMB state %p: r0: %p, r1: %p, r2: %p, r3: %p, r4: %p, r5: %p, r6: %p, r7: %p, lr: %p\n", state,
            state->r[0],state->r[1],state->r[2],state->r[3],state->r[4],state->r[5],state->r[6],state->r[7], state->lr);
}

void after_mlc_hook(trampoline_t_state *state){
    debug_printf("HOOK: MLC MOUNTED!!!!\n");
}


void install_arm_trampolines(void){
    // the original overwritten instruction will be added to the end of the trampoline automatically
    trampoline_hook_before(0x04002d18, print_end);
    // it is possible to hook the same location multiple times
    // the old target is extracted from the overwritten bl instruction!
    trampoline_hook_before(0x04002d18, print_state);
    trampoline_hook_before(0x04002d18, print_start);

    trampoline_hook_before(0x04002d80, print_state);
    // also blreplace can chain existing hooks
    trampoline_blreplace(0x04002d80 ,getKeyHook);

    trampoline_hook_before(0x04002d88, print_state);
}

void install_thumb_trampolines(void){
    trampoline_t_blreplace(0x05027d18, dev_wait_wrapper);
    trampoline_t_blreplace(0x05027d54, mount_wrapper);
    trampoline_t_hook_before(0x05027d58, print_thumb_state);
    trampoline_t_hook_before(0x05027d58, after_mlc_hook);
}

// This fn runs before everything else in kernel mode.
// It should be used to do extremely early patches
// (ie to BSP and kernel, which launches before MCP)
// It jumps to the real IOS kernel entry on exit.
__attribute__((target("arm")))
void kern_main()
{
    // Make sure relocs worked fine and mappings are good
    debug_printf("we in here mlc check plugin kern %p\n", kern_main);

    debug_printf("init_linking symbol at: %08x\n", wafel_find_symbol("init_linking"));

    install_arm_trampolines();
    install_thumb_trampolines();
}

// This fn runs before MCP's main thread, and can be used
// to perform late patches and spawn threads under MCP.
// It must return.
void mcp_main()
{

}
