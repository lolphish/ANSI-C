/*
    ICS 53: Principles of System Design
    March 10, 2016
    Client that create 10 Threads, each asking the server
    for 20 files (listed in the files array) (200 files in total).
    This code will only work on the UCI Openlab, where the files listed
    exists.
*/
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <strings.h>
#include "Timer.h"

#define N_THREADS 10

char * files[] = {
    "/usr/share/dict/words",
    "/usr/include/sqlite3.h",
    "/usr/include/tclDecls.h",
    "/usr/include/bfd.h",
    "/usr/include/libmng.h",
    "/usr/include/elf.h",
    "/usr/include/gmpxx.h",
    "/usr/include/tkDecls.h",
    "/usr/include/H5overflow.h",
    "/usr/include/tcl.h",
    "/usr/include/gmp-x86_64.h",
    "/usr/include/curses.h",
    "/usr/include/lcms.h",
    "/usr/include/netapi.h",
    "/usr/include/gcrypt.h",
    "/usr/include/zlib.h",
    "/usr/include/ldap.h",
    "/usr/include/geos_c.h",
    "/usr/include/kdb.h",
    "/usr/include/tk.h",
};

#define files_length() (sizeof files / sizeof files[0])

void error(char *msg)
{
    perror(msg);
    exit(-1);
}

/*
    Create the main server addr with the given
    port and host, using gethostbyname to translate
    the string object to a host ip address.
*/
struct sockaddr_in make_server_addr(char *host, short port)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    struct hostent *hp = gethostbyname(host);
    if ( hp == 0 )
        error(host);
    addr.sin_family = AF_INET;
    bcopy(hp->h_addr_list[0], &addr.sin_addr, hp->h_length);
    addr.sin_port = htons(port);
    return addr;
}

/*
    Creates a socket and attempt to connect to the server
    sockaddr. 
*/
int connect_socket(char *host, short port)
{
    int status;
    int tries = 3;
    struct sockaddr_in addr = make_server_addr(host, port);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if ( s == -1 )
        error("socket()");
    status = connect(s, (struct sockaddr*)&addr, sizeof addr);
    if ( status < 0 )
        error("connect refused");
    return s;
}

/*
    Send the name of a file to the server, writing it to the
    server socket.
*/
void request_file_from_server(int server_socket, char *file)
{
    int len = strlen(file);
    int n = write(server_socket, file, len);
    if ( n != len )
        error("short write");
}

/*
    After writing the name of a file to the server, receive
    the file from the server and store it.
*/
void read_file_from_server(int server_socket, char *file)
{
    char buf[BUFSIZ];
    int n;
    mode_t mode = 0666;
    int ofd = open(file, O_WRONLY | O_CREAT, mode);
    if ( ofd == -1 )
        perror("open()");
    while ( (n = read(server_socket, buf, BUFSIZ)) > 0 )
        write(ofd, buf, n);
    close(ofd);
}

/*
    Used to give threads data necessary to function
*/
struct Thread_data
{
    int id;
    pthread_t thread_id;
    char * host;
    short port;
    char path[BUFSIZ];
};

/*
    Create a Location for the file
*/
void make_file_name(char *local_name, char *dir, char *original_path)
{
    char *p = rindex(original_path, '/');
    if ( !p )
        error("rindex()");
    sprintf(local_name, "%s/%s", dir, p+1);
}

/*
    Helper function that is called by each thread.
    Requests the file and store the file on the machine
    as received by the server.
*/
int remote_copy(struct Thread_data * data, char * file)
{
    int server_socket = connect_socket(data->host, data->port);
    request_file_from_server(server_socket, file);
    char local_name[BUFSIZ];
    make_file_name(local_name, data->path, file);
    read_file_from_server(server_socket, local_name);
    close(server_socket);
}

/*
    Create a directory for each individual thread, storing
    the 20 files copied into there.
*/
void make_empty_dir_for_copies(struct Thread_data * data)
{
    mode_t mode = 0777;
    sprintf(data->path, "./Thread_%d", data->id);
    mkdir(data->path, mode);
}

#define N_FILES_TO_COPY files_length() // copy them all
// #define N_FILES_TO_COPY 1 // just copy one when testing

/*
    Function for the thread to copy all files listed.
*/
void *thread_work(void *arg)
{
    struct Thread_data * data = (struct Thread_data *)arg;
    make_empty_dir_for_copies(data);
    for ( int i=0; i < N_FILES_TO_COPY; ++i )
        remote_copy(data, files[i]);
    pthread_exit(0);
}

/*
    Create N_THREADS threads ,store necessary data into
    the Thread_data struct, and spawn a thread to enact
    the remote copy.
*/
void start_threads(char *host, short port, struct Thread_data thread_args[])
{
    for ( int i = 0; i < N_THREADS; ++i )
    {
        struct Thread_data * t = &thread_args[i];
        t->id = i;
        t->host = host;
        t->port = port;
        pthread_create(&t->thread_id, NULL, thread_work, t);
    }
}

void join_threads(struct Thread_data thread_args[])
{
    for ( int i=0; i < N_THREADS; i++ )
        pthread_join(thread_args[i].thread_id, NULL);
}

int main(int argc, char *argv[])
{
    double eTime;
    Timer_start();
    if ( argc != 3 )
    {
        fprintf(stderr, "Usage: %s host port\n", argv[0]);
        exit(-1);
    }

    struct Thread_data thread_args[N_THREADS];
    char *host = argv[1];
    short port = atoi(argv[2]);
    start_threads(host, port, thread_args);
    join_threads(thread_args);

    Timer_elapsedUserTime(&eTime);
    printf("Time taken: %lf\n", eTime);
    pthread_exit(0);
}
