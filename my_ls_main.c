#include "philn1_47604490_hw3_my_ls.h"

int main(int argc, char* argv[])
{ 
    if (argc >= 2)
    	for (int i = 1; i < argc; i++)
    		check_type(argv[i], 0);
    else
    	check_type(".", 0);
}
