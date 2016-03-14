/*
    ICS 53: Principles of System Design
    March 10, 2016
    Leader-Follower system that creates multiple threads that
    individually and concurrently interact with clients,
    handle the requests, sending the file to the remote 
    client machine.
*/
#include <sys/types.h>
#include <signal.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>

#define PORT 54760
#define BACKLOG 200
#define N_THREADS 5
#define thread_id thread_ids

pthread_mutex_t count_mutex;


void error(char * msg)
{
	fprintf(stderr, "ERROR: %s failed\n", msg);
	exit(-1);
}

void time_out(int arg)
{
	fprintf(stderr, "Server timed out\n");
	exit(0);
}

/*
    Initialize a sockaddr struct with the given
    port.
*/
struct sockaddr_in make_sockaddr(int port)
{
	struct sockaddr_in addr;
	memset(&addr, 0 , sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	return addr;
}

/*
    Create the main server socket with the given
    port, using setsockopt to change its options
    and binding the socket to a sockaddr object.
*/
int create_server_socket(int port)
{
	int s = socket(AF_INET, SOCK_STREAM, 0);
	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	if (s == -1)
		error("setsockopt()");
	struct sockaddr_in addr = make_sockaddr(port);	
	bind(s, (struct sockaddr*) &addr, sizeof addr);
	listen(s, BACKLOG);
	return s;
}

/*
    Reads from the client socket for the name of the 
    file that is to be copied over to the client 
    machine.
*/
void get_file_request(int socket, char * fileName)
{
	char buf[BUFSIZ];
	int n = read(socket, buf, BUFSIZ);
	if (n < 0)
		error("read from socket");
	buf[n] = '\0';
	strcpy(fileName, buf);
	printf("Server got file name of '%s'\n", fileName);
}

/*
    Writes the file prompted by the client over to
    the client machine.
*/
void write_file_to_client_socket(char *file, int socket)
{
	char buf[BUFSIZ];
	int n;
	int ifd = open(file, O_RDONLY);
	if (ifd == -1)
		error("open()");
	while ( (n = read(ifd, buf, BUFSIZ)) > 0)
		write(socket, buf, n);
	close(ifd);	
}


/*
    Used to indicate a maximum amount of time
    a server is to be opened before automatically
    stopping. (In case the client does not respond
    or there is an error).
*/
void set_time_out(int seconds)
{
	struct itimerval value = {0};
	value.it_value.tv_sec = seconds;
	setitimer(ITIMER_REAL, &value, NULL);
	signal(SIGALRM, time_out);
}

/*
	Overarching function for each thread, obtaining the
	file that the client wants and sending it to the client.
*/
void handle_request(int client_socket)
{
    char fileName[BUFSIZ];
    get_file_request(client_socket, fileName);
    
    write_file_to_client_socket(fileName, client_socket);
    close(client_socket);
}

/*
	Each thread runs in a continual loop and handle requests
	on their own, only stopping when accept fails or when the 
	time runs out.
*/
void * thread_work(void * arg)
{
	int server_socket = *((int *) arg);
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t sin_size = sizeof client_addr;
    set_time_out(5);
    while ( 1)
    {
    	pthread_mutex_lock(&count_mutex);
    	if( (client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &sin_size)) ==  -1)
			break;	
        set_time_out(5);
        handle_request(client_socket);
        pthread_mutex_unlock(&count_mutex);
    }
   	pthread_exit(0); 
}

/*
	Spawns N_THREADS threads that independently interact with 
	client requests. Joining them and exitting after each threadwork
	is finished.
*/
void create_pthreads(int server_socket)
{
	pthread_t threads[N_THREADS];
	for (int i = 0; i < N_THREADS; ++i)
		pthread_create(&threads[i], NULL, thread_work, (void *) &server_socket);

	for(int i = 0; i < N_THREADS; ++i)
		pthread_join(threads[i], NULL);

	pthread_exit(NULL);
}

int main(int argc, char * argv[])
{	
	if (argc != 2)
		error("usage: server port");
	short port = atoi(argv[1]);
    pthread_mutex_init(&count_mutex, NULL);

    int server_socket = create_server_socket(port);
   	create_pthreads(server_socket);

    shutdown(server_socket, 2);
	return 0;
}
