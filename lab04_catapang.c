#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct
{
    int **matrix;
    int start_col;
    int cols;
    int rows;
    int slave_sock;
    int cpu_core;
} ThreadArgs;

#define INET_AD inet_addr("127.0.0.1")

// Function to generate a matrix
int **generate_matrix(int n)
{
    int **matrix = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        matrix[i] = malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = rand() % 100 + 1;
        }
    }
    return matrix;
}

// Function to print a matrix
void print_matrix(int **matrix, int rows, int cols)
{
    printf("Matrix:\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

// what happens during connection
void *distribute_matrix(void *arg)
{
    ThreadArgs *thread = (ThreadArgs *)arg;

    int **matrix = thread->matrix;
    int start_col = thread->start_col;
    int cols = thread->cols;
    int rows = thread->rows;
    int slave_sock = thread->slave_sock;

    // Send matrix sizes to slave
    if (send(slave_sock, &rows, sizeof(int), 0) == -1)
    {
        perror("Error sending matrix row size to slave");
        exit(EXIT_FAILURE);
    }
    if (send(slave_sock, &cols, sizeof(int), 0) == -1)
    {
        perror("Error sending matrix column size to slave");
        exit(EXIT_FAILURE);
    }

    // Send the matrix values
    for (int i = 0; i < rows; i++)
    {
        if (send(slave_sock, matrix[i] + start_col, cols * sizeof(int), 0) == -1)
        {
            perror("Error sending matrix data to slave");
            exit(EXIT_FAILURE);
        }
    }

    char ack;
    if (recv(slave_sock, &ack, sizeof(char), 0) == -1)
    {
        perror("Error receiving acknowledgment from slave");
        exit(EXIT_FAILURE);
    }
    printf("\nAcknowledgement from slave");

    close(slave_sock);
    pthread_exit(NULL);
}

// Master function
void master(int n, int p, int t)
{
    printf("Starting master...\n");

    // Create matrix
    int **matrix = generate_matrix(n);
    // print_matrix(matrix, n, n);

    // Socket creation and binding
    int master_sock,
        slave_sock;
    struct sockaddr_in master_addr, slave_addr;
    socklen_t slave_size;
    master_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (master_sock < 0)
    {
        perror("Error while creating socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully\n");

    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(p);
    master_addr.sin_addr.s_addr = INET_AD;

    if (bind(master_sock, (struct sockaddr *)&master_addr, sizeof(master_addr)) < 0)
    {
        perror("Couldn't bind to the port");
        exit(EXIT_FAILURE);
    }
    printf("Done with binding\n");

    // Turn on the socket to listen for incoming connections
    if (listen(master_sock, 50) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("\nListening....\n");
    }

    struct timeval time_before, time_after;

    // Matrix distribution to slaves
    pthread_t tid[t];
    ThreadArgs args[t];
    int cols_per_thread = n / t;
    int extra_cols = n % t;
    int start_col = 0;
    int num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    cpu_set_t cpuset;

    gettimeofday(&time_before, NULL);
    for (int i = 0; i < t; i++)
    {
        slave_size = sizeof(slave_addr);
        slave_sock = accept(master_sock, (struct sockaddr *)&slave_addr, &slave_size);
        if (slave_sock < 0)
        {
            perror("Can't accept");
            exit(EXIT_FAILURE);
        }

        args[i].matrix = matrix;
        args[i].start_col = start_col;
        args[i].cols = cols_per_thread + ((i == t - 1) ? extra_cols : 0);
        args[i].rows = n;
        args[i].slave_sock = slave_sock;
        args[i].cpu_core = i % (num_processors - 1);

        CPU_ZERO(&cpuset);
        CPU_SET(args[i].cpu_core, &cpuset);
        pthread_create(&tid[i], NULL, distribute_matrix, (void *)&args[i]);
        pthread_setaffinity_np(tid[i], sizeof(cpu_set_t), &cpuset);
        start_col += args[i].cols;
    }

    // Wait for all threads to finish
    for (int i = 0; i < t; i++)
    {
        pthread_join(tid[i], NULL);
    }
    gettimeofday(&time_after, NULL);
    double elapsed_time = (time_after.tv_sec - time_before.tv_sec) + (time_after.tv_usec - time_before.tv_usec) / 1e6;

    printf("\nTime elapsed: %.6f seconds\n", elapsed_time);

    // Clean up matrix
    for (int i = 0; i < n; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
}

// Slave function
void slave(int n, int p, int t)
{
    // Socket creation and connection to master
    int slave_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (slave_sock < 0)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    // get the master sock details
    struct sockaddr_in master_addr;
    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(p);
    master_addr.sin_addr.s_addr = INET_AD;

// keep trying to connect slave to master
connect:
    if (connect(slave_sock, (struct sockaddr *)&master_addr, sizeof(master_addr)) < 0)
    {
        perror("\nUnable to connect. Retrying to connect...");
        goto connect;
    }
    printf("\nConnected with master successfully!\n");

    struct timeval time_before, time_after;
    gettimeofday(&time_before, NULL);

    // Receive matrix from master
    int rows, cols;
    if (recv(slave_sock, &rows, sizeof(int), 0) == -1)
    {
        perror("Error receiving matrix size from master");
        exit(EXIT_FAILURE);
    }
    if (recv(slave_sock, &cols, sizeof(int), 0) == -1)
    {
        perror("Error receiving matrix size from master");
        exit(EXIT_FAILURE);
    }

    int **submatrix = malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++)
    {
        submatrix[i] = malloc(cols * sizeof(int));
        if (recv(slave_sock, submatrix[i], cols * sizeof(int), 0) == -1)
        {
            perror("Error receiving matrix data from master");
            exit(EXIT_FAILURE);
        }
    }

    // Send acknowledgment to master
    char ack = 'A'; // Dummy acknowledgment
    if (send(slave_sock, &ack, sizeof(char), 0) == -1)
    {
        perror("Error sending acknowledgment to master");
        exit(EXIT_FAILURE);
    }
    gettimeofday(&time_after, NULL);

    double elapsed_time = (time_after.tv_sec - time_before.tv_sec) + (time_after.tv_usec - time_before.tv_usec) / 1e6;

    printf("\nTime elapsed: %.6f seconds\n", elapsed_time);

    // Print received submatrix
    printf("Received Submatrix:\n");
    // print_matrix(submatrix, rows, cols);

    // Clean up submatrix
    for (int i = 0; i < rows; i++)
    {
        free(submatrix[i]);
    }
    free(submatrix);

    // Close socket
    close(slave_sock);
}

int main(int argc, char *argv[])
{
    int n, p, s, t;

    // printf("Enter values for matrix size, port, master=0/slave=1, and number of slaves (separated by spaces): ");
    // scanf("%d %d %d %d", &n, &p, &s, &t);

    // Check if the correct number of arguments is provided
    if (argc != 2)
    {
        printf("Usage: %s [0 for master, 1 for slave]\n", argv[0]);
        return 1;
    }

    // Convert the argument to an integer
    s = atoi(argv[1]);

    srand(time(NULL)); // Initialize random seed

    if (s == 0)
    {
        master(n, p, t);
    }
    else if (s == 1)
    {
        slave(n, p, t);
    }

    return 0;
}

/*
References:
https://www.geeksforgeeks.org/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/?ref=ml_lbp
*/