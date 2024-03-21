#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

// Function to calculate Pearson Correlation Coefficient
void pearson_cor(int **X, int *y, int start_col,int m, int n, double *v)
{
    for (int start_col; start_col <= n; start_col)
    {
        int x_sum = 0, y_sum = 0, x_sqr = 0, y_sqr = 0, xy = 0;
        v[i] = 0;
        for (int j = 0; j < m; j++)
        {
            x_sum += X[j][i];
            y_sum += y[j];
            x_sqr += pow(X[j][i], 2);
            y_sqr += pow(y[j], 2);
            xy += X[j][i] * y[j];
        }
        int numerator = m * xy - x_sum * y_sum;
        int denominator = sqrt((m * x_sqr - x_sum * x_sum) * (m * y_sqr - y_sum * y_sum));
        v[i] = numerator / denominator;
    }
}

typedef struct
{
    int **X;
    int *y;
    int start_col;
    int cols;
    int rows;
    double *sub_v;
} ThreadArgs;

void *threaded_pearson_cor(void *arg)
{
    ThreadArgs *thread = (ThreadArgs *)arg;
    pearson_cor(thread->X, thread->y,thread->start_col thread->rows, thread->cols, thread->sub_v);
    pthread_exit(NULL);
}

int main()
{
    int n, t;
    int **X, *y;
    double *v;
    ThreadArgs *args;
    struct timeval start, end;
    double elapsed_time;

    printf("Enter the size of the square matrix and the number of threads (n t): ");
    scanf("%d %d", &n, &t);

    if (n <= 0 || t <= 0 || n < t)
    {
        printf("Invalid input.\n");
        return 1;
    }

    printf("Allocating memory\n");
    X = malloc(n * sizeof(int *));
    y = malloc(n * sizeof(int));
    v = malloc(n * sizeof(double));
    args = malloc(t * sizeof(ThreadArgs));
    printf("Memory Allocated\n");

    if (X == NULL || y == NULL || v == NULL || args == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < n; i++)
    {
        X[i] = malloc(n * sizeof(int));
        if (X[i] == NULL)
        {
            printf("Memory allocation failed.\n");
            return 1;
        }
    }

    srand(time(NULL));

    printf("Assigning values to matrix\n");

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            X[i][j] = rand() % 100 + 1;
        }
        y[i] = rand() % 100 + 1;
    }

    printf("Done\n");

    // Get the starting time
    gettimeofday(&start, NULL);
    pthread_t tid[t];

    int cols_per_thread = n / t;
    int extra_cols = n % t;
    int start_col = 0;

    for (int i = 0; i < t; i++)
    {
        printf("Creating thread %d .\n", i);
        args[i].X = X;
        args[i].y = y;
        args[i].start_col = start_col;
        args[i].cols = cols_per_thread + ((i == t - 1) ? extra_cols : 0);
        args[i].rows = n;
        args[i].sub_v = malloc(args[i].cols * sizeof(double));

        if (args[i].sub_v == NULL)
        {
            printf("Memory allocation failed.\n");
            return 1;
        }

        pthread_create(&tid[i], NULL, threaded_pearson_cor, (void *)&args[i]);
        start_col += args[i].cols;
    }

    for (int i = 0; i < t; i++)
    {
        printf("Waiting for thread %d to complete...\n", i);
        pthread_join(tid[i], NULL);
        printf("Thread %d completed.\n", i);
        for (int j = 0; j < args[i].cols; j++)
        {
            v[j + args[i].start_col] = args[i].sub_v[j];
        }
        free(args[i].sub_v);
    }

    // Get the ending time
    gettimeofday(&end, NULL);

    // Calculate the elapsed time in seconds
    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Matrix Size: %d \t Thread Count: %d \t Time elapsed: %.6f seconds\n", n, t, elapsed_time);

    for (int i = 0; i < n; i++)
    {
        free(X[i]);
    }
    free(X);
    free(y);
    free(args);
    free(v);

    return 0;
}
