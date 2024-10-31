#ifndef _MKLISP_H_
#define _MKLISP_H_

#define MAX_NAME 32
#define SEXP_LAMBDA 1
#define SEXP_APP 2
#define SEXP_VAR 3
#define SEXP_NUM 4
#define SEXP_INTERNAL 5

typedef struct application {
	struct _sexp* fu;
	struct _sexp* ar;
} app;

typedef struct lambda {
	int va;
	struct _sexp* bo;
} lambda;

typedef struct _sexp {
	int ty;
	union {
		lambda la;
		app ap;
		int va;
		int nu;
		int in;
	};
} sexp;

typedef struct _dict {
	int var;
	sexp exp;
	struct _dict* next;
} Dict;

typedef struct _varstack {
	char name[MAX_NAME];
	int num;
	struct _varstack* next;
} VarStack;



#endif /* _MKLISP_H_ */
