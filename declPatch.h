#if !defined(HEADER) || !defined(NAME)
#error "Must define HEADER and NAME!"
#endif

#include "util.h"

#define PATCH_BEGIN(file,csum) __PATCH_BEGIN(NAME,file,csum)
#define __PATCH_BEGIN(name,file,csum) _PATCH_BEGIN(name,file,csum)
#define PATCH_END() __PATCH_END(NAME)
#define __PATCH_END(name) _PATCH_END(name)
#define PATCH(offset,...) __PATCH(NAME,offset,__VA_ARGS__)
#define __PATCH(name,offset,...) _PATCH(name,offset,__VA_ARGS__)
#define PATCH_REL(offset,addr) __PATCH_REL(NAME,offset,addr)
#define __PATCH_REL(name,offset,addr) _PATCH_REL(name,offset,addr)
#define PATCH_VAR(offset,var) __PATCH_VAR(NAME,offset,var)
#define __PATCH_VAR(name,offset,var) _PATCH_VAR(name,offset,var)
#define PATCH_ORIG(offset,...) __PATCH_ORIG(NAME,offset,__VA_ARGS__)
#define __PATCH_ORIG(name,offset,...) _PATCH_ORIG(name,offset,__VA_ARGS__)

/*** PATCHES OBJECT ***/
#define _PATCH_BEGIN(name,file,csum) \
	static struct { \
		HMODULE lib; \
		unsigned char *mem; \
	} name = { NULL, NULL };

#define _PATCH_END(name)

#define _PATCH(name,offset,...)
#define _PATCH_REL(name,offset,addr)
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...)

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG

/*** PATCH FUNCTIONS ***/
/*check csum/data*/
#define _PATCH_BEGIN(name,file,csum) \
	int patch_##name##_check(void) \
	{ \
		const unsigned int want_checksum = (csum); \
		unsigned int checksum;
#define _PATCH_END(name)

#define _PATCH(name,offset,...)
#define _PATCH_REL(name,offset,addr)
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...) \
	const unsigned char name##_##offset##_orig[] = { __VA_ARGS__ };

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG




#define _PATCH_BEGIN(name,file,csum) \
	checksum = util_csum_library(name.lib); \
	if (checksum != want_checksum) { \
		LOG("CSUM mismatch for " #name "; expected %08x, got %08x", want_checksum, checksum); \
		return -2; \
	}
#define _PATCH_END(name) \
		return 0; \
	}

#define _PATCH(name,offset,...)
#define _PATCH_REL(name,offset,addr)
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...) \
	if (memcmp(LIBABS(name.lib, offset), name##_##offset##_orig, sizeof(name##_##offset##_orig))) { \
		LOG("Expected " #__VA_ARGS__ " at " #name "+%08x", offset); \
		return -3; \
	}

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG

/*variables*/
#define _PATCH_BEGIN(name,file,csum) \
	void patch_##name##_var(void) \
	{
#define _PATCH_END(name) \
	}

#define _PATCH(name,offset,...)
#define _PATCH_REL(name,offset,addr)
#define _PATCH_VAR(name,offset,var) \
		(var) = LIBABS(name.lib, offset);
#define _PATCH_ORIG(name,offset,...)

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG

/*data*/
#define _PATCH_BEGIN(name,file,csum) \
	void patch_##name##_data(void) \
	{
#define _PATCH_END(name)

#define _PATCH(name,offset,...) \
	unsigned char name##_##offset##_data[] = { __VA_ARGS__ };
#define _PATCH_REL(name,offset,addr) \
	void *name##_##offset##_data = addr;
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...)

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG



#define _PATCH_BEGIN(name,file,csum) \
	size_t len = sizeof(void*)
#define _PATCH_END(name) \
	; \
	unsigned char *mem = malloc(len);

#define _PATCH(name,offset,...) \
	+ sizeof(name##_##offset##_data) + sizeof(void*) + sizeof(unsigned int)
#define _PATCH_REL(name,offset,addr) \
	_PATCH(name,offset,addr)
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...)

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG




#define _PATCH_BEGIN(name,file,csum) \
		*(void**)mem = NULL; mem += sizeof(void*);
#define _PATCH_END(name) \
		name.mem = mem; \
	}

#define _PATCH(name,offset,...) \
	memcpy(mem, LIBABS(name.lib, offset), sizeof(name##_##offset##_data)); mem += sizeof(name##_##offset##_data); \
	*(unsigned int*)mem = sizeof(name##_##offset##_data); mem += sizeof(unsigned int); \
	*(void**)mem = LIBABS(name.lib, offset); mem += sizeof(void*); \
	patch_mem(LIBABS(name.lib, offset), name##_##offset##_data, sizeof(name##_##offset##_data));
#define _PATCH_REL(name,offset,addr) \
	memcpy(mem, LIBABS(name.lib, offset), sizeof(name##_##offset##_data)); mem += sizeof(name##_##offset##_data); \
	*(unsigned int*)mem = sizeof(name##_##offset##_data); mem += sizeof(unsigned int); \
	*(void**)mem = LIBABS(name.lib, offset); mem += sizeof(void*); \
	patch_rel(LIBABS(name.lib, offset), name##_##offset##_data);
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...)

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG

/*cleanup func*/
#define _PATCH_BEGIN(name,file,csum) \
	void unpatch_##name(void) \
	{ \
		unsigned char *mem, *addr; \
		if (!name.lib) return; \
		mem = name.mem; \
		/* Unpatch in reverse */ \
		while ((addr = (*(void**)(mem -= sizeof(void*))))) { \
			unsigned int len = *(unsigned int*)(mem -= sizeof(unsigned int)); \
			mem -= len; \
			patch_mem(addr, mem, len); \
		}
#define _PATCH_END(name) \
		free(mem); \
		FreeLibrary(name.lib); \
		name.lib = NULL; \
	}

#define _PATCH(name,offset,...)
#define _PATCH_REL(name,offset,addr)
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...)

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG

/*patchp func*/
#define _PATCH_BEGIN(name,file,csum) \
	int patch_##name(void) \
	{ \
		int ret; \
		if (name.lib) return 0; \
		name.lib = LoadLibrary(file); \
		if (!name.lib) return -1; \
		if ((ret = patch_##name##_check())) { \
			FreeLibrary(name.lib); \
			name.lib = NULL; \
			return ret; \
		} \
		patch_##name##_var(); \
		patch_##name##_data();
#define _PATCH_END(name) \
		return 0; \
	}

#define _PATCH(name,offset,...)
#define _PATCH_REL(name,offset,addr)
#define _PATCH_VAR(name,offset,var)
#define _PATCH_ORIG(name,offset,...)

#include HEADER

#undef _PATCH_BEGIN
#undef _PATCH_END
#undef _PATCH
#undef _PATCH_REL
#undef _PATCH_VAR
#undef _PATCH_ORIG

/*** fin ***/

#undef PATCH_BEGIN
#undef PATCH_END
#undef PATCH
#undef PATCH_REL
#undef PATCH_VAR
#undef PATCH_ORIG
#undef __PATCH_BEGIN
#undef __PATCH_END
#undef __PATCH
#undef __PATCH_REL
#undef __PATCH_VAR
#undef __PATCH_ORIG

#undef HEADER
#undef NAME
