#pragma once
#ifndef syntax_analysis_h
#define syntax_analysis_h

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <queue>

#include "lexical_analysis.h"
#include "expression_analysis.h"

queue<long long *>ctn_addr;
queue<long long *>brk_addr;

void statement()
{
	// there are 6 kinds of statements here:
	// 1. if (...) <statement> [else <statement>]
	// 2. while (...) <statement>
	// 3. { <statement> }
	// 4. return xxx;
	// 5. <empty statement>;
	// 6. expression; (expression end with semicolon)
	long long *a, *b, *c;
	int type;

	if (token == Int || token == String || token == Void || token == Real)
	{
		if (token == Int) { type = INT; }
		else if (token == String) { type = STRING; }
		else if (token == Real) { type = REAL; }
		else if (token == Void) { type = VOID; }
		match(token);

		while (token != ';')
		{
			while (token == Mul) {
				match(Mul);
				type = type + PTR;
			}

			if (token != Id) {
				printf("%d: bad paramater declaration\n", line);
				exit(-1);
			}
			if (cur_id->class_ != 0 && cur_id > ed[depth - 1]) {  //find id in this depth
				printf("%d: duplicate parameter declaration\n", line);
				exit(-1);
			}
			else if (cur_id->class_ != 0 && cur_id <= ed[depth - 1]) {  //find id in pre depth
				identifier this_id = *cur_id;
				cur_id = ++ed[depth];

				cur_id->token = this_id.token;
				cur_id->hash = this_id.hash;
				cur_id->name = this_id.name;

			}
			char *src_before_id = src;
			identifier *t_id = cur_id;
			match(Id);

			cur_id->class_ = Loc;
			cur_id->type = type;
			cur_id->value = ++local_pos;
			params++;
			// get an 8 byte of space in stack.
			if (token == ',')match(',');
			else {
				src = src_before_id;
				token = Id;
				cur_id = t_id;
				expression(Assign);
				if (token == ',')match(',');
			}
		}
		match(';');
	}

	else if (token == If)
	{
		// …| cond | JZ | addr1 | …statement1… | JMP | addr2 | statement2 | …
		//                   |______________________________|___________|       |
		//                                                  |___________________|

		match(If);
		//match('(');
		expression(Assign);
		//match(')');

		*++text = JZ;
		b = ++text;

		statement();
		if (token == Else)
		{
			match(Else);

			*b = (long long)(text + 3);
			*++text = JMP;
			b = ++text;

			statement();
		}

		*b = (long long)(text + 1);
	}
	else if (token == While)
	{
		// бн| cond | JZ | addr1 | бнstatement1бн | JMP | addr2 |бн
		//      |            |______________________________|_____|
		//      |___________________________________________|
		match(While);

		a = text + 1;
		//match('(');
		expression(Assign);
		//match(')');

		*++text = JZ;
		b = ++text;

		statement();

		*++text = JMP;
		*++text = (long long)a;
		*b = (long long)(text + 1);

		while (!brk_addr.empty())
		{
			long long *addr = brk_addr.front();
			brk_addr.pop();
			*addr = (long long)(text + 1);
		}
		while (!ctn_addr.empty())
		{
			long long *addr = ctn_addr.front();
			ctn_addr.pop();
			*addr = (long long)a;
		}
	}
	else if (token == For)
	{
		// ...| ...statement1... | cond | JZ | addr1 | ...statement3... | ...statement2... | JMP | addr2 |...
		//                          |           |___________________________________________________|______|
		//                          |_______________________________________________________________|

		match(For);
		statement(); // statement1

		a = text + 1; //addr2
		*(text + 1) = 1;
		if (token != ';')expression(Assign); //condition
							//        match(';');

		*++text = JZ;
		b = ++text; //addr1

		char *src_before_statement2 = src;
		while (*src != '{')++src;
		//        src++;

		int line_before_statement2 = line;
		line++;

		//        match('{');
		get_next_token();
		statement(); //statement3
					 // match('}');
		int tag = 0;
		if (token == '}') { depth++; tag++; } //if the next token == '}', then depth--, which is unexcepted.

		char *src_after_statement3 = src;
		int token_after_statement3 = token;
		src = src_before_statement2;

		int line_after_statement3 = line;
		line = line_before_statement2;

		get_next_token();
		c = text + 1;
		while (token != '{') //statement2
		{
			expression(Assign);
			if (token == ',')match(',');
		}

		*++text = JMP;
		*++text = (long long)a;
		*b = (long long)(text + 1);

		src = src_after_statement3;
		line = line_after_statement3;
		token = token_after_statement3;
		depth--;
		depth -= tag;

		while (!brk_addr.empty())
		{
			long long *addr = brk_addr.front();
			brk_addr.pop();
			*addr = (long long)(text + 1);
		}
		while (!ctn_addr.empty())
		{
			long long *addr = ctn_addr.front();
			ctn_addr.pop();
			*addr = (long long)c;
		}
	}
	else if (token == Do)
	{
		match(Do);
		a = (text + 1);
		statement();
		if (token != Until) {
			printf("%d: expected an \"until\" here.\n", line);
			exit(-1);
		}
		match(Until);

		expression(Assign);
		*++text = JNZ;
		*++text = (long long)a;

		while (!brk_addr.empty())
		{
			long long *addr = brk_addr.front();
			brk_addr.pop();
			*addr = (long long)(text + 1);
		}
		while (!ctn_addr.empty())
		{
			long long *addr = ctn_addr.front();
			ctn_addr.pop();
			*addr = (long long)a;
		}
	}
	else if (token == Break)
	{
		match(Break);
		*++text = JMP;
		brk_addr.push(++text);
	}
	else if (token == Continue)
	{
		match(Continue);
		*++text = JMP;
		ctn_addr.push(++text);
	}

	else if (token == '{')
	{
		match('{');
		while (token != '}')
			statement();
		match('}');
	}
	else if (token == Return)
	{
		match(Return);
		if (token != ';')
			expression(Assign);
		match(';');

		*++text = LEV;
	}
	else if (token == ';')match(';');
	else {
		expression(Assign);
		while (token == ',') {
			match(',');
			expression(Assign);
		}
		match(';');
	}
}

void function_body()
{
	// type func_name (...) {...}
	//                   -->|   |<--

	// ... {
	// 1. local declarations
	// 2. statements
	// }

	local_pos = index_of_var_bp;

	*++text = ENT;
	//*++text = ed[depth] - ed[depth - 2];

	while (token != '}')statement();

	*++text = LEV;
}

void function_parameter() // ERROR: the depth of the func_para is wrong now ! ! !
{
	int type;
	params = 0;

	// initializing of function parameter list.
	// clear current parameters list.
	func_id.token = Id;
	func_id.p_list = (int *)malloc(sizeof(int) * 23); // supporting 23 parameters.
	memset(func_id.p_list, 0, sizeof(int) * 23);
	func_id.p_hash = func_id.p_size = 0;
	
	// end adding part.

	while (token != ')')
	{
		if (token == Int) { type = INT; }
		else if (token == String) { type = STRING; }
		else if (token == Real) { type = REAL; }
		else if (token == Void) { type = VOID; }
		else {
			printf("%d: invalid parameters\n", line);
			exit(-1);
		}
		match(token);

		//pointer type
		while (token == Mul) {
			match(Mul);
			type = type + PTR;
		}

		func_id.p_hash = func_id.p_hash + type * (params + 1) * 17;
		func_id.p_list[func_id.p_size++] = type;

		//paramater name
		if (token != Id) {
			printf("%d: bad paramater declaration\n", line);
			exit(-1);
		}
		// cur_id is now a function parameter value.
		if (cur_id->class_  != 0 && cur_id > ed[depth - 1]) {  //find id in this depth
			printf("%d: duplicate parameter declaration\n", line);
			exit(-1);
		}
		else if (cur_id->class_ != 0 && cur_id <= ed[depth - 1]) {  //find id in pre depth
			identifier this_id = *cur_id;
			cur_id = ++ed[depth];

			cur_id->token = this_id.token;
			cur_id->hash = this_id.hash;
			cur_id->name = this_id.name;

		}
		match(Id);

		cur_id->class_ = Loc;
		cur_id->type = type;
		cur_id->value = params++;

		if (token == ',')match(',');
	}

	index_of_var_bp = params + 1;
	// now adding func_id to list(already having parameter version)
	for (identifier * it = ed[depth - 1]; it >= id_list; it--) {
		if (it->token == 0) {
			*it = func_id;
			break;
		}
	}
	
	memset(&func_id, 0, sizeof(identifier));
}

void function_declaration()
{
	// type func_name (...) {...}
	//               | this part
	match('(');
	ed[depth + 1] = ed[depth];
    depth++;
	function_parameter();
	match(')');

	match('{');
	function_body();
	//match("}"); //this '}' has been matched in function_body
	depth--;
}

void global_declaration()
{
	//EBNF: 

	// global_declaration ::= enum_decl | variable_decl | function_decl
	//
	// enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num'} '}'
	//
	// variable_decl ::= type {'*'} id { ',' {'*'} id } ';'
	//
	// function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

	int type = 0;

	while (token)
	{
		if (token == Int) { type = INT; }
		else if (token == String) { type = STRING; }
		else if (token == Real) { type = REAL; }
		else if (token == Void) { type = VOID; }
		else {
			printf("%d: invalid parameters\n", line);
			exit(-1);
		}
		match(token);

		while (token != ';' && token != '}')
		{
			while (token == MUL) { //index
				match(MUL);
				type = type + PTR;
			}

			if (token != Id) {
				//invalid declaration
				printf("%d: bad global declaration\n", line);
				exit(-1);
			}
			if (cur_id->class_ == Glo) {
				// some problem with this part.
				// because of function overloading, only judging for global variable.
				printf("%d: duplicate global declaration\n", line);
				exit(-1);
			}
			if (cur_id->name == "main" && cur_id->value != -1) {
				printf("%d: duplicate main function declaration.\n", line);
				exit(-1);
			}
			match(Id);
			cur_id->type = type;

			if (token == '(')
			{
				func_id = (*cur_id); // save the identifier of the pretential function name.
									 // but it's not deep copying.
				cur_id->token = 0;
				// delete this identifer in id_list
				// and construct a new one later.

				func_id.class_ = Fun;
				func_id.value = (long long)(text + 1);
				function_declaration();
				// beginning address is text + 1
			}
			else
			{
				cur_id->class_ = Glo;
				cur_id->value = (long long)data_;
				data_ = (char *)((long long)data_ + sizeof(long long));
				// get an 8 byte space in data_
			}

			if (token == ',') match(',');
		}
		get_next_token();
	}
}

void program()
{
	get_next_token();
	while (token > 0) {
		global_declaration();
	}
}

#endif
