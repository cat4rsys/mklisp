#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "mklisp.h"
#include "parser.h"

#define NEWSEXP() malloc(sizeof(sexp))

int subst(sexp* exp, int v, int sv) {
	static int subvar = -1;
	switch (exp->ty) {
	case SEXP_LAMBDA:
		if (!v) {
			sv = exp->la.va;
			exp->la.va = v = subvar--;
		}
		return subst(exp->la.bo, v, sv);
	case SEXP_APP:
		return subst(exp->ap.fu, v, sv) && subst(exp->ap.ar, v, sv);
	case SEXP_VAR:
		if (exp->va == sv) exp->va = v;
		return 1;
	}
	return 0;
}

int clone(sexp* dst, sexp* src) {
	*dst = *src;
	switch(src->ty) {
	case SEXP_LAMBDA: return clone(dst->la.bo = NEWSEXP(), src->la.bo);
	case SEXP_APP: return clone(dst->ap.fu = NEWSEXP(), src->ap.fu) && clone(dst->ap.ar = NEWSEXP(), src->ap.ar);
	default:
		return 1;
	}
}

int unify(int var, sexp* res, Dict* dict) {
	if ( !dict )  return 1;
	if ( dict->var == var ) {
		tab() && printf("Found var %d: ", var); print_exp(&dict->exp); printf("\n");
		return clone(res, &dict->exp);
	} 
	return unify(var, res, dict->next);
}

int level = 0;

int tab() {
	for ( int k = 0; k < level; k++ ) printf("\t");
	return 1;
}

int reduce(sexp* exp, Dict* dict);

int reduce_app(sexp* exp, Dict* dict) {
	tab() && printf("Reducing application "); print_exp(exp); printf("....\n");
	tab() && printf("Reducing argument\n");
	if (!reduce(exp->ap.ar, dict)) return !printf("Cannot reduce application argument\n"); 
	tab() && printf("Reducing function\n");
	if (!reduce(exp->ap.fu, dict)) return !printf("Cannot reduce application function\n");
	tab() && printf("Reduced to ") && print_exp(exp) && printf("\n");
	switch(exp->ap.fu->ty) {
	case SEXP_LAMBDA:
		//subst(exp->ap.fu, 0, 0);
		tab() && printf("After subst ") && print_exp(exp) && printf("\n");
		Dict d = (Dict){exp->ap.fu->la.va, *exp->ap.ar, dict};
		tab() && printf("Setting var %d to ", exp->ap.fu->la.va); print_exp(exp->ap.ar); printf("\n");
		if(!reduce(exp->ap.fu->la.bo, &d)) return !printf("Cannot reduce body\n");
		*exp =  *exp->ap.fu->la.bo;	
		return 1;
	default:
		return tab() && printf("Unexpected type %d for application\n", exp->ap.fu->ty);
	}
}

int reduce(sexp* exp, Dict* dict) {
	level++;
	tab() && printf("Reducing expression ");
	print_exp(exp);
	printf("....\n");
	switch(exp->ty){
	case SEXP_LAMBDA:
		return reduce(exp->la.bo, dict) && level--;
	case SEXP_VAR:
		return unify(exp->va, exp, dict) && level--;
	case SEXP_APP:
		return reduce_app(exp, dict) && level--;
	default:
		return !printf("Uknown sexp type: %d\n", exp->ty);
	}
}

int print_exp(sexp* exp) {
	if( !exp) return printf("NULL");
	switch(exp->ty) {
	case SEXP_LAMBDA:
		return printf("[%d ", exp->la.va) && print_exp(exp->la.bo) && printf("]");
	case SEXP_APP:
		return printf("(") && print_exp(exp->ap.fu) &&  printf(" ") &&  print_exp(exp->ap.ar) &&  printf(")");
	case SEXP_VAR:
		return printf("%d", exp->va);
	default:
		return printf("<UT%d>", exp->ty);
	}
}

int main(int argc, char* argv[]) {
	if ( argc > 1) {
		char* s = argv[1];
		sexp exp;
		if ( parse_exp(&s, &exp) ) {
			print_exp(&exp) && putchar('\n');
			reduce(&exp, NULL);
			print_exp(&exp) && putchar('\n');
			return 0;
		}
		return -printf("Cannot parse\n");
	} 
	return -printf("Usage: %s \"lambda expression\"\n", argv[0]);

}
