#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Function to calculate Pearson Correlation Coefficient
void pearson_cor(int n, int X[][n], int y[], double v[])
{
    int i, j;
    double sumX, sumY, sumXY, sumXsq, sumYsq;

    for (i = 0; i < n; i++)
    {
        sumX = sumY = sumXY = sumXsq = sumYsq = 0.0;

        for (j = 0; j < n; j++)
        {
            sumX += X[i][j];
            sumY += y[j];
            sumXY += X[i][j] * y[j];
            sumXsq += X[i][j] * X[i][j];
            sumYsq += y[j] * y[j];
        }

        double numerator = (n * sumXY) - (sumX * sumY);
        double denominator = sqrt((n * sumXsq - sumX * sumX) * (n * sumYsq - sumY * sumY));

        if (denominator == 0)
            v[i] = 0; // Handle division by zero
        else
            v[i] = numerator / denominator;
    }
}

int main()
{
    int n, i, j;
    printf("Enter the size of the square matrix and vector (n): ");
    scanf("%d", &n);

    // Allocate memory for the square matrix X and vector y
    int X[n][n], y[n];

    // Generate random non-zero values for X and y
    srand(time(NULL)); // Seed for random number generation
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            X[i][j] = rand() % 20 + 1; // Random integers between 1 and 20
        }
        y[i] = rand() % 20 + 1; // Random integers between 1 and 20
    }

    // Allocate memory for vector v
    double v[n];

    // Measure time before computation
    clock_t time_before = clock();

    // Calculate Pearson Correlation Coefficient
    pearson_cor(n, X, y, v);

    // Measure time after computation
    clock_t time_after = clock();

    // Calculate elapsed time
    double time_elapsed = (double)(time_after - time_before) / CLOCKS_PER_SEC;

    // Output the result
    printf("Pearson Correlation Coefficients (v):\n");
    for (i = 0; i < n; i++)
    {
        printf("v[%d] = %lf\n", i, v[i]);
    }

    // Output the elapsed time
    printf("Time Elapsed: %lf seconds\n", time_elapsed);

    return 0;
}
