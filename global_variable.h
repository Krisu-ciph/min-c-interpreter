#ifndef global_variable_h
#define global_variable_h

#include <vector>

extern int token = 0, line = 0;
extern long long token_val = 0;
extern double ftoken_val = 0;

extern char *src = NULL, *old_src = NULL, *data_ = NULL;

extern long long *text = NULL, *old_text = NULL;
extern long long *pc = NULL, cycle = 0;

extern long long *var_bp = NULL, *var_sp = NULL, *var_stack = NULL;
extern long long *ins_bp = NULL, *ins_sp = NULL, *ins_stack = NULL;

extern long long local_pos = 0, index_of_var_bp = 0;

extern long long params = 0;


extern int expr_type = 0;

#endif /* global_variable_h */
