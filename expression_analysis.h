#ifndef expression_analysis_h
#define expression_analysis_h

#include "global_variable.h"
#include "lexical_analysis.h"
#include "syntax_analysis.h"

enum { STRING = String, INT, REAL, CHAR, VOID, PTR };

void expression(int level) { // level is the priority of operators.
    
    // testing.
    if (token == ';') {
        return ;
    }
    // testing.
    if (token == 0) {
        printf("%d unexpected token EOF of expression\n", line);
        exit(-1);
    }
    
    // deal with constant.
    if (token == Inum) {
        match(Inum);
        
        // assemble
        *++text = I_IMM;
        *++text = token_val;
        expr_type = INT;
    }
    else if (token == Fnum) {
        match(Fnum);
        
        // assemble
        *++text = F_IMM;
        *(double *)++text = ftoken_val;
        expr_type = REAL;
    }
    else if (token == Cstr) {
        match(Cstr);
        // assemble
        *++text = I_IMM;
        *++text = token_val;
        // ??? need match?
        expr_type = STRING;
    }
    
    // deal with identifier.
    else if (token == Id) {
        // 1. function calling.
        // 2. variable.
        match(Id);
        
        identifier * t_id = cur_id;
        
        if (token == '(') { // is a function.
            
            // adding part.
			identifier tmp = *t_id;
            
            tmp.p_list = (int *)malloc(sizeof(int) * 23);
            memset(tmp.p_list, 0, sizeof(int) * 23);
            tmp.p_hash = tmp.p_size = 0;
            // end.
            
            int num_args = 0;
            *++text = ADJ;
            *++text = -(params + 1);
            
            match('(');
            while (token != ')') {
                expression(Assign); // current
                // Assign has the lowest priority.
                // this part deal with parameters.
                
                tmp.p_list[tmp.p_size++] = expr_type;
//                tmp_func->para_list.push_back(expr_type);
                tmp.p_hash = tmp.p_hash + expr_type * (num_args + 1) * 17;
                
                *++text = V_PUSH;
                num_args++;
                
                if (token == ',') {
                    match(',');
                }
            }
            match(')');
        
            // adding finding same function in here.
            for (identifier * it = ed[depth]; it >= id_list; --it) {
                if (it->hash == tmp.hash && tmp.name == it->name) {
                    // function over-loading.
                    if (it->class_ == Fun && it->p_hash == tmp.p_hash && it->p_size == tmp.p_size && memcmp(it->p_list, tmp.p_list, it->p_size) == 0 ) {
                        // find the same function.
                        t_id = it;
                        break;
                    }
                    
                }
            }
            
            if (t_id->class_ == Fun) {
                *++text = CALL;
//				*++text = params + 1;
                *++text = t_id->value;
            }
            else {
//                因为tmp没有在idlist里找到和他相同的函数，所以到达了这里
                
                printf("%d: bad function calling\n", line);
                exit(-1);
            }
            
            // clean the stack for arguments.
//            if (num_args > 0) { // if argument is 'void', no need to make spaces in stack for variable
//                *++text = ADJ;
//                *++text = num_args; // number of arguments.
//            }
            expr_type = t_id->type;
        }
        else {
            // variable part.
            if (t_id->class_ == Loc) {
                *++text = LEA;
                *++text = index_of_var_bp - t_id->value;
                // local variable, look into stack and load the address of it into ax(register)
                // both int, real, string..? work with this.
            }
            else if (t_id->class_ == Glo) {
                *++text = I_IMM;
                *++text = t_id->value;
                // globle variable, read the address of it into ax(register)
            }
            else {
				t_id->class_ = Loc;
				t_id->value = ++local_pos;

				*++text = LEA;
				*++text = index_of_var_bp - t_id->value;
            }
            // load ax into head of stack.
            expr_type = t_id->type;
            if (expr_type == INT)
                *++text = LI;
            else if (expr_type == STRING) {
                *++text = LI;
            }
            else if (expr_type == REAL)
                *++text = LF;
            else {
                printf("%d: undefined variable\n", line);
                exit(-1);
            }
            // what about string?
        }
    }
    else if (token == '(') {
        // not dealing with type-casting yet.
        match('(');
        if (token == Int || token == String || token == Real) {
            int to_type = token; // INT = Int
            match(token);
            // token is the type.
            match(')');

            expression(Inc);
            // now token is Id. and var is in ax.
            if (expr_type == INT && to_type == STRING)
                *++text = ITOA;
            else if (expr_type == INT && to_type == REAL)
                *++text = ITOF;
            else if (expr_type == REAL && to_type == STRING)
                *++text = FTOA;
            else if (expr_type == REAL && to_type == INT)
                *++text = FTOI;
            else if (expr_type == STRING && to_type == INT)
                *++text = ATOI;
            else if (expr_type == STRING && to_type == REAL)
                *++text = ATOF;
            expr_type = to_type;
        }
        // normal parenthesis
        else {
            expression(Assign);
            match(')');
        }
    }
    else if (token == Mul) {
        // *<addr>
        match(Mul);
        expression(Inc); // high priority of Inc
        
        if (expr_type >= PTR) {
            expr_type = expr_type - PTR;
            if (expr_type == REAL)
                *++text = LF;
            else if (expr_type == INT)
                *++text = LI;
            // what about string, again???
            // string has no *str?
        }
        else {
            printf("%d: bad dereference\n", line);
            exit(-1);
        }
        expr_type = expr_type + PTR; // return to prefer.
    }
    else if (token == '!') {
        match('!');
        expression(Inc);
        
        // assemble
        *++text = PUSH;
        *++text = I_IMM;
        *++text = 0;
        *++text = EQ;
        // see if value in ax is 0
        // ax == 0?  1 : 0
        // push the value in ax into stack, compare it with 0 in ax
    }
    else if (token == '~') {
        match('~');
        expression(Inc);
        
        // assemble
        *++text = PUSH;
        *++text = I_IMM;
        *++text = -1;
        *++text = XOR;
        
        expr_type = INT;
    }
    else if (token == Add) {
        match(Add);
        expression(Inc);
        
        expr_type = INT;
    }
    else if (token == Sub) {
        // -var
        match(Sub);
        if (token == Inum) { // constant number
            *++text = I_IMM;
            *++text = -token_val;
            match(Inum);
            // change *token_val into -*token_val, and push the address into stack.
            expr_type = INT;
        }
        else if (token == Fnum) {
            ftoken_val = -ftoken_val;
            
            *++text = F_IMM;
            *(double *)++text = ftoken_val;
            match(Fnum);
            // save -num into ax.
            expr_type = REAL;
        }
        else if (token == Id && cur_id->type == INT) { // int variable.
            *++text = I_IMM;
            *++text = -1;
            *++text = PUSH;
            expression(Inc);
            *++text = MUL;
            expr_type = INT;
        }
        else if (token == Id && cur_id->type == REAL) {
            *++text = F_IMM;
            *(double *)++text = -1.0;
            *++text = PUSH;
            expression(Inc);
            *++text = F_MUL;
            expr_type = REAL;
        }
        else if (token == '(') {// -(...)
            expression(Inc);
            if (expr_type == INT) {
                *++text = PUSH;
                *++text = I_IMM;
                *++text = -1;
                *++text = MUL;
                expr_type = INT;
            }
            else if (expr_type == REAL) {
                *++text = PUSH;
                *++text = F_IMM;
                *(double *)++text = -1.0;
                *++text = F_MUL;
                expr_type = REAL;
            }
        }
        else {
            printf("%d: invalid use of operator \'-\'\n", line);
            exit(-1);
        }
    }
    else if (token == Inc || token == Dec) { /// ????
        int tok = token;
        match(token);
        expression(Inc);
        if (*text == LI) {
            *text = PUSH;
            *++text = LI;
        }
        else {
            printf("%d: bad left-value of pre-increment\n", line);
            exit(-1);
        }
        
        *++text = PUSH;
        *++text = I_IMM;
        if (expr_type == REAL + PTR)
            *++text = sizeof(double);
        else if (expr_type == PTR + CHAR)
            *++text = sizeof(char);
        else if (expr_type > PTR)
            *++text = sizeof(int);
        else
            *++text = 1;
        *++text = (tok == Inc? ADD : SUB);
        *++text = SI;
        // needed to be changed.
    }
    
    /////
    ///
    // adding IO-part.
    else if (token == Out) {
        // Out the result of the expression.
        match(Out);
        // now token is another expression.
        expression(Assign);
        while (true) {
            if (expr_type == INT)
                *++text = OUTI;
            else if (expr_type == REAL)
                *++text = OUTF;
            else if (expr_type == STRING) {
                // some bug may occur here......
                // emmmmmmm...
//                if (*text == LI)
                    *++text = OUTS;
//                else
//                    *++text = OUTS;
            }
            if (token == ';') {
                *++text = I_IMM;
                *++text = '\n';
                *++text = OUTC;
                break; // end
            }
            else if (token == ',') {
                match(',');
                // now token is another expression.
                expression(Assign);
            }
            else {
                printf("%d: unexpected token in \"out\" operation.\n", line);
                exit(-1);
            }
        }
    }
    else if (token == In) {
        // input:   in var;
        match(In);
        if (token != Id && token != '(') {
            printf("%d: syntax error!\n", line);
            exit(-1);
        }
        else if (token == '(') {
            match('(');
            expression(Assign);
            if (expr_type != STRING) {
                printf("%d: invalid use of in\n", line);
                exit(-1);
            }
            *++text = OUTS_WT_SP;
            match(')');
        }
        expression(Assign);
        // the address of variable is now in ax.
        while (true) {
            if (expr_type == INT)
                *text = INI;
            else if (expr_type == REAL)
                *text = INF;
            else if (expr_type == STRING) {
                *text = INS;
            }
            else {
                printf("%d: invalid left-value\n", line);
                exit(-1);
            }
            
            if (token == ';')
                break;
            else if (token == ',') {
                match(',');
                expression(Assign);
            }
            else {
                printf("%d: unexpected token in \"in\"  operation.\n", line);
                exit(-1);
            }
        }
    }
    //
    ///
    ////
    
    else {
        printf("%d: bad expression\n", line);
        exit(-1);
    }
    long long *addr;
    while (token >= level) {
        int cur_ex_type = expr_type;
        ////////////////////
        //     ASSIGN     //
        ////////////////////
        if (token == Assign) {
            match(Assign);
            if (expr_type == INT || expr_type == REAL) {
                if (*text == LI || *text == LF) {
                    *text = PUSH; // save the lvalue's pointer(address)
                }
                else {
                    printf("%d: bad left-value in assignment\n", line);
                    exit(-1);
                }
                expression(Assign);
                
                expr_type = cur_ex_type;
                if (expr_type == REAL)
                    *++text = SF;
                else if (expr_type == INT || expr_type == STRING)
                    *++text = SI;
            }
            else if (expr_type == STRING) {
                if (*text == LI || *text == LF) {
                    *text = PUSH; // save the lvalue's pointer(address)
                }
                expression(Assign);
                *++text = S_ASSIGN;
                // now must be a string address in ax.
                
            }
        }
        
        // ????
        else if (token == Or) {
            match(Or);
            *++text = JNZ;
            addr = ++text;
            expression(And);
            *addr = (long long)(text + 1);
            expr_type = INT;
        }
        else if (token == And) {
            match(And);
            *++text = JZ;
            addr = ++text;
            expression(Bor);
            *addr = (long long)(text + 1);
            expr_type = INT;
        }
        else if (token == Band) {
            // bitwise and
            match(Band);
            *++text = PUSH;
            expression(Eq);
            *++text = AND;
            expr_type = INT;
        }
        else if (token == Bor) {
            // bitwise or
            match(Bor);
            *++text = PUSH;
            expression(Xor);
            *++text = OR;
            expr_type = INT;
        }

        
        else if (token == Xor) {
            match(Xor);
            *++text = PUSH;
            expression(Band);
            *++text = XOR;
            expr_type = INT;
        }
        else if (token == Band) {
            match(Band);
            *++text = PUSH;
            expression(Eq);
            *++text = AND;
            expr_type = INT;
        }
        
        // == , != , < , <= , > , >=
        else if (token == Eq) {
            match(Eq);
            *++text = PUSH;
            expression(Ne);
            *++text = cur_ex_type == INT? EQ : F_EQ;
            expr_type = INT;
        }
        else if (token == Ne) {
            match(Ne);
            *++text = PUSH;
            expression(Lt);
            *++text = cur_ex_type == INT? NE : F_NE;
            expr_type = INT;
        }
        else if (token == Lt) {
            match(Lt);
            *++text = PUSH;
            expression(Shl);
            *++text = cur_ex_type == INT? LT : F_LT;
            expr_type = INT;
        }
        else if (token == Gt) {
            match(Gt);
            *++text = PUSH;
            expression(Shl);
            *++text = cur_ex_type == INT? GT : F_GT;
            expr_type = INT;
        }
        else if (token == Le) {
            match(Le);
            *++text = PUSH;
            expression(Shl);
            *++text = cur_ex_type == INT? LE : F_LE;
            expr_type = INT;
        }
        else if (token == Ge) {
            match(Ge);
            *++text = PUSH;
            expression(Shl);
            *++text = cur_ex_type == INT? GE : F_GE;
            expr_type = INT;
        }
        else if (token == Shl) {
            match(Shl);
            *++text = PUSH;
            expression(Add);
            *++text = SHL;
            expr_type = INT;
        }
        else if (token == Shr) {
            match(Shr);
            *++text = PUSH;
            expression(Add);
            *++text = SHR;
            expr_type = INT;
        }
        
        
        // '+' '-' '*' '/'
        else if (token == Add) {
            match(Add);
            *++text = PUSH;
            expression(Mul);
            if (cur_ex_type > PTR && cur_ex_type == expr_type) {
                // pointer subtraction
                *++text = Add;
                *++text = PUSH;
                *++text = I_IMM;
                *++text = sizeof(int);
                *++text = DIV;
                expr_type = INT;
            }
            else if (cur_ex_type > PTR && expr_type == INT) {
                // pointer movement
                *++text = PUSH;
                *++text = I_IMM;
                *++text = sizeof(int);
                *++text = MUL;
                *++text = ADD;
                expr_type = cur_ex_type;
            }
            else if (cur_ex_type == expr_type && expr_type == INT) {
                // interger
                *++text = ADD;
                expr_type = INT;
            }
            else if (cur_ex_type == expr_type && expr_type == REAL) {
                // floating-point.
                *++text = F_ADD;
                expr_type = REAL;
            }
            else {
                printf("%d: Error! Can not operate on different numeric type (yet).\n", line);
                exit(-1);
            }
        }
        else if (token == Sub) {
            match(Sub);
            *++text = PUSH;
            expression(Mul);
            if (cur_ex_type > PTR && cur_ex_type == expr_type) {
                // pointer subtraction
                *++text = SUB;
                *++text = PUSH;
                *++text = I_IMM;
                *++text = sizeof(int);
                *++text = DIV;
                expr_type = INT;
            }
            else if (cur_ex_type > PTR && expr_type == INT) {
                // pointer movement
                *++text = PUSH;
                *++text = I_IMM;
                *++text = sizeof(int);
                *++text = MUL;
                *++text = SUB;
                expr_type = cur_ex_type;
            }
            else if (cur_ex_type == expr_type && expr_type == INT) {
                *++text = SUB;
                expr_type = INT;
            }
            else if (cur_ex_type == expr_type && expr_type == REAL) {
                *++text = F_SUB;
                expr_type = REAL;
            }
            else {
                printf("%d: Error! Can not operate on different numeric type (yet).\n", line);
                exit(-1);
            }
        }
        else if (token == Mul) {
            // multiply~~~~~~~
            // adding string multiplying
            if (expr_type == STRING) {
                match(Mul);
                *++text = PUSH;
                expression(Bor); // low priority.
                if (expr_type != INT) {
                    printf("%d: invalid string multiplication.\n", line);
                    exit(-1);
                }
                *++text = S_MUL;
                expr_type = STRING;
            }
            // end adding.
            
            else { // normal numeric multiplying
                match(Mul);
                *++text = PUSH;
                expression(Inc);
                if (expr_type == INT) {
                    *++text = MUL;
                    expr_type = INT;
                }
                else if (expr_type == REAL) {
                    *++text = F_MUL;
                    expr_type = REAL;
                }
            }
        }
        else if (token == Div) {
            // multiply~~~~~~~
            match(Div);
            *++text = PUSH;
            expression(Inc);
            if (expr_type == INT) {
                *++text = DIV;
                expr_type = INT;
            }
            else if (expr_type == REAL) {
                *++text = F_DIV;
                expr_type = REAL;
            }
        }
        else if (token == Mod) {
            match(Mod);
            *++text = PUSH;
            expression(Inc);
            *++text = MOD;
            expr_type = cur_ex_type;
        }
        else if (token == Pow) {
            int cur_ex_type = expr_type;
            match(Pow);
            // now value in ax.
            *++text = PUSH;
            expression(Inc);
            if (expr_type == INT && cur_ex_type == expr_type)
                *++text = POW;
            else if (expr_type == REAL && cur_ex_type == expr_type)
                *++text = F_POW;
            else {
                printf("%d: syntax error\n", line);
                exit(-1);
            }
        }
        
        // string operations.
        ////
        //////
        else if (token == Str_add) {
            if (expr_type != String) {
                printf("%d: invalid first parameter of string  addition\n", line);
                exit(-1);
            }
            match(Str_add);
            if (expr_type != String) {
                printf("%d: invalid second parameter of string  addition\n", line);
                exit(-1);
            }
            *++text = PUSH;
            expression(Bor);
            *++text = S_ADD;
            expr_type = String;
        }
        else if (token == Str_del) {
            if (expr_type != String) {
                printf("%d: invalid first parameter of string deletion\n", line);
                exit(-1);
            }
            match(Str_del);
            *++text = PUSH;
            expression(Bor);
            if (expr_type != INT) {
                printf("%d: invalid second parameter of string deletion\n", line);
                exit(-1);
            }
            *++text = S_DEL;
            expr_type = String;
        }
        //////
        ////
        // new adding
        
        else if (token == Inc || token == Dec) {
            // postfix ++ and --
            // increase and decrease it to get prefer value on ax.
            if (*text == LI) {
                *text = PUSH;
                *++text = LI;
            }
            else {
                printf("%d: bad value in increment\n", line);
                exit(-1);
            }
            
            *++text = PUSH;
            *++text = I_IMM;
            *++text = (expr_type > PTR? sizeof(int) : 1);
            *++text = (token == Inc? ADD : SUB);
            *++text = SI;
            *++text = PUSH;
            *++text = I_IMM;
            *++text = (expr_type > PTR? sizeof(int) : 1);
            *++text = (token == Inc? SUB : ADD);
            match(token);
        }
        else if (token == Brak) {
            match(Brak);
            *++text = PUSH;
            expression(Assign);
            match(']');
            
            if (cur_ex_type > PTR) {
                // pinter
                *++text = PUSH;
                *++text = I_IMM;
                *++text = sizeof(int);
                *++text = MUL;
            }
            else if (cur_ex_type < PTR) {
                printf("%d: pointer type expected\n", line);
                exit(-1);
            }
            expr_type = cur_ex_type - PTR;
            *++text = ADD;
            *++text = LI;
        }
        else {
            printf("%d: complation error, unexpected token of %d\n", line, token);
            exit(-1);
        }
    }
}

#endif
