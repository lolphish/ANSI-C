/*
 * ICS 53: Principles of System Design
 * Winter Quarter 2015/2016
 * Shell Program that can handle pipes, background, and redirection.
 * Program parsing is not exactly correct and can only handle
 * if reasonably spaced. 
 * e.g: cat < existingInputFile | tr A-Z a-z | tee newOutputFile1 | tr a-z A-Z > newOutputFile2 &
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#define SIZE 1028
char * parseArray[SIZE];
char * command;
char * argumentList;
int count = 0;
char pipeArray[SIZE][SIZE][SIZE];
int pipe_counter[SIZE];
int pipe_count = 0;

void wait_for_children();


// Separate the input by spaces
void parse (char * buf, char * args[])
{
	while (*buf != 0)
	{
		while (isspace(*buf))
            *buf++ = '\0';
        *args++ = buf;
        while (*buf != '\0' && *buf != ' ' && *buf != '\t')
            buf++;
	}
	*args = '\0';
}

// Strip the new line at the end of the input (due to fgets)
char * strip_line(char * line)
{
    if (line[strlen(line)-1] == '\n')
           line[strlen(line)-1] = '\0';
    return line;
}

// Create the global count variable
void count_argument_size()
{ 
    for (int i = 0; parseArray[i]; ++count, ++i); 
}

// Helper function for parsing the < > in the input
void crunch_up(char ** array, int index)
{
    for (int i = count-1; i >=  index; --i)
        array[i+1] = array[i];
    count++;
}

/* 
 * If the user input has no spaces between the redirection symbol and input
 * i.e: <input, store_parsed_arguments will separate the symbol from the input
 * i.e: < , input
 */
void store_parsed_arguments( char * array[] )
{
    for ( int i = 0; i < count; ++i)
        if ((array[i][0] == '>' || array[i][0] == '<') && array[i][1])
        {
            crunch_up(array, i); 
            array[i] = array[i+1][0] == '>' ? ">" : "<";
            array[i+1] = ++array[i+1];
        }
}

// Separate the redirection (if any) with the command arguments
void parse_redirection(int index, char ** arguments, char ** redirection)
{
    int i = 0;
    for ( ; i < pipe_counter[index] && strcmp(pipeArray[index][i], ">") != 0 && strcmp(pipeArray[index][i], "<") != 0; ++i)
            *arguments++ = pipeArray[index][i];
    for ( ; i < pipe_counter[index]; ++i)
        *redirection++ = pipeArray[index][i];
}
/*
 * Find if there is a redirection input: if there is one, use dup2
 * to make the redirection
 */
int redirect_in(char ** redirection)
{    
    for (int i = 0; redirection[i]; ++i)
    {
        if (strcmp(redirection[i], "<") == 0 )
        {
           int in = open(redirection[i+1], O_RDONLY); 
           return in;
        }
    }
    return -1;
}

/*
 * Find if there is a redirection output: if there is one, use dup2
 * to make the redirection
 */
int redirect_out(char ** redirection)
{
    for (int i = 0; redirection[i]; ++i)
    {
       if (strcmp(redirection[i],">") == 0)
       {
           int out = open(redirection[i+1], O_WRONLY | O_TRUNC | O_CREAT, 
                  0664); 
           return out;
       } 
    }
    return -1;
}

// Execute the given command line args
void execute(char * args[], int background, int in, int out)
{
    int pid;
    switch (pid = fork())
    {
        case -1: perror("fork failed");
        case 0:
                 dup2(in, STDIN_FILENO);
                 dup2(out, STDOUT_FILENO);
                 if (execvp (args[0], args) == -1) 
                     perror("execvp");
        default:
                close(in);
                close(out);
				if (background != 0)
                 	while(wait(0) != -1);
				else
					printf("");//printf("Printed for background purposes\n");
                break;
    }
}

/*
 * Execute the argument with the piping (if there is one) and create
 * any redirection needed 
 */
int execute_redirection(char ** arguments, char ** redirection, int input, int output, int background)
{
    int in = (input != -1 ? input : redirect_in(redirection));
    int out = (output != -1 ? output : redirect_out(redirection));
    execute(arguments, background, in, out); 
    return 1;
    
}

/*
 * checks if there is a & (background) symbol in the argument
 * if there is a background symbol, return 0, else 1
 */
int run_background(int index)
{
    if (strcmp(pipeArray[pipe_count-1][pipe_counter[index] -1], "&") == 0)
        return 0;
    return -1;
}

/*
 * check if the input is bigger than pipe and end if it is
 * creat the argument argv and the redirection items and run the 
 * execute function
 */
int execute_all( int index, int input, int output)
{
   char * arguments[SIZE];
   char * redirection[SIZE];
   parse_redirection( index, arguments, redirection);
   int background = run_background(index);
   execute_redirection(arguments, redirection, input, output, background);
   return 1;
}

// helper function to check if the pipe function worked
void _print_all_pipes()
{
    for (int i = 0; i < pipe_count; ++i)
    {
        printf("PIPE %d\n", i);
        for (int j = 0; j < pipe_counter[i]; ++j)
        {
            printf("%s\n", pipeArray[i][j]);
        }
    }
}

// Split each command 
void make_pipe_array()
{
    int i;
    for (i = 0; parseArray[i]; ++i ){
        int counter = 0;
        for (int j = 0; parseArray[i] && (strcmp(parseArray[i], "|") != 0); ++i, ++j, ++counter){
           strcpy(pipeArray[pipe_count][j], parseArray[i]);
        }
           pipe_counter[pipe_count++] = counter;
    }
}

void wait_for_children()
{
    int pid;
    while ( (pid = wait(0)) > 0 );
}

// Make the pipe function between the item in index first and index second
void pipe_all(int first, int second)
{
    int pipefds[2];
    if (pipe(pipefds) == -1)
    {
        perror("cannot pipe");
        exit(1);
    }
    execute_all(first, -1 ,pipefds[1]);
    execute_all(second , pipefds[0], -1);
        
}


// Runs the program without a pipe if there are only one command
void pipe_one(int first)
{
    execute_all(first, -1, -1);
}
/*
 * Program will work differently depending on if there are one command
 * or multiple commands. If there are one command, a pipe is not made.
 */
void pipe_one_or_more()
{
    if (pipe_count <= 1)
        pipe_one(0);
    else
    {
        for (int i = 0; i < pipe_count-1; ++i)
             pipe_all(i,i+1);
    }
}

/*
 * Calls the count function to count the parseArray to the global counter
 * then call the store_parsed_arguments function parse the < > symbols
 * then split each command by pipe using make_pipe_array() and call the 
 * pipe_one_or_more function
 */
int run_program()
{
    count_argument_size();
    store_parsed_arguments(parseArray);
    make_pipe_array();
    pipe_one_or_more();
    return 1;
}


// main takes in a line of input, strips it of the ending \n and runs shell
int main(int argc, char * argv[])
{
    char buf[SIZE];
    fgets(buf, SIZE, stdin); strip_line(buf);
    parse(buf, parseArray);
    run_program();
    return 0;
}
