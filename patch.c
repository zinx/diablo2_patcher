#include "common.h"
#include "patch.h"
#include "slist.h"
#include "util.h"

/*** RAW PATCHING ***/

void patch_mem(void *dest, const void *data, size_t len)
{
	DWORD old, dummy;
	VirtualProtect(dest, len, PAGE_EXECUTE_READWRITE, &old);
	memcpy(dest, data, len);
	VirtualProtect(dest, len, old, &dummy);
}

void patch_rel(void *dest, const void *addr)
{
	DWORD rel_addr = REL(dest, addr);
	patch_mem(dest, &rel_addr, sizeof(rel_addr));
}

/*** FUNCTION PATCHING ***/

#define MAX_INSNS	28
struct pentry {
	patch_func_t entry, exit;
	void *data; /* for your own use */
	unsigned char ret_code[MAX_INSNS+6]; /* dynamically generated resume code */
	unsigned char entry_code[10]; /* dynamically generated entry code */
	union {
		HMODULE lib;
		slist_t list;
	} u;
	DWORD offset;
};

extern void patch_handler();

static slist_t pentry_head = {NULL};

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define PENTRY_PER_PAGE	(PAGE_SIZE/sizeof(struct pentry))

static struct pentry *pentry_alloc(void)
{
	struct pentry *node;
	if (slist_empty(&pentry_head)) {
		void *mem = VirtualAlloc(NULL, PAGE_SIZE, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
		DWORD dummy;
		VirtualProtect(mem, PAGE_SIZE, PAGE_EXECUTE_READWRITE, &dummy);
		void *mem_end = (void*)((char*)mem + PAGE_SIZE);
		for (node = (struct pentry*)mem; (void*)node < mem_end; ++node)
			slist_add(&node->u.list, &pentry_head);
	}
	node = slist_entry(pentry_head.next, struct pentry, u.list);
	slist_del(&node->u.list, &pentry_head);
	return node;
}

static void pentry_free(struct pentry *node)
{
	slist_add(&node->u.list, &pentry_head);
}

void *patch_function(HMODULE lib, DWORD offset, DWORD insns, patch_func_t entry, patch_func_t exit, void *data)
{
	struct pentry *node = pentry_alloc();

	node->u.lib = lib;
	node->offset = offset;
	node->entry = entry;
	node->exit = exit;
	node->data = data;

	node->entry_code[0] = 0x68; /* push */
	*((struct pentry**)&node->entry_code[1]) = node;
	node->entry_code[5] = 0xE9; /* jmp */
	SETREL(&node->entry_code[6], patch_handler);

	memcpy(&node->ret_code[0], LIBABS(lib, offset), insns);
	node->ret_code[insns] = 0xE9; /* jmp */
	SETREL(&node->ret_code[insns+1], LIBABS(lib, offset + insns));

	const unsigned char call_insn = 0xE9; /* jmp */
	patch_mem(LIBABS(lib, offset), &call_insn, 1);
	patch_rel(LIBABS(lib, offset) + 1, &node->entry_code[0]);

	return node;
}

void unpatch_function(void *func)
{
	struct pentry *node = (struct pentry*)func;
	patch_mem(LIBABS(node->u.lib, node->offset), node->ret_code, 5);
	pentry_free(node);
}
