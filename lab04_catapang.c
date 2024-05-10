#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CONFIG_FILE "config.txt"
#define MAX_IP_LENGTH 16
#define MAX_SLAVE_INFO 100

// allocate memory and assign values to matrix of size n x n
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

// read config file
int read_config(char slave_info[][MAX_IP_LENGTH], int *slave_ports)
{
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file)
    {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }
    int num_slaves;
    fscanf(file, "%d", &num_slaves);
    for (int i = 0; i < num_slaves; i++)
    {
        fscanf(file, "%s %d", slave_info[i], &slave_ports[i]);
    }
    fclose(file);
    return num_slaves;
}

// Function for master to distribute submatrices to slaves
void distribute_to_slaves(int **matrix, int n, int num_slaves, char slave_info[][MAX_IP_LENGTH], int *slave_ports)
{
    struct sockaddr_in slave_addr;
    int sockfd, addrlen = sizeof(struct sockaddr_in);
    char ack[4] = "ack";

    for (int i = 0; i < num_slaves; i++)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        memset(&slave_addr, 0, sizeof(slave_addr));
        slave_addr.sin_family = AF_INET;
        slave_addr.sin_port = htons(slave_ports[i]);

        if (inet_pton(AF_INET, slave_info[i], &slave_addr.sin_addr) <= 0)
        {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        if (connect(sockfd, (struct sockaddr *)&slave_addr, sizeof(slave_addr)) < 0)
        {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        // Send submatrix to slave
        int start_row = i * (n / num_slaves);
        int end_row = (i + 1) * (n / num_slaves);
        for (int row = start_row; row < end_row; row++)
        {
            send(sockfd, matrix[row], n * sizeof(int), 0);
        }

        // Receive acknowledgment from slave
        recv(sockfd, ack, 4, 0);
        printf("Received acknowledgment from slave %d\n", i);
        close(sockfd);
    }
}

// Function for slave to receive submatrix from master
void receive_from_master(int sockfd, int **submatrix, int n)
{
    for (int row = 0; row < n; row++)
    {
        recv(sockfd, submatrix[row], n * sizeof(int), 0);
    }
}

// Function for slave to send acknowledgment to master
void send_acknowledgment(int sockfd)
{
    char ack[4] = "ack";
    send(sockfd, ack, 4, 0);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Usage: %s <n> <p> <s>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]); // matrix size
    int p = atoi(argv[2]); // port number
    int s = atoi(argv[3]); // master or slave

    srand(time(NULL));

    if (s == 0)
    {
        // Master instance
        int **matrix = generate_matrix(n);

        char slave_info[MAX_SLAVE_INFO][MAX_IP_LENGTH];
        int slave_ports[MAX_SLAVE_INFO];
        // int num_slaves = read_config(slave_info, slave_ports);
        int num_slaves = 3;

        struct timeval time_before, time_after;

        gettimeofday(&time_before, NULL);

        distribute_to_slaves(matrix, n, num_slaves, slave_info, slave_ports);

        gettimeofday(&time_after, NULL);

        double elapsed_time = (time_after.tv_sec - time_before.tv_sec) +
                              (time_after.tv_usec - time_before.tv_usec) / 1e6;

        printf("Time elapsed: %.6f seconds\n", elapsed_time);

        // Free allocated memory for matrix
        for (int i = 0; i < n; i++)
        {
            free(matrix[i]);
        }
        free(matrix);
    }
    else if (s == 1)
    {
        // Slave instance
        int **submatrix = malloc(n * sizeof(int *));
        for (int i = 0; i < n; i++)
        {
            submatrix[i] = malloc(n * sizeof(int));
        }

        int sockfd, new_sockfd;
        struct sockaddr_in serv_addr, cli_addr;
        int addrlen = sizeof(struct sockaddr_in);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(p);

        if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("Binding failed");
            exit(EXIT_FAILURE);
        }

        if (listen(sockfd, 1) < 0)
        {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

        new_sockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&addrlen);
        if (new_sockfd < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        receive_from_master(new_sockfd, submatrix, n);
        printf("Received submatrix from master\n");
        send_acknowledgment(new_sockfd);

        // Free allocated memory for submatrix
        for (int i = 0; i < n; i++)
        {
            free(submatrix[i]);
        }
        free(submatrix);

        close(new_sockfd);
        close(sockfd);
    }
    else
    {
        printf("Invalid value for status. Must be 0 for master or 1 for slave.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
