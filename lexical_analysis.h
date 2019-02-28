#ifndef lexical_analysis_h
#define lexical_analysis_h

#include "lexical_analysis.h"
#include "global_variable.h"

#include <iostream>
#include <cctype>
#include <vector>
using namespace std;

// "string int real else if return while "
// matching the second line of enum (for initializing)
// "in out void main"
// matching the third line

// token list.
enum {
    // integer constant, floating point constant, function, system operation, identifier
    Inum = 128, Fnum, Cstr, Fun, Sys, Glo, Loc, Id,
    String, Int, Real, Else, If, Return, While, For, Do, Until, Break, Continue, 
    In, Out, Void, Main,
    // the former are pre-defined identifier
    Assign,
    Cond,
    Str_add, Str_del,
    Bor, Band,
    Xor, Or, And,
    Eq, Ne, Lt, Gt, Le, Ge,
    Shl, Shr,
    Add, Sub,
    Mul, Div, Mod,
    Pow,
    Inc, Dec,
    Brak
    // these are operators
};

struct identifier {
    int token;
    int hash; // the hash value of token
    string name; // the name of identifier
    
    // later processing
    int class_; // global, local
    int type; // int, real, cstring
    long long value; // address
    
    // function part.
    long long p_hash;
    int * p_list; // supporting at most 10 parameters.
    int p_size;
} ; // current parsed idntifier

extern identifier *id_list = NULL, *cur_id = NULL, *id_main = NULL, func_id = { 0 };
extern identifier *(*ed) = NULL;
extern int depth = 0;

void next() {
    char *last_pos;
    
    while ((token = *src)) {
        if (*src == '\n' || *src == ' ' || *src == '\t') {
            if (*src == '\n')
                ++line;
            ++src;
            continue;
        }
        if (isalpha(token) || token == '_') { // id
            last_pos = src; // ??
            int new_hash = token;
            string new_name;
            while (isalpha(*src) || isdigit(*src) || *src == '_') {
                new_name.push_back(*src);
                new_hash = new_hash * 147 + *src;
                src++;
            }
            // writing in the empty space closest from begin.
            for (cur_id = ed[depth]; cur_id >= id_list; cur_id--) { // tokens begin with 128, 0 -> unused
                if (cur_id->hash == new_hash && new_name == cur_id->name) {
                    // function over-loading.
                    if (cur_id->class_ == Fun && (cur_id->p_hash != func_id.p_hash ||
                                                  cur_id->p_size != func_id.p_size ||
                                                  memcmp(cur_id->p_list, func_id.p_list, cur_id->p_size) != 0)) {
                        continue; // not the same function
                    }
                    token = cur_id->token;
                    return;
                }
            }

			cur_id = ++ed[depth];
			memset(cur_id, 0, sizeof(identifier));
            cur_id->name = new_name;
            cur_id->hash = new_hash;
			cur_id->type = Int;
            
            token = cur_id->token = Id; // is a identifier
            return ;
        }
        // dealing with integer value and floating-point value
        else if (isdigit(token)) { // numbers
            string tmp;
            int size = 0, num_dot = 0;
            bool is_float = false;
            while (isdigit(*src) || *src == '.') {
                if (*src == '.') {
                    is_float = true;
                    num_dot++;
                }
                tmp.push_back(*src++);
            }
            // changing part.
            // writing constant into data area, and return address.
            if (num_dot >= 2) {
                printf("%d: invalid floating-point number.\n", line);
                exit(-1);
            }
            if (is_float) {
                token = Fnum;
                ftoken_val = atof(tmp.data());
            }
            else {
                token = Inum;
                token_val = atoi(tmp.data());
            }
            return ;
        }
        // strings
        else if (token == '"') {
            last_pos = data_;
            ++src;
            while (*src != 0 && *src != '"') {
                token_val = *src++;
                // supporting only '\n' and '\t' for now
                if (token_val == '\\') {
                    token_val = *src++;
                    if (token_val == 'n')
                        token_val = '\n';
                    else if (token_val == 't')
                        token_val = '\t';
                    else if (token_val == '"')
                        token_val = '\"';
                }
                *data_++ = token_val;
            }
            src++;
            token = Cstr;
            token_val = (long long)last_pos; // return the address of string
            *data_++ = '\0';
            return ;
        }
        else if (token == '/') { // comment?
            if (*(src + 1) == '/') {
                while (*src != 0 && *src != '\n') {
                    ++src;
                }
                continue;
            }
            else {
                token = Div;
            }
        }
        else if (token == '=') {
            if (*(src + 1) == '=') { // ==
                ++src;
                token = Eq;
            }
            else {
                token = Assign;
            }
        }
        else if (token == '+') {
            if (*(src + 1) == '+') {
                ++src;
                token = Inc;
            }
            else {
                token = Add;
            }
        }
        else if (token == '-') {
            if (*(src + 1) == '-') { // "--"
                ++src;
                token = Dec;
            }
            else { // "-"
                token = Sub;
            }
        }
        else if (token == '!') {
            if (*(src + 1) == '=') {
                ++src;
                token = Ne;
            }
            // not adding "!id" yet
        }
        else if (token == '<') {
            if (*(src + 1) == '=') {
                ++src;
                token = Le;
            }
            else if (*(src + 1) == '>') {
                ++src;
                token = Ne;
            }
            else if (*(src + 1) == '<') { // >>
                ++src;
                token = Shl;
            }
            else {
                token = Lt;
            }
        }
        else if (token == '>') {
            if (*(src + 1) == '=') {
                ++src;
                token = Ge;
            }
            else if (*(src + 1) == '>') { // >>
                ++src;
                token = Shr;
            }
            else {
                token = Gt;
            }
        }
        else if (token == '&') {
            if (*(src + 1) == '&') {
                ++src;
                token = And;
            }
            else {
                token = Band;
            }
        }
        else if (token == '|') {
            if (*(src + 1) == '|') {
                ++src;
                token = Or;
            }
            else {
                token = Bor;
            }
        }
        else if (token == '*') {
            if (*(src + 1) == '*') {
                ++src;
                token = Pow;
            }
            else {
                token = Mul;
            }
        }
        else if (token == '$') {
            token = Str_add;
        }
        else if (token == '#') {
            token = Str_del;
        }
        else if (token == '^') {
            token = Xor;
        }
        else if (token == '%') {
            token = Mod;
        }
        else if (token == '?') { // ????
            token = Cond;
        }
        else if (token == '[') { // ????
            token = Brak;
        }
        // ???
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            // directly return the character as token;
        }
        ++src;
        return ;
    }
    return ;
}

void id_list_inintializer() {
    // supporting only 1000 different identifer.
    if ((id_list = (identifier *)malloc(sizeof(identifier) * 1000)) == NULL) {
        printf("failed to allocate memory for identifier list.\n");
        exit(-1);
    }
	if ((ed = (identifier **)malloc(sizeof(identifier *) * 1000)) == NULL) {
		printf("failed to allocate memory for identifier end list.\n");
		exit(-1);
	}
    memset(id_list, 0, sizeof(identifier) * 1000);
	ed[0] = id_list;

    src = (char *)"string int real else if return while for do until break continue in out void main";
    for (int tok = String; tok <= Void; tok++) {
        next();
        cur_id->token = tok;
    } // loading predefined identifier into id_list.

	next();
    id_main = cur_id;
	id_main->token = Id;

    src = old_src;
}

void print_token();

void get_next_token()
{
	next();
    if (token == '{') {
        ed[depth + 1] = ed[depth];
        depth++;
    }
	else if (token == '}')
        depth--;
}

void match(int tok) {
	if (token == tok) {
		get_next_token();
		// test
//		print_token();
		// test
	}
	else {
		printf("%d: expected token: %d\n", line, tok);
		exit(-1);
	}
}






// testing part:
// WARNING: will cause side effect of src and id_list

extern const string mat[] = {
    "Inum", "Fnum", "Cstr", "Fun", "Sys", "Glo", "Loc", "Id",
    "String", "Int", "Real", "Else", "If", "Return", "While", "For",
    "In", "Out", "Void", "Main",
    // the former are pre-defined identifier
    "Assign", "Cond", "Str_Add", "Str_del", "Bor", "Band", "Or", "Xor", "And", "Eq", "Ne", "Lt", "Gt", "Le", "Ge",
    "Shl", "Shr", "Add", "Sub", "Mul", "Div", "Mod", "Pow", "Inc", "Dec", "Brak"
};

void print_token() {
    if (token >= 128) {
        if (Id <= token && token <= While)
            printf("<%s>\n", cur_id->name.data());
        else {
            printf("<%s, %lld>\n", mat[token-128].data(), token_val);
            if (token == Fnum) {
                cout << ftoken_val << endl;
            }
            if (token == Cstr) {
                cout << (char *)token_val << endl;
            }
        }
    }
    else
        printf("<%c>\n", token);
}

void test() {
    while (*src != 0) {
        next();
        if (token == 0)
            break;
        print_token();
        token = token_val = 0; // delete pre
    }
}


#endif /* lexical_analysis_hpp */

