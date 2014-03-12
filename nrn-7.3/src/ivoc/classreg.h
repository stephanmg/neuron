#ifndef classreg_h
#define classreg_h
#include <stdio.h>

#if HAVE_IV
#include <InterViews/resource.h>
#else
#define nil 0
#endif

struct Object;

typedef struct Member_func {
	const char* name; double (*member)(void*);
}Member_func;

typedef struct Member_ret_obj_func {
	const char* name; Object** (*member)(void*);
}Member_ret_obj_func;

typedef struct Member_ret_str_func {
	const char* name; const char** (*member)(void*);
}Member_ret_str_func;

extern "C" {
extern char* gargstr(int);
extern double* getarg(int);
extern double  chkarg(int, double min, double max);
extern int ifarg(int);
extern void class2oc(const char*, void* (*cons)(Object*), void (*destruct)(void*),
	Member_func*,
	bool (*checkpoint)(void**) = nil,
	Member_ret_obj_func* = nil,
	Member_ret_str_func* = nil);
}

#endif
