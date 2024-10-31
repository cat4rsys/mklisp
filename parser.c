#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"

int skip_space(char** string) {
	if (**string == '\n' || **string == '\t' || **string == ' ') {
		(*string)++;
		return skip_space(string);
	} 
	return 1;
}

int parse_const(char **string, char c) {
	if ( **string == c ) {
		(*string)++;
		return 1;
	} 
	return 0;
}

VarStack* stack = NULL;

int push_var(char* name) {
	static int num = 0;
	VarStack* s  = malloc(sizeof(VarStack));
	strcpy(s->name, name);
	s->num = ++num;
	s->next = stack;
	stack = s;
	return s->num;
}

void pop_var() {
	if (stack) 
		stack = ({VarStack* n = stack->next; free(stack); n;});
}

int _find_var(VarStack* s, char* name, int* var) {
	if ( !s ) return 0;
	if ( !strcmp(name, s->name) ) { *var = s->num; return 1; }
	return _find_var(s->next, name, var);
}
int find_var(char* name, int* var) { return _find_var(stack, name, var); }


int parse_new_var(char** string, int* var) {
	char varname[MAX_NAME];
	int advance;
	if ( sscanf(*string, "%[a-zA-Z]%n", varname, &advance) == 1) {
		*string  += advance;
		*var = push_var(varname);
		return 1;
	}
	return 0;
}

int parse_var(char** string, sexp* expr) {
	char varname[MAX_NAME];
	int advance;
	int var;
	if (sscanf(*string, "%[a-zA-Z]%n", varname, &advance)  == 1) {
		if (find_var(varname, &var)) {
			*expr = (sexp){.ty=SEXP_VAR, .va=var};
			*string += advance;
			return 1;
		}
		return !printf("Expected existing variable name, but no outer lambda found with definition of %s at %p\n", varname, *string);
	}
	return 0;
}

int parse_lambda(char** string, sexp* expr) {
	char* s = *string;
	if ( parse_const(string, '[') && skip_space(string) ) {
		sexp* bo = malloc(sizeof(sexp));
		*expr = (sexp){.ty = SEXP_LAMBDA, .la=(lambda){0, bo}};
		if ( parse_new_var(string, &expr->la.va) 
				&& skip_space(string) 
				&& parse_exp(string, bo) 
				&& skip_space(string)
				&& parse_const(string, ']') ) {
			pop_var();
			return 1;
		}
		free(bo);
		printf("Expected lambda expression at %p\n", *string);
	} 
	*string = s;
	return 0;
}

int parse_app(char** string, sexp* expr) {
	char* s = *string;
	if ( parse_const(string, '(') && skip_space(string)) {
		sexp *fu = malloc(sizeof(sexp)), *ar = malloc(sizeof(sexp));
		*expr = (sexp){.ty=SEXP_APP, .ap=(app){fu, ar}};
		if ( parse_exp(string, fu) && skip_space(string) && parse_exp(string, ar) && skip_space(string) && parse_const(string, ')')) 
			return 1;
		free(fu);
		free(ar);
		printf("Expected application at %p\n", *string);
	}
	*string = s;
	return 0;
}

int parse_exp(char** string, sexp* exp) {
	return skip_space(string) && (parse_lambda(string, exp) || parse_app(string, exp) || parse_var(string, exp));
}


