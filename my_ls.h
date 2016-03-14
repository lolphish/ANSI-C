#ifndef LS_H
#define LS_H
#include <sys/types.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>

// returns a bool value for whether str1 == str2
bool _strcmp_equals(char * str1, char * str2);

/*
 * Check if the given directory is a valid path. If it is not, then exit the
 * program.
 */
DIR * open_dir(char * dirName);

/*  
 * Creates a directory file, a dirent struct, a stat struct, 
 * and a char for usage in interpret file.
 */
void generate_stat(char * name, int padding);

/*
 * Will take in the members amde in the generate stat function for use, then, if the
 * file or directory given is not a pointer to the current or previous directory, the
 * function will run read stat.
 */
void interpret_file(char * name, struct dirent * file, struct stat st, char * buf, int padding);

/*
 * the function will print the necessary information for the file, or recursively loop 
 * if given a directory.
 */
void read_stat(char * buf, struct stat st, char * file_name, int padding);

// In the beginning, check if the type of arg is file or directory, and perform their respective code
void check_type(char * name, int padding);

/*
 * prints the necessary components for a directory, then recursively
 * calls the generate_stat function for that file 
 */ 
void _is_dir(char * name, struct stat st, char * file_name, int padding);

// Print, in order: mode  num_links  owner  group  size_in_bytes modification_timedate name
void _is_file(char *name, struct stat st, int padding);

// Prints the owner if there is one, else default
void _print_owner(struct stat st);

// Prints the group if there is one, else default
void _print_group(struct stat st);

// Computes the size and time of modification of the file and prints it
void _print_size_and_time(struct stat st);

// an amalgamation of various print functions for the file
void _print_variables(struct stat st);

// Prints the Read, Write, and Execute permissions for the user
void _print_user_permission(struct stat st);

// Prints the Read, Write, and Execute permissions for the group
void _print_group_permissions(struct stat st);

// Prints the Read, Write, and Execute permissions for others
void _print_other_permissions(struct stat st);

// Amalgamation of permission print statements
void _print_permissions(struct stat st);

#endif //LS_H
