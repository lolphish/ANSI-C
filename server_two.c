/*
    ICS 53: Principles of System Design
    March 10, 2016
    Thread pool system that has one thread that receives
    client requests for files on the server and stores these 
    client requests in a queue. At the same time, N_THREADS -1
    other threads dequeue client requests from the queue 
    and handle the requests, sending the file to the remote 
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

#define BACKLOG 200
#define N_THREADS 5


// ------------------------ QUEUE FUNCTIONS ------------------------------
/*
    Simple queue that allows for enqueing, dequeueing, and indicating whether
    the queue is empty of full.
*/
#define QUE_MAX 5
#define ELE int
ELE _que[QUE_MAX];

int _front = 0, _rear = 0;

void que_error(char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    exit(-1);
}

int que_is_full()
{
    return (_rear + 1) % QUE_MAX == _front; /* this is why one slot is unused */
}

int que_is_empty()
{
    return _front == _rear;
}

void que_enq(ELE v)
{
    if ( que_is_full() )
        que_error("enq on full queue");
    _que[_rear++] = v;
    if ( _rear >= QUE_MAX )
        _rear = 0;
}

ELE que_deq()
{
    if ( que_is_empty() )
        que_error("deq on empty queue");
    ELE ret = _que[_front++];
    if ( _front >= QUE_MAX )
        _front = 0;
    return ret;
}

// --------------------------- SERVER -------------------------
// if more than BACKLOG clients in the server accept queue, client connect will fail
//
int files_sent = 0;
pthread_mutex_t count_mutex;
pthread_cond_t count_threshold_cv;

void error(char *msg)
{
    fprintf(stderr, "ERROR: %s failed\n", msg);
    exit(-1);
}

/*
    Initialize a sockaddr struct with the given
    port.
*/
struct sockaddr_in make_server_addr(short port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
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
int create_server_socket(short port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    struct sockaddr_in my_addr = make_server_addr(port);
    if ( s == -1 )
        error("socket()");
    bind(s, (struct sockaddr*)&my_addr, sizeof my_addr);
    listen(s, BACKLOG);
    return s;
}

/*
    Reads from the client socket for the name of the 
    file that is to be copied over to the client 
    machine.
*/
void get_file_request(int socket, char *fileName)
{
    char buf[BUFSIZ];
    int n = read(socket, buf, BUFSIZ);
    if ( n < 0 )
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
    if ( ifd == -1 )
        error("open()");
    while ( (n = read(ifd, buf, BUFSIZ)) > 0 )
        write(socket, buf, n);
    close(ifd);
}

/*
    Output Thread: Dequeue a client socket from the queue and process
    that request and send the file to the client. If the queue is empty.
    wait for the main thread to queue another client before proceeding.
*/
void * handle_request(void * arg)
{
	while (files_sent <= 200)
	{
		pthread_mutex_lock(&count_mutex);
		while (que_is_empty())
			pthread_cond_wait(&count_threshold_cv, &count_mutex);

		int client_socket = que_deq();
		pthread_cond_signal(&count_threshold_cv);
		char fileName[BUFSIZ];
		get_file_request(client_socket, fileName);
		write_file_to_client_socket(fileName, client_socket);
		close(client_socket);
		++files_sent;
		pthread_mutex_unlock(&count_mutex);
	}
	pthread_exit(0);
    return NULL;
}

void time_out(int arg)
{
    fprintf(stderr,  "Server timed out\n");
    exit(0);
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
    Main thread: Accept client connections and inputs them into a queue
    to be used by other concurrently running threads. When the queue is full,
    wait until one of the output threads take and dequeue a thread before
    continuing to accept requests.
*/
void * request_work(void * arg)
{
	while (files_sent <= 200)
	{
		int server_sock = *((int *) arg);
		int client_socket;
		struct sockaddr_in client_addr;
		socklen_t sin_size = sizeof client_addr;
		set_time_out(5);
		while ( (client_socket = accept(server_sock, (struct sockaddr*) &client_addr, &sin_size)) > 0)
		{
			if (que_is_full())
				pthread_cond_wait(&count_threshold_cv, &count_mutex);
			que_enq(client_socket);
			pthread_cond_signal(&count_threshold_cv);
		}
	}
	pthread_exit(0);
   return NULL; 
}


/*
    Join the threads together
*/
void join_threads(pthread_t threads[])
{
    for (int i = 0; i < N_THREADS; ++i)
        pthread_join(threads[i], NULL);  
    printf ("Main(): Waited on %d  threads. Done.\n", N_THREADS); 
}

/*
    Create N_THREADS threads and connect the threads together.
    Have one thread run as the collector of client requests and
    the rest of the threads workers that enact the request of the
    client.
*/
void accept_client_requests(int server_socket)
{
    pthread_t threads[N_THREADS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    // Create threads 
    pthread_create(&threads[0], &attr, request_work, (void *) &server_socket);  
    for (int i = 1; i < N_THREADS; ++i)
        pthread_create(&threads[i], &attr, handle_request, NULL);  

    join_threads(threads); 
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&count_threshold_cv);
}


int main(int argc, char *argv[])
{
	if (argc != 2)
		error("usage: server port");
	short port = atoi(argv[1]);

    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&count_threshold_cv, NULL);
    int server_socket = create_server_socket(port);
    accept_client_requests(server_socket);
    shutdown(server_socket, 2);
    return 0;
}
