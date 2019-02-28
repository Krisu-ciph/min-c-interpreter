#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<time.h>
#include<memory.h>

#include<string>
#include<iostream>
#include<algorithm>
#include<vector>
#include<queue>
#include<list>
#include<set>
#include<map>

#include "global_variable.h"
#include "virtual_machine.h"
#include "lexical_analysis.h"
#include "syntax_analysis.h"
#include "expression_analysis.h"

//#define debug_
//#define test_lex_
//#define test_expr_
#define output_assemble

int main(int argc, char *argv[])
{

	argc--;
	argv++;

	int poolsize = 64 * 1024 * 1024;

	if (!(src = old_src = (char *)malloc(poolsize))) {
		printf("could not malloc(%d) for source area\n", poolsize);
		return -1;
	}

	id_list_inintializer(); // id_list, read src after this.
	virtual_machine_initializer(); // text, data_, stack, sp, bp, pc = NULL, ax
								   //read file
	size_t file_size;
	FILE *fd;

	if ((fd = fopen(*argv, "r")) == NULL) {
		printf("could not open(%s)\n", *argv);
		return -1;
	}
	if ((file_size = fread(src, 1, poolsize - 1, fd)) <= 0) {
		printf("read() returned %lu\n", file_size);
		return -1;
	}

	src[file_size] = 0;
	fclose(fd);

	long st_time = clock();


#ifdef test_lex_
    printf("the total size of the src is: %ld\n\n", file_size);
    cout << src << endl;
#endif

#ifdef test_lex_
    test();
    cout << endl;
#endif

#ifdef test_expr_
    src = old_src;
    pc = text;
    while (true) {
        get_next_token();
        print_token();
        if (token == 0)
            break;
        expression(Assign);
    }
    cout << endl;

    text = old_text;
    for (int i = 0; i < 20; i++) {
        print_text(*(text + i));
    }
    cout << endl;
    pc = text + 1;
    eval();
#endif

	line = 1;
	id_main->value = -1;
	program();

	if (!(pc = (long long *)id_main->value)) {
		printf("main() not defined\n");
		return -1;
	}

	//setup stack
	*--ins_sp = EXIT;
	*--ins_sp = PUSH; long long *tmp = ins_sp;
	//*--ins_sp = argc;
	//*--ins_sp = (long long)argv;
	*--ins_sp = (long long)tmp;

#ifdef output_assemble
	long long *st = (long long *)id_main->value;
	for (int i = 0; i <= 40; i++) {
		print_text(*(st + i));
        if (*(st + i - 1) == CALL) {
            long long *func = (long long *)*(st + i);
            for (int j = 0; ; j++) {
                if (*(func + j - 1) == LEV)
                    break;
                putchar('\t');
                print_text(*(func + j));

            }
        }
	}
    cout << endl;
#endif

    eval();

	printf("using time: %lfs.\n", (double)(clock() - st_time) / CLOCKS_PER_SEC);

	return 0;
}
