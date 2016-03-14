#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "philn1_47604490_hw3_my_ls.h"

DIR * open_dir(char * dirName)
{
    DIR * mydir;
    if (!(mydir = opendir(dirName)))
    {
        fprintf(stderr, "Error(%s) opening %s\n", strerror(errno), dirName);
        close(1);
    } 
    return mydir;
}

void generate_stat(char * name, int padding)
{
    DIR * mydir = open_dir(name);
    struct dirent *file;
    struct stat st;
    char buf[512];
    while((file = readdir(mydir)))
    {
        interpret_file(name, file, st, buf, padding);
    }
    closedir(mydir);
}

void interpret_file(char * name, struct dirent * file, struct stat st, char * buf, int padding)
{
    if (!_strcmp_equals(file->d_name,".") && !_strcmp_equals(file->d_name,".."))
    {
        sprintf(buf, "%s/%s", name, file->d_name);
        stat(buf, &st); 
        read_stat(buf, st, file->d_name, padding);
    } 
}

void read_stat(char * buf, struct stat st, char * file_name, int padding)
{
    if (S_ISDIR(st.st_mode))
        _is_dir(buf, st, file_name, padding);
    else if (S_ISREG(st.st_mode))
        _is_file(file_name, st, padding);
}
void check_type(char * name, int padding)
{
    struct stat st;
    stat(name, &st);
    if (S_ISREG(st.st_mode))
        _is_file(name, st, padding);
    else if (S_ISDIR(st.st_mode))
        _is_dir(name, st, name, padding);
    else
        open_dir(name);
}

void _is_dir(char * name, struct stat st, char * file_name, int padding)
{
    for (int i = 0; i < padding; i++)
        printf("\t");
    _print_variables(st);
    printf(" %s\n", file_name);
    generate_stat(name, padding+1);
}

void _is_file(char *name, struct stat st, int padding)
{
    for (int i = 0; i < padding; i++)
        printf("\t");
    _print_variables(st);
    printf(" %s\n", name);
}


void _print_owner(struct stat st)
{
    struct passwd *pwd;
    if ((pwd = getpwuid(st.st_uid)) != NULL)
        printf(" %s", pwd->pw_name);
    else
        printf(" %d", st.st_uid);
}


void _print_group(struct stat st)
{
    struct group *grp;
    if ((grp = getgrgid(st.st_gid)) != NULL)
        printf(" %s", grp->gr_name);
    else
        printf(" %d", st.st_gid);
}

void _print_size_and_time(struct stat st)
{
    struct tm *tm;
    char datestring[256];
    tm = localtime(&st.st_mtime);
    strftime(datestring, sizeof(datestring), "%b %d %R", tm);
    printf(" %8jd %s", (intmax_t)st.st_size, datestring);
}

void _print_variables(struct stat st)
{
    _print_permissions(st);
    printf(" %lu", st.st_nlink);
    _print_owner(st);
    _print_group(st);
    _print_size_and_time(st);
}

void _print_user_permission(struct stat st)
{
    printf( (st.st_mode & S_IRUSR) ? "r" : "-");
    printf( (st.st_mode & S_IWUSR) ? "w" : "-");
    printf( (st.st_mode & S_IXUSR) ? "x" : "-");
}

void _print_group_permissions(struct stat st)
{
    printf( (st.st_mode & S_IRGRP) ? "r" : "-");
    printf( (st.st_mode & S_IWGRP) ? "w" : "-");
    printf( (st.st_mode & S_IXGRP) ? "x" : "-");
}

void _print_other_permissions(struct stat st)
{
    printf( (st.st_mode & S_IROTH) ? "r" : "-");
    printf( (st.st_mode & S_IWOTH) ? "w" : "-");
    printf( (st.st_mode & S_IXOTH) ? "x" : "-");
}

void _print_permissions(struct stat st)
{
    printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
    _print_user_permission(st);
    _print_group_permissions(st);
    _print_other_permissions(st);
}

bool _strcmp_equals(char * str1, char * str2)
{
    return strcmp(str1, str2) == 0;
}