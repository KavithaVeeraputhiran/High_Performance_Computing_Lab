#include <stdio.h>
#include <omp.h>

int main() {
    int i, n = 10000;
    int a[n];

    for(i = 0; i < n; i++) {
        a[i] = i * 2;
    }

    // Open file for writing
    FILE *fptr = fopen("schedule_results.txt", "w");
    if (fptr == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    double start = omp_get_wtime();

    // Note: Writing to a file inside a parallel loop can be slow
    // because the file system becomes a bottleneck (race for the file lock).
    #pragma omp parallel for schedule(static, 2)
    for(i = 0; i < n; i++) {
        // Critical section ensures threads don't scramble the text lines
        #pragma omp critical
        {
            fprintf(fptr, "Thread %d processed element %d\n", omp_get_thread_num(), i);
        }
        a[i] = a[i] + 5;
    }

    double end = omp_get_wtime();

    fprintf(fptr, "\nTime: %f sec \n", end - start);

    fprintf(fptr, "\nFinal Array:\n");
    for(i = 0; i < n; i++) {
        fprintf(fptr, "%d ", a[i]);
    }

    fclose(fptr);
    printf("All logs and array data written to schedule_results.txt\n");

    return 0;
}