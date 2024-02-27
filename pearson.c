#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Function to calculate Pearson Correlation Coefficient
void pearson_cor(double **X, double *y, int m, int n)
{
    double v[n];
    for (int i = 0; i < n; i++)
    {
        double x_sum = 0, y_sum = 0, x_sqr = 0, y_sqr = 0, xy = 0;
        v[i] = 0;

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
    int n;
    printf("Enter the size of matrix (n): ");
    scanf("%d", &n);

    // Allocate memory for the matrix X and vector y
    double **X = malloc(n * sizeof(double *));
    double *y = malloc(n * sizeof(double));

    // Check for memory allocation failure
    if (X == NULL || y == NULL)
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

    // double X_values[][2] = {{3.63, 3.63}, {3.02, 3.02}, {3.82, 3.82}, {3.42, 3.42}, {3.59, 3.59}, {2.87, 2.87}, {3.03, 3.03}, {3.46, 3.46}, {3.36, 3.36}, {3.3, 3.3}};
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            X[i][j] = randomNonZero();
        }
    }

    // Assign values for y
    // double y_values[] = {53.1, 49.7, 48.4, 54.2, 54.9, 43.7, 47.2, 45.2, 54.4, 50.4};
    for (int i = 0; i < n; i++)
    {
        y[i] = randomNonZero();
    }

    // Print matrix X and vector y
    // print_matrix(X, n, n);
    // print_vector(y, n);
    clock_t time_before = clock();
    pearson_cor(X, y, n, n);
    clock_t time_after = clock();

    clock_t time_elapsed = time_after - time_before;

    printf("Time Elapsed for n = %d is %f seconds", n, (double)time_elapsed / CLOCKS_PER_SEC);

    // Free allocated memory
    for (int i = 0; i < n; i++)
    {
        free(X[i]);
    }
    free(X);
    free(y);

    return 0;
}
