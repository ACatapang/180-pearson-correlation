#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define INET_AD inet_addr("127.0.0.1")

// Function to calculate Pearson Correlation Coefficient
void pearson_cor(double **X, double *y, int m, int n, double *v)
{
    for (int i = 0; i < n; i++)
    {
        double x_sum = 0, y_sum = 0, x_sqr = 0, y_sqr = 0, xy = 0;
        // v[i] = 0;

        for (int j = 0; j < m; j++)
        {
            // get summations
            x_sum += X[j][i];
            y_sum += y[j];
            x_sqr += pow(X[j][i], 2);
            y_sqr += pow(y[j], 2);
            xy += X[j][i] * y[j];
        }
        double numerator = m * xy - x_sum * y_sum;
        double denominator = sqrt((m * x_sqr - x_sum * x_sum) * (m * y_sqr - y_sum * y_sum));
        if (denominator != 0)
        {
            v[i] = numerator / denominator;
        }
        else
        {
            v[i] = 0.0; // Avoid division by zero
        }

        // printf("\nR[%d]: %f\n\tx = %f\n\ty = %f\n\tx2 = %f\n\ty2 = %f\n\txy = %f\n", i, v[i], x_sum, y_sum, x_sqr, y_sqr, xy);
    }
}

double *sample_vector(int n)
{
    double y_values[] = {53.1, 49.7, 48.4, 54.2, 54.9, 43.7, 47.2, 45.2, 54.4, 50.4};
    double *vector = malloc(n * sizeof(double));

    for (int i = 0; i < n; i++)
    {
        vector[i] = y_values[i];
    }

    return vector;
}

double **sample_matrix(int n)
{
    double col_val[] = {3.63, 3.02, 3.82, 3.42, 3.59, 2.87, 3.03, 3.46, 3.36, 3.3};
    double **matrix = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++)
    {
        matrix[i] = malloc(n * sizeof(double));
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = col_val[i];
        }
    }
    return matrix;
}

typedef struct
{
    double **matrix;
    double *y_vector;
    double *v;
    int start_col;
    int cols;
    int rows;
    int slave_sock;
    // int cpu_core;
} ThreadArgs;

// Function to generate vector y
double *generate_vector(int n)
{
    double *vector = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++)
    {
        vector[i] = rand() % 100 + 1;
    }
    return vector;
}

// Function to generate a matrix
double **generate_matrix(int n)
{
    double **matrix = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++)
    {
        matrix[i] = malloc(n * sizeof(double));
        for (int j = 0; j < n; j++)
        {
            matrix[i][j] = rand() % 100 + 1;
        }
    }
    return matrix;
}

// Function to print a matrix
void print_matrix(double **matrix, int rows, int cols)
{
    printf("Matrix:\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%f ", matrix[i][j]);
        }
        printf("\n");
    }
}

// what happens during connection
void *distribute_matrix(void *arg)
{
    ThreadArgs *thread = (ThreadArgs *)arg;

    double **matrix = thread->matrix;
    double *y_vector = thread->y_vector;
    double *v = thread->v;
    int start_col = thread->start_col;
    int cols = thread->cols;
    int rows = thread->rows;
    int slave_sock = thread->slave_sock;

    // Send matrix sizes to slave
    if (send(slave_sock, &rows, sizeof(int), 0) == -1)
    {
        perror("Error sending matrix row size to slave");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }
    if (send(slave_sock, &cols, sizeof(int), 0) == -1)
    {
        perror("Error sending matrix column size to slave");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    // send the y_vector values
    if (send(slave_sock, y_vector, rows * sizeof(double), 0) == -1)
    {
        perror("Error sending y vector data to slave");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    // Send the matrix row by row
    for (int i = 0; i < rows; i++)
    {
        if (send(slave_sock, matrix[i], cols * sizeof(double), 0) == -1)
        {
            perror("Error sending matrix row to slave");
            close(slave_sock);
            exit(EXIT_FAILURE);
        }
    }

    // receive sub v pearson from slave and copy values to master vector
    double *subvector = malloc(cols * sizeof(double));
    if (recv(slave_sock, subvector, cols * sizeof(double), MSG_WAITALL) == -1)
    {
        perror("Error receiving sub v from slave");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    // printf("\nHERE!\n");
    // for (size_t i = 0; i < cols; i++)
    // {
    //     printf("%f ", subvector[i]);
    // }

    // Assign the received subvector to the appropriate positions in v
    for (size_t i = 0; i < cols; i++)
    {
        v[start_col + i] = subvector[i];
    }

    // receive acknoeldgement message from slave
    char ack[16];
    if (recv(slave_sock, ack, sizeof(ack), MSG_WAITALL) == -1)
    {
        perror("Error receiving acknowledgment from slave");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    printf(" %s\n", ack);

    close(slave_sock);
    pthread_exit(NULL);
}

// Master function
void master(int n, int p, int t)
{
    printf("Starting master...\n");

    // Create matrix
    double **matrix = generate_matrix(n);
    double *y_vector = generate_vector(n);

    // double **matrix = sample_matrix(n);
    // double *y_vector = sample_vector(n);

    // Socket creation and binding
    int master_sock, slave_sock;
    struct sockaddr_in master_addr, slave_addr;
    socklen_t slave_size;
    master_sock = socket(AF_INET, SOCK_STREAM, 0);

    if (master_sock < 0)
    {
        perror("Error while creating socket");
        close(master_sock);
        exit(EXIT_FAILURE);
    }

    printf("Socket created successfully\n");

    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(p);
    master_addr.sin_addr.s_addr = INET_AD;

    if (bind(master_sock, (struct sockaddr *)&master_addr, sizeof(master_addr)) < 0)
    {
        perror("Couldn't bind to the port");
        close(master_sock);
        exit(EXIT_FAILURE);
    }
    printf("Done with binding\n");

    // Turn on the socket to listen for incoming connections
    if (listen(master_sock, 50) < 0)
    {
        perror("Listen failed");
        close(master_sock);
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
    // int num_processors = sysconf(_SC_NPROCESSORS_ONLN);
    double *v = malloc(n * sizeof(double));
    // cpu_set_t cpuset;

    gettimeofday(&time_before, NULL);
    for (int i = 0; i < t; i++)
    {
        slave_size = sizeof(slave_addr);
        slave_sock = accept(master_sock, (struct sockaddr *)&slave_addr, &slave_size);
        if (slave_sock < 0)
        {
            perror("Can't accept");
            close(master_sock);
            close(slave_sock);
            exit(EXIT_FAILURE);
        }

        args[i].matrix = matrix;
        args[i].y_vector = y_vector;
        args[i].v = v;
        args[i].start_col = start_col;
        args[i].cols = cols_per_thread + ((i == t - 1) ? extra_cols : 0);
        args[i].rows = n;
        args[i].slave_sock = slave_sock;
        // args[i].cpu_core = i % (num_processors - 1);

        // CPU_ZERO(&cpuset);
        // CPU_SET(args[i].cpu_core, &cpuset);
        pthread_create(&tid[i], NULL, distribute_matrix, (void *)&args[i]);
        // pthread_setaffinity_np(tid[i], sizeof(cpu_set_t), &cpuset);
        start_col += args[i].cols;
    }

    // Wait for all threads to finish
    for (int i = 0; i < t; i++)
    {
        pthread_join(tid[i], NULL);
    }
    gettimeofday(&time_after, NULL);

    double elapsed_time = (time_after.tv_sec - time_before.tv_sec) + (time_after.tv_usec - time_before.tv_usec) / 1e6;

    // print_matrix(matrix, n, n);

    // printf("Y Vector:\n");
    // for (size_t i = 0; i < n; i++)
    // {
    //     printf("%f ", y_vector[i]);
    // }
    // printf("\n");

    // printf("Pearson Vector:\n");
    // for (size_t i = 0; i < n; i++)
    // {
    //     printf("%f ", v[i]);
    // }
    // printf("\n");

    printf("\nTime elapsed: %.6f seconds\n", elapsed_time);

    // Clean up matrix
    for (int i = 0; i < n; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
    free(y_vector);
    free(v);
}

// Slave function
void slave(int n, int p, int t)
{
    // Socket creation and connection to master
    int slave_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (slave_sock < 0)
    {
        perror("Unable to create socket");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    // get the master sock details
    struct sockaddr_in master_addr;
    master_addr.sin_family = AF_INET;
    master_addr.sin_port = htons(p);
    master_addr.sin_addr.s_addr = INET_AD;

    // keep trying to connect slave to master
    while (1)
    {
        if (connect(slave_sock, (struct sockaddr *)&master_addr, sizeof(master_addr)) < 0)
        {
            perror("\nUnable to connect. Retrying to connect...");
            sleep(7);
        }
        else
        {
            printf("\nConnected with master successfully!\n");
            break;
        }
    }

    struct timeval time_before, time_after;
    gettimeofday(&time_before, NULL);

    // Receive matrix sizes from master
    int rows, cols;
    if (recv(slave_sock, &rows, sizeof(int), MSG_WAITALL) == -1)
    {
        perror("Error receiving matrix size from master");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }
    if (recv(slave_sock, &cols, sizeof(int), MSG_WAITALL) == -1)
    {
        perror("Error receiving matrix size from master");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    // receive y vector
    double *sub_yvector = malloc(n * sizeof(double));
    if (recv(slave_sock, sub_yvector, n * sizeof(double), MSG_WAITALL) == -1)
    {
        perror("Error receiving y vector data from master");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    // receive submatrix
    double **submatrix = malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++)
    {
        submatrix[i] = malloc(cols * sizeof(double));
        if (recv(slave_sock, submatrix[i], cols * sizeof(double), MSG_WAITALL) == -1)
        {
            perror("Error receiving matrix data from master");
            close(slave_sock);
            exit(EXIT_FAILURE);
        }
    }

    double *subv = malloc(cols * sizeof(double));
    pearson_cor(submatrix, sub_yvector, rows, cols, subv);

    if (send(slave_sock, subv, cols * sizeof(double), 0) == -1)
    {
        perror("Error sending sub v to master");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }

    // Send acknowledgment to master
    char ack[] = "ACK from slave";
    if (send(slave_sock, ack, sizeof(ack), 0) == -1)
    {
        perror("Error sending acknowledgment to master");
        close(slave_sock);
        exit(EXIT_FAILURE);
    }
    gettimeofday(&time_after, NULL);

    double elapsed_time = (time_after.tv_sec - time_before.tv_sec) + (time_after.tv_usec - time_before.tv_usec) / 1e6;

    // Print received submatrix
    // printf("Received Submatrix:\n");
    // print_matrix(submatrix, rows, cols);
    // printf("\nY vector in slave:\n");
    // for (size_t i = 0; i < rows; i++)
    // {
    //     printf("%f ", sub_yvector[i]);
    // }

    // printf("\nSub pearson vector:\n");
    // for (size_t i = 0; i < cols; i++)
    // {
    //     printf("%f ", subv[i]);
    // }

    printf("\n%.6f seconds\n", elapsed_time);

    // Clean up submatrix
    for (int i = 0; i < rows; i++)
    {
        free(submatrix[i]);
    }
    free(submatrix);
    free(sub_yvector);
    free(subv);

    close(slave_sock);
}

// Function to read configuration from a file
void read_config(const char *filename, int *n, int *p, int *t)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    if (fscanf(file, "%d %d %d", n, p, t) != 3)
    {
        perror("Error reading configuration");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

int main(int argc, char *argv[])
{
    int n, p, s, t;

    // Check if the correct number of arguments is provided
    if (argc != 3)
    {
        printf("Usage: %s [0 for master, 1 for slave] [config file]\n", argv[0]);
        return 1;
    }

    // Convert the first argument to an integer
    s = atoi(argv[1]);

    // Read the configuration from the file
    read_config(argv[2], &n, &p, &t);

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