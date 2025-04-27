/*
 * Copyright (c) 2025 
 *
 * funchook.h - function hook header
 * author: Xing Qingjie <xqjcool@gmail.com>
 * version: 1.0.0
 * history: 04/27/2025	created
 */
#ifndef _FUNCHOOK_H_
#define _FUNCHOOK_H_

/*
 * call the original function in hook function
 * @_name_: name of the original function
 * @...: params to be passed
 */
#define CALL_ORIG_FUNCION(_name_, ...)   stub_##_name_(__VA_ARGS__)

/*
 * define the hook function and related definitions
 * @_name_: name of the original function
 * @_retype_: return value type of the original function
 * @...: params definition of the original function
 */
#define HOOK_DEFINE(_name_, _retype_, ...) \
	static _retype_ (*ptr_##_name_)(__VA_ARGS__); \
static _retype_ stub_##_name_(__VA_ARGS__) \
{ \
        asm volatile ( \
	    "nop;nop;nop;nop;nop;nop;nop;nop;" \
	    "nop;nop;nop;nop;nop;nop;nop;nop;" \
	    "nop;nop;nop;nop;nop;nop;nop;nop;" \
	    "nop;nop;nop;nop;nop;nop;nop;nop;" \
        ); \
	return (_retype_)0; \
} \
static _retype_ hook_##_name_(__VA_ARGS__) \

/*
 * register the hook function
 * @_name_: name of the original function
 */
#define HOOK_REGISTER(_name_) \
	hook_register(#_name_, (void **)&ptr_##_name_, \
			(void *)hook_##_name_, (void *)stub_##_name_)

int hook_register(const char *func_name, void **pptr_func, 
		void *hook_func, void *stub_func);

#endif
