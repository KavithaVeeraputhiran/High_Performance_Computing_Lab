#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include<math.h>
  // You can change matrix size here (e.g., 5, 50, 100)

// Function to generate random matrix
void generateMatrix(int n, int **A) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            A[i][j] = rand() % 10;
}

// Print int ** matrix
void printMatrix(int n, int **A) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", A[i][j]);
        printf("\n");
    }
}

int main() {
    srand(time(NULL));
    int n;
    printf("enter the size of the matrix");
    scanf("%d",&n);

    // Allocate matrices dynamically
    int **A = malloc(n * sizeof(int *));
    int **B = malloc(n * sizeof(int *));
    int **add = malloc(n * sizeof(int *));
    int **sub = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        A[i] = malloc(n * sizeof(int));
        B[i] = malloc(n * sizeof(int));
        add[i] = malloc(n * sizeof(int));
        sub[i] = malloc(n * sizeof(int));
    }

    printf("Matrix size: %dx%d\n\n", n, n);

    generateMatrix(n, A);
    generateMatrix(n, B);

    // Optional: Print small matrices
    if (n <= 5) {
        printf("Matrix A:\n"); printMatrix(n, A);
        printf("\nMatrix B:\n"); printMatrix(n, B);
    }

    struct timespec start, end;

    // ================= SERIAL EXECUTION =================
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            add[i][j] = A[i][j] + B[i][j];
           // sub[i][j] = A[i][j] - B[i][j];

            if (n <= 5)
                printf("Serial: add[%d][%d]=%d, sub[%d][%d]=%d\n",
                       i,j,add[i][j],i,j,sub[i][j]);
        }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double serial_time =
        (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec)/1000000;

    if (n <= 5) {
        printf("\nSerial Addition Result:\n"); printMatrix(n, add);

    }
 clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < n; i++)
    {      for (int j = 0; j < n; j++) {
           // add[i][j] = A[i][j] + B[i][j];
           sub[i][j] = A[i][j] - B[i][j];

            if (n <= 5)
                printf("Serial: add[%d][%d]=%d, sub[%d][%d]=%d\n",
                       i,j,add[i][j],i,j,sub[i][j]);
        }
}

    clock_gettime(CLOCK_MONOTONIC, &end);
    double serial_time2 =
        (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec)/1000000;

if(n<=5){
printf("\nSerial Subtraction Result:\n"); printMatrix(n, sub);
}



    // ================= PARALLEL EXECUTION =================
    clock_gettime(CLOCK_MONOTONIC, &start);

    pid_t pid = fork();
    if (pid == 0) {
        // -------- CHILD PROCESS: subtraction --------
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                sub[i][j] = A[i][j] - B[i][j];

        if (n <= 5) {
            printf("\nChild Process: Subtraction Result:\n");
            printMatrix(n, sub);
        }
        exit(0);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double p = (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec)/1000000;

        int pid1 = fork();
        if(pid1==0) {
        // -------- PARENT PROCESS: addition --------

 clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                add[i][j] = A[i][j] + B[i][j];

        if (n <= 5) {
            printf("\nParent Process: Addition Result:\n");
            printMatrix(n, add);
        }
        wait(NULL);
   exit(0);
        }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double parallel_time2 =
        (end.tv_sec - start.tv_sec)*1000 +
        (end.tv_nsec - start.tv_nsec)/1000000;

    printf("\nExecution Time:\n");
    printf("Serial Time   : %lf seconds\n", serial_time+serial_time2);

    printf("1st  Time : %lf seconds\n", p);
     printf("2rd Time : %lf seconds\n",parallel_time2);
if(p<parallel_time2)
{
    printf("Parallel Time : %lf seconds\n",parallel_time2);
}
else
{
    printf("Parallel Time : %lf seconds\n", p);
}
    // Free memory
    for (int i = 0; i < n; i++) {
        free(A[i]); free(B[i]); free(add[i]); free(sub[i]);
    }
    free(A); free(B); free(add); free(sub);

    return 0;
}
/*[23bcs011@mepcolinux ex1a]$cc add.c -std=c99 -o add
[23bcs011@mepcolinux ex1a]$./add
enter the size of the matrix 50
Matrix size: 50x50


Execution Time:
Serial Time   : 0.000000 seconds
1st  Time : 0.000000 seconds
2rd Time : 0.000000 seconds
Parallel Time : 0.000000 seconds
[23bcs011@mepcolinux ex1a]$./add
enter the size of the matrix 1000
Matrix size: 1000x1000


Execution Time:
Serial Time   : 25.000000 seconds
1st  Time : 1.000000 seconds
2rd Time : 1.000000 seconds
Parallel Time : 1.000000 seconds
[23bcs011@mepcolinux ex1a]$./add
enter the size of the matrix 5000
Matrix size: 5000x5000


Execution Time:
Serial Time   : 562.000000 seconds
1st  Time : 15.000000 seconds
2rd Time : 42.000000 seconds
Parallel Time : 42.000000 seconds
[23bcs011@mepcolinux ex1a]$./add
enter the size of the matrix 2000
Matrix size: 2000x2000


Execution Time:
Serial Time   : 97.000000 seconds
1st  Time : 3.000000 seconds
2rd Time : 5.000000 seconds
Parallel Time : 5.000000 seconds*/