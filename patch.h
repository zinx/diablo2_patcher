#ifndef patcher_PATCH_h
#define patcher_PATCH_h 1

#ifdef __cplusplus
extern "C" {
#endif

#define LIBABS(lib,offset) \
	((void*)((char*)(lib) + (offset)))

#define REL(at,addr) \
	((DWORD)(((char*)(addr)) - ((char*)(at)) - 4))

#define SETREL(dst,src) \
	(*(DWORD*)(dst) = REL(dst, src))

#define LE_BYTES(x) \
	(x)&0xff, ((x)>>8)&0xff, ((x)>>16)&0xff, ((x)>>24)&0xff

#define LE_BYTES_W(x) \
	(x)&0xff, ((x)>>8)&0xff

/*** Raw patching ***/

void patch_mem(void *dest, const void *data, size_t len);
void patch_rel(void *dest, const void *addr);

/*** Function overriding ***/

enum {
	R_EDI,
	R_ESI,
	R_EBP,
	R_ESP,
	R_EBX,
	R_EDX,
	R_ECX,
	R_EAX,
	R_COUNT
};

typedef struct patch_ctx_s patch_ctx_t;
struct patch_ctx_s {
	void *data;
	DWORD regs[R_COUNT];
};

#define patch_ctx_reg(ctx,n) ((ctx)->regs[n])
#define patch_ctx_arg(ctx,n) (((DWORD*)ctx->regs[R_ESP])[(n)+2])
#define patch_ctx_ret(ctx) ((void*)patch_ctx_arg(ctx, -1))

typedef void (_cdecl *patch_func_t)(patch_ctx_t *ctx, void *data);

void *patch_function(HMODULE lib, DWORD offset, DWORD insns, patch_func_t entry, patch_func_t exit, void *data);
void unpatch_function(void *func);

#ifdef __cplusplus
}
#endif

#endif
