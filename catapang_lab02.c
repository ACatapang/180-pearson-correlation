#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define MAX_NUM 100 // Maximum value for random numbers

// Structure for passing arguments to thread function
typedef struct
{
    int **X;
    int *y;
    int n;
    int start_row;
    int rows_per_thread;
    double *result;
} ThreadArgs;

// Function to calculate Pearson correlation coefficient
double pearson_cor(int **X, int *y, int n, int start_row, int rows_per_thread)
{
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
    int end_row = start_row + rows_per_thread;

    for (int i = start_row; i < end_row; i++)
    {
        for (int j = 0; j < n; j++)
        {
            sum_x += X[i][j];
            sum_y += y[j];
            sum_xy += X[i][j] * y[j];
            sum_x2 += X[i][j] * X[i][j];
            sum_y2 += y[j] * y[j];
        }
    }

    double numerator = n * sum_xy - sum_x * sum_y;
    double denominator = sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));

    if (denominator == 0)
        return 0;
    else
        return numerator / denominator;
}

// Thread function
void *thread_func(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg;
    args->result = pearson_cor(args->X, args->y, args->n, args->start_row, args->rows_per_thread);
    pthread_exit(NULL);
}

int main()
{
    int n, t;

    // printf("Enter the size of the square matrix and the number of threads (n t): ");
    scanf("%d %d", &n, &t);

    if (n <= 0 || t <= 0 || n < t)
    {
        printf("Invalid input.\n");
        return 1;
    }

    // Initialize random seed
    srand(time(NULL));

    // Allocate memory for matrix X
    int **X = (int **)malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
    {
        X[i] = (int *)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++)
        {
            X[i][j] = rand() % MAX_NUM + 1; // Random non-zero integers
        }
    }

    // Allocate memory for vector y
    int *y = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++)
    {
        y[i] = rand() % MAX_NUM + 1; // Random non-zero integers
    }

    // Divide matrix X into t submatrices
    int rows_per_thread = n / t;

    // Array to hold thread IDs
    pthread_t threads[t];

    // Array to hold thread arguments
    ThreadArgs args[t];

    clock_t time_before = clock();

    // Create threads
    for (int i = 0; i < t; i++)
    {
        args[i].X = X;
        args[i].y = y;
        args[i].n = n;
        args[i].start_row = i * rows_per_thread;
        args[i].rows_per_thread = rows_per_thread;

        pthread_create(&threads[i], NULL, thread_func, &args[i]);
    }

    // Join threads and accumulate results
    double r = 0;
    for (int i = 0; i < t; i++)
    {
        pthread_join(threads[i], NULL);
        r += args[i].result;
    }
    r /= t;

    clock_t time_after = clock();

    // Calculate elapsed time in seconds
    double time_elapsed = (double)(time_after - time_before) / CLOCKS_PER_SEC;

    // Output time_elapsed and result
    printf("Matrix Size: %d \t Thread Count: %d \t Time elapsed: %.6f seconds\n", n, t, time_elapsed);

    // Free allocated memory
    for (int i = 0; i < n; i++)
    {
        free(X[i]);
    }
    free(X);
    free(y);

    return 0;
}
