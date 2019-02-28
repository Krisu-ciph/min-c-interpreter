#pragma once

#ifndef _VIRTUAL_MACHINE_H_
#define _VIRTUAL_MACHINE_H_

#include "global_variable.h"
#include <cmath>
using namespace std;

enum {
	I_IMM = 1000, LC, LI, SI, SC,      PUSH, JMP, JZ, JNZ, CALL, ENT, ADJ, LEV, LEA,
	OR, AND, XOR, EQ, NE, LT, LE, GT, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD, POW,
	EXIT, INI, INF, INS, OUTI, OUTF, OUTS, OUTC, OUTS_WT_SP, ASSIGN,
    // adding floating-point operations
    
    LF, F_IMM, SF, F_MUL, F_DIV, F_ADD, F_SUB,
    F_EQ, F_NE, F_LT, F_LE, F_GT, F_GE, F_POW,
    
    S_ADD, S_DEL, S_MUL, S_ASSIGN, // not realize.
    // convertion
    ATOI, ATOF, ITOF, ITOA, FTOI, FTOA, V_PUSH
};

union {
    long long ival;
    double fval;
} ax;
long long offset = 0;

void print_text(long long x);

long long eval() {
	long long op;
	while (true) {
		op = *pc++;
        
//        print_text(op);
//        cout << pc << endl;
        
		if (op == '\0')break;

		/* MOV */
        if (op == I_IMM) { ax.ival = *pc++; }
        else if (op == F_IMM) {
            ax.fval = *(double *)pc++;
        }
        
        // loading true value of address in the register
        else if (op == LC) { ax.ival = *(char *)ax.ival; } // get the value of address in ax and transfer it as char, the previous data in ax is address of a value.
        else if (op == LI) { ax.ival = *(long long *)ax.ival; }
        else if (op == LF) { ax.fval = *(double *)ax.ival; }
        
        // saving value in the address of head of stack
        else if (op == SI)
//            cout << (*sp++) << endl;
            *(long long *)(*ins_sp++) = (long long)ax.ival;
        else if (op == SC)
            *(char *)(*ins_sp++) = (char)ax.ival;
        else if (op == SF) 
            *(double *)(*ins_sp++) = ax.fval;
        
		
		/*PUSH*/
        else if (op == PUSH) { *--ins_sp = ax.ival; }
        else if (op == V_PUSH) { *--var_sp = ax.ival; }
//        else if (op == F_PSH) { *(double *)--sp = ax.fval; }

		/* JMP */
		else if (op == JMP) { pc = (long long *)*pc; }

		/* JZ/JNZ */
		else if (op == JZ) { pc = (ax.ival)? pc + 1 : (long long *)*pc; }
        else if (op == JNZ) { pc = (ax.ival) ? (long long *)*pc : pc + 1; }

		/* CALL */
		else if (op == CALL) {
            *--ins_sp = (long long)(pc + 1);
            pc = (long long *)*pc;
        }

		/* ENT */
		else if (op == ENT) {
			*--ins_sp = (long long)ins_bp;
            ins_bp = ins_sp;

			*--var_sp = (long long)var_bp;
            var_bp = var_sp;
		}

		/* ADJ */
		else if (op == ADJ) {
            var_sp = var_sp + *pc++;
        }

		/* LEV */ // leave
		else if (op == LEV) {
			ins_sp = ins_bp;
            ins_bp = (long long *)*ins_sp++;
            pc = (long long *)*ins_sp++;

			var_sp = var_bp;
            var_bp = (long long *)*var_sp++;
		}

		/* LEA */ // load address for arguments.
		else if (op == LEA) {
            ax.ival = (long long)(var_bp + *pc++ - 1);
        }

		/* operators */
        // calculate the value in the address of head of stack with value in ax
        // and store in ax.
		else if (op == OR)  ax.ival = *ins_sp++ | ax.ival;
		else if (op == AND) ax.ival = *ins_sp++ & ax.ival;
        else if (op == XOR) ax.ival = *ins_sp++ ^ ax.ival;

        
//		else if (op == ASSIGN) *pc++ = (int)ax.ival;

		else if (op == EQ) ax.ival = *ins_sp++ == ax.ival;
        else if (op == NE) ax.ival = *ins_sp++ != ax.ival;
        else if (op == LT) ax.ival = *ins_sp++ <  ax.ival;
        else if (op == LE) ax.ival = *ins_sp++ <= ax.ival;
		else if (op == GT) ax.ival = *ins_sp++ >  ax.ival;
		else if (op == GE) ax.ival = *ins_sp++ >= ax.ival;
        
		else if (op == ADD) ax.ival = *ins_sp++ + ax.ival;
		else if (op == SUB) ax.ival = *ins_sp++ - ax.ival;
		else if (op == MUL) ax.ival = *ins_sp++ * ax.ival;
		else if (op == DIV) ax.ival = *ins_sp++ / ax.ival;
		else if (op == MOD) ax.ival = *ins_sp++ % ax.ival;
        ////
        else if (op == POW) {
            long long a = *ins_sp++, k = ax.ival, ret = 1;
            while (k) {
                if (k % 2)
                    ret *= a;
                a *= a;
                k /= 2;
            }
            ax.ival = ret;
        }
        ////
        else if (op == SHL) ax.ival = *ins_sp++ << ax.ival;
        else if (op == SHR) ax.ival = *ins_sp++ >> ax.ival;
        
        
        else if (op == F_ADD)
            ax.fval = *(double *)ins_sp++ + ax.fval;
        else if (op == F_SUB)
            ax.fval = *(double *)ins_sp++ - ax.fval;
        else if (op == F_MUL)
            ax.fval = *(double *)ins_sp++ * ax.fval;
        else if (op == F_DIV)
            ax.fval = *(double *)ins_sp++ / ax.fval;
        ////
        else if (op == F_POW)
            ax.fval = pow(*(double *)ins_sp++, ax.fval);
        ////
        else if (op == F_EQ) ax.ival = *(double *)ins_sp++ == ax.fval;
        else if (op == F_NE) ax.ival = *(double *)ins_sp++ != ax.fval;
        else if (op == F_LT) ax.ival = *(double *)ins_sp++ <  ax.fval;
        else if (op == F_LE) ax.ival = *(double *)ins_sp++ <= ax.fval;
        else if (op == F_GT) ax.ival = *(double *)ins_sp++ >  ax.fval;
        else if (op == F_GE) ax.ival = *(double *)ins_sp++ >= ax.fval;
        
        else if (op == S_ADD) {
            char *s = (char *)*ins_sp++, *t = (char *)ax.ival;
            char *ans = data_;
            while ((*data_++ = *s++)) ;
            data_--;
            while ((*data_++ = *t++)) ;
            *data_++ = '\0';
            ax.ival = (long long)ans;
        }
        else if (op == S_DEL) {
            char *s = (char *)*ins_sp++;
            int ip = (int)ax.ival;
            for (char* pos = s + ip; ; pos++) {
                swap(*pos, *(pos + 1));
                if (!*pos)
                    break;
            }
            ax.ival = (long long)s;
        }
        else if (op == S_MUL) {
            char *old_s = (char *)*ins_sp++;
            int times = (int)ax.ival;
            char *ans = data_;
            while (times--) {
                char *s = old_s;
                while (((*data_++ = *s++))) ;
                data_--;
            }
            *++data_ = '\0';
            ++data_;
            ax.ival = (long long)ans;
        }
        else if (op == S_ASSIGN) {
            long long *lval = (long long *)*ins_sp++;
            char *rval = (char *)ax.ival;
            *lval = (long long)data_;
            while ((*data_++ = *rval++)) ;
            *data_++ = '\0';
        }

		/* system operations */
		else if (op == EXIT) { /*printf("exit(%lld)", *sp);*/ return *ins_sp; }
        
		else if (op == OUTI) printf("%lld ", ax.ival);
		else if (op == OUTF) printf("%lf ", ax.fval);
		else if (op == OUTS) printf("%s ", (char *)ax.ival);
        else if (op == OUTC) printf("%c", (char)ax.ival);
        else if (op == OUTS_WT_SP) printf("%s", (char *)ax.ival);
        
		else if (op == INI) scanf("%lld", (long long *)ax.ival);
        else if (op == INF) scanf("%lf", (double *)ax.ival);
        // string has some problem.
        else if (op == INS) {
            long long * v_pos = (long long *)ax.ival;
            char *ans = data_;
            scanf("%s", data_);
            while (*data_ != '\0')
                data_++;
            *data_++ = '\0';
            *v_pos = (long long)ans;
        }
        
        // convertion
        else if (op == ATOI) {
            char * s = (char *)ax.ival;
            for (int i = 0; s[i] != '\0'; i++) {
                if (!isdigit(s[i])) {
                    printf("Invalid convertion from string to int.\n");
                    exit(-1);
                }
            }
            ax.ival = atoi(s);
        }
        else if (op == ATOF) {
            char * s = (char *)ax.ival;
            int num_dot = 0;
            for (int i = 0; s[i] != '\0'; i++) {
                if (s[i] == '.')
                    num_dot++;
                else if (!isdigit(s[i]) || num_dot > 2) {
                    printf("Invalid convertion from string to real.\n");
                    exit(-1);
                }
            }
            ax.fval = atof(s);
        }
        else if (op == ITOF){
            ax.fval = (double)ax.ival;
        }
        else if (op == ITOA) {
            char * ans = data_;
            sprintf(data_, "%lld", ax.ival);
            while (*data_++ != 0) ;
            ax.ival = (long long)ans;
        }
        else if (op == FTOI) {
            ax.ival = (long long)ax.fval;
        }
        else if (op == FTOA) {
            char * ans = data_;
            sprintf(data_, "%lf", ax.fval);
            while (*data_++ != 0) ;
            ax.ival = (long long)ans;
        }
	}
	return 0;
}

void virtual_machine_initializer() {
    size_t mem_size = 64 * 1024 * 1024;
    if (!(data_ = (char *)malloc(mem_size))) {
        printf("could not malloc(%lu) for data area\n", mem_size);
        exit(-1);
    }
	if (!(text = old_text = (long long *)malloc(mem_size * sizeof(long long)))) {
		printf("could not malloc(%lu) for text area\n",mem_size);
		exit(-1);
	}
	if (!(ins_stack = (long long *)malloc(mem_size * sizeof(long long)))) {
		printf("could not malloc(%lu) for instrument stack area\n", mem_size);
		exit(-1);
	}
	if (!(var_stack = (long long *)malloc(mem_size * sizeof(long long)))) {
		printf("could not malloc(%lu) for variable stack area\n", mem_size);
		exit(-1);
	}
	memset(text, 0, mem_size * sizeof(long long));
    memset(data_, 0, mem_size);
	memset(ins_stack, 0, mem_size * sizeof(long long));
	memset(var_stack, 0, mem_size * sizeof(long long));
	ins_bp = ins_sp = (long long *)((long long)ins_stack + (mem_size - 1) * sizeof(long long));
	var_bp = var_sp = (long long *)((long long)var_stack + (mem_size - 1) * sizeof(long long));

    ax.ival = 0, ax.fval = 0;
    pc = NULL;
}

//test part.
const string str[] = {
    "I_IMM", "LC", "LI", "SI", "SC",      "PUSH", "JMP", "JZ", "JNZ", "CALL", "ENT", "ADJ", "LEV", "LEA",
    "OR", "AND", "XOR", "EQ", "NE", "LT", "LE", "GT", "GE", "SHL", "SHR", "ADD", "SUB", "MUL", "DIV", "MOD", "POW",
    "EXIT", "INI", "INF", "INS", "OUTI", "OUTF", "OUTS", "OUTC", "OUTS_WT_SP", "ASSIGN",
    // adding floating-point operations
    
    "LF", "F_IMM", "SF", "F_MUL", "F_DIV", "F_ADD", "F_SUB", "F_POW",

    "F_EQ", "F_NE", "F_LT", "F_LE", "F_GT", "F_GE",
    "S_ADD", "S_DEL", "S_MUL", "S_ASSIGN",
    // convertion
    "ATOI", "ATOF", "ITOF", "ITOA", "FTOI", "FTOA", "V_PUSH"
};

void print_text(long long x) {
    if (I_IMM <= x && x <= V_PUSH) {
        cout << str[x - 1000] << endl;
    }
    else {
        cout << x << endl;
    }
}

#endif
