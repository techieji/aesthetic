#include "a.h"

// Preprocessor
// Doesn't really take directives, but rather expands 'a into (quote a) and such.
// Here's the list of things it will expand:
// 	'a into (quote a)
// 	`a into (quasiquote a)
// 	,a into (unquote a)

char* append_str(char* inp, char* next, int n) {
	// printf("%.*s\n", n, next);
	int inp_len = strlen(inp), next_len = strlen(next);
	if (n <= inp_len) {
		strncpy(inp, next, n);
		return inp + n;
	} else {
		puts("Realloc!");
		char* ret = realloc(inp, inp_len + next_len + PPBUF_SIZE);
		strncpy(ret, next, n);
		return ret + n;
	}
}

char* append_lit(char* inp, char* next) {
	return append_str(inp, next, strlen(next));
}

char* append_char(char* inp, char next) {
	char* dummy = malloc(2);
	dummy[0] = next;
	dummy[1] = '\0';
	return append_lit(inp, dummy);
}

char* preprocess_str(char* string) {
	char* realoutput = malloc(strlen(string) * sizeof(char) + PPBUF_SIZE);
	char* output = realoutput;
	for (; *string != '\0'; string++) {
		if (*string == '\'' || *string == '`' || *string == ',') {
			int level = 1;
			char c;
			int offset;
			for (offset = 0; (c = *(string + offset)) != '\0'; offset++) {
				if (c == '(') level++;
				if (c == ')') level--;
				if (!level) break;
			}
			switch (*string) {
				case '\'': output = append_lit(output, "(quote "); break;
				case '`': output = append_lit(output, "(quasiquote "); break;
				case ',': output = append_lit(output, "(unquote "); break;
			}
			output = append_str(output, ++string, --offset);
			output = append_char(output, ')');
		} else
			output = append_char(output, *string);
		puts(realoutput);
	}
	return realoutput;
}
