#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>

/* Windows */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = '\0';
	return cpy;
}

void add_history(char* unused) {}

/* otherwise */
#else
#include <editline/readline.h>
#include <editline/history.h>
#endif


long eval_op(long x, char* op, long y) {
	if(strcmp(op, "+") == 0) { return x + y; }
	if(strcmp(op, "-") == 0) { return x - y; }
	if(strcmp(op, "*") == 0) { return x * y; }
	if(strcmp(op, "/") == 0) { return x / y; }
	if(strcmp(op, "%") == 0) { return x % y; }	
	if(strcmp(op, "^") == 0) { return (int)pow(x,y); }
	return 0;
}

long eval(mpc_ast_t* t) {
	if(strstr(t->tag, "number")) {
		return atoi(t->contents);
	}

	char* op = t->children[1]->contents;

	long x = eval(t->children[2]);

	int i = 3;

	while(strstr(t->children[i]->tag, "expr")) {
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}


	return x;
}

int calculate_leaves(mpc_ast_t* t) {

	if(strstr(t->tag, "number")) {
		return 1;
	}


	int sum_of_nodes = 0;

	for(int i = 0; i < t->children_num; i++) {
		sum_of_nodes += calculate_leaves(t->children[i]);
	}

	return sum_of_nodes;
}

int calculate_children(mpc_ast_t* t) {
	if(t->children_num == 0) {
		return 1;
	}

	int max_depth = 0;

	for(int i = 0; i < t->children_num; i++) {
		int depth = calculate_children(t->children[i]);
		if(depth > max_depth) {
			max_depth = depth;
		}	
	}
	
	return max_depth + 1;
}

int main(int argc, char** argv) {
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	/* Define the language */
	mpca_lang(MPCA_LANG_DEFAULT,
			"	\
			number : /-?[0-9]+/ ; \
			operator : '+' | '-' | '*' | '/' | '%' | '^' ; \
			expr : <number> | '(' <operator> <expr>+ ')'  ; \
			lispy : /^/ <operator> <expr>+ /$/ ; \
			",

			Number, Operator, Expr, Lispy);


	while(1) {
		char* input = readline("lispy> ");

		add_history(input);

		mpc_result_t r;
		// mpc_ast_t* ast = r.output;
		if(mpc_parse("<stdin>", input, Lispy, &r)) {
			// mpc_ast_print(r.output);
			// mpc_ast_delete(r.output);

			long result = eval(r.output);
			// int num_of_leaves = calculate_leaves(r.output);
			// int  most_children = calculate_children(r.output);

			printf("%li\n", result);
			//printf("%i\n", num_of_leaves);
			//printf("%i\n", most_children);
			mpc_ast_delete(r.output);
		} else  {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}
	
	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;

}
