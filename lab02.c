#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

// Function to calculate Pearson Correlation Coefficient
// m rows x n columns
void pearson_cor(double **X, double *y, int m, int n, double *v)
{

    // column iteration
    for (int i = 0; i < n; i++)
    {
        double x_sum = 0, y_sum = 0, x_sqr = 0, y_sqr = 0, xy = 0;
        v[i] = 0;

        // row iteration
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
        v[i] = numerator / denominator;
        // printf("\nR[%d]: %f\n\tx = %f\n\ty = %f\n\tx2 = %f\n\ty2 = %f\n\txy = %f\n", i, v[i], x_sum, y_sum, x_sqr, y_sqr, xy);
    }
}

typedef struct ARG
{

    double **X;
    double *y;
    int start_col;
    int cols;
    int rows;
    double *sub_v;

} ThreadArgs;

void *threaded_pearson_cor(void *arg)
{
    ThreadArgs *thread = (ThreadArgs *)arg;
    double **X = thread->X;
    double *y = thread->y;
    int start_col = thread->start_col;
    int cols = thread->cols;
    int rows = thread->rows;
    double *sub_v = thread->sub_v;

    // get the submatrix from X
    double **sub_X = X + start_col;

    pearson_cor(sub_X, y, rows, cols, sub_v);
    pthread_exit(NULL); // thread terminates
}

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

void print_vector(double *vector, int size)
{
    printf("Vector:\n");
    for (int i = 0; i < size; i++)
    {
        printf("%f\n", vector[i]);
    }
}

int randomNonZero()
{
    int num;
    do
    {
        num = rand();
    } while (num == 0); // Repeat until num is non-zero
    return num;
}

int main()
{
    int n, t;
    double **X, *y, *v;
    ThreadArgs *args;

    printf("Enter the size of the square matrix and the number of threads (n t): ");
    scanf("%d %d", &n, &t);

    // prompt an invalid input if thread count is greater than the matrix size
    if (n <= 0 || t <= 0 || n < t)
    {
        printf("Invalid input.\n");
        return 1;
    }

    // Allocate memory for the matrix X and vector y
    X = malloc(n * sizeof(double *));
    y = malloc(n * sizeof(double));
    v = malloc(n * sizeof(double));

    // Check for memory allocation failure
    if (X == NULL || y == NULL || v == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Allocate memory for each row of X
    for (int i = 0; i < n; i++)
    {
        X[i] = malloc(n * sizeof(double));
        if (X[i] == NULL)
        {
            printf("Memory allocation failed.\n");
            return 1;
        }
    }

    // assign values to X matrix
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            X[i][j] = randomNonZero();
        }
    }

    // Assign values for y vector
    for (int i = 0; i < n; i++)
    {
        y[i] = randomNonZero();
    }

    clock_t time_before = clock();

    // create threads
    pthread_t tid[t];
    args = malloc(t * sizeof(ThreadArgs));
    if (args == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    int cols_per_thread = n / t;
    int extra_cols = n % t;
    int start_col = 0;

    for (int i = 0; i < t; i++)
    {
        args[i].X = X;
        args[i].y = y;
        args[i].start_col = start_col;
        args[i].cols = cols_per_thread + (i < extra_cols ? 1 : 0);
        args[i].rows = n;

        double *sub_v = malloc(args[i].cols * sizeof(double));
        if (sub_v == NULL)
        {
            printf("Memory allocation failed.\n");
            return 1;
        }

        args[i].sub_v = sub_v;

        pthread_create(&tid[i], NULL, threaded_pearson_cor, (void *)&args[i]);

        start_col += args[i].cols;
    }

    // join threads
    for (int i = 0; i < t; i++)
    {
        pthread_join(tid[i], NULL);
    }

    int v_index = 0;
    for (int i = 0; i < t; i++)
    {
        for (int j = 0; j < args[i].cols; j++)
        {
            v[v_index++] = args[i].sub_v[j];
        }
        free(args[i].sub_v);
    }

    clock_t time_after = clock();
    double time_elapsed = (double)(time_after - time_before) / CLOCKS_PER_SEC;
    printf("Time Elapsed for n = %d is %f seconds", n, time_elapsed);

    for (int i = 0; i < n; i++)
    {
        free(X[i]);
    }
    free(args);
    free(X);
    free(y);

    return 0;
}
