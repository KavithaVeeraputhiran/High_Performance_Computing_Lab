#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main() {
    int n = 1000000;
    int *A, *B, *C;

    A = (int*)malloc(n * sizeof(int));
    B = (int*)malloc(n * sizeof(int));
    C = (int*)malloc(n * sizeof(int));

    for(int i = 0; i < n; i++) {
        A[i] = i;
        B[i] = i * 2;
    }

    double start = omp_get_wtime();

    #pragma omp parallel for
    for(int i = 0; i < n; i++) {
        C[i] = A[i] + B[i];
    }

    double end = omp_get_wtime();

    // Open file for writing
    FILE *fptr = fopen("all_results.txt", "w");
    if (fptr == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Write the execution time header
    fprintf(fptr, "Total Time: %f sec\n", end - start);
    fprintf(fptr, "Index, Result\n");

    // Loop through all 1,000,000 elements
    for(int i = 0; i < n; i++) {
        fprintf(fptr, "%d, %d\n", i, C[i]);
    }

    fclose(fptr);
   // printf("Successfully wrote 1,000,000 lines to all_results.txt\n");

    free(A);
    free(B);
    free(C);

    return 0;
}