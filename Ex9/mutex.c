
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function to integrate: f(x) = x^2 */
double f(double x) { return x * x; }

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <thread_count>\n", argv[0]);
        return 1;
    }

    int thread_count = atoi(argv[1]);
    if (thread_count < 1) thread_count = 1;

    double a, b;
    int n;
    printf("Enter lower limit (a): ");
    if (scanf("%lf", &a) != 1) return 1;
    printf("Enter upper limit (b): ");
    if (scanf("%lf", &b) != 1) return 1;
    printf("Enter number of trapezoids (n): ");
    if (scanf("%d", &n) != 1) return 1;
    if (n <= 0) { fprintf(stderr, "n must be positive\n"); return 1; }

    double h = (b - a) / n;
    double total_sum = (f(a) + f(b)) / 2.0; // endpoints contribution
    int total_iters = n - 1; // indices i = 1..n-1

    #pragma omp parallel num_threads(thread_count)
    {
        int tid = omp_get_thread_num();
        int tcount = omp_get_num_threads();

        int base = (total_iters > 0) ? (total_iters / tcount) : 0;
        int rem  = (total_iters > 0) ? (total_iters % tcount) : 0;
        int my_count = base + (tid < rem ? 1 : 0);

        int start_idx;
        if (total_iters <= 0 || my_count == 0) {
            start_idx = 1; // unused if my_count==0
        } else if (tid < rem) {
            start_idx = tid * (base + 1) + 1;
        } else {
            start_idx = rem * (base + 1) + (tid - rem) * base + 1;
        }
        int end_idx = start_idx + my_count; // one past last

        double my_real_sum = 0.0;
        int j;
        for ( j = start_idx; j < end_idx; ++j) {
            my_real_sum += f(a + j * h);
        }

        double reported_value;
        int is_auxiliary = 0;
        if (my_count > 0) {
            reported_value = my_real_sum;
        } else {
            // Thread had no real iterations: give it one auxiliary evaluation
            // Choose a point inside [a,b] deterministically per thread so output is stable
            double x_aux;
            if (total_iters > 0) {
                // pick a point corresponding to an existing index (wrap around)
                int pick = 1 + (tid % total_iters);
                x_aux = a + pick * h;
            } else {
                // degenerate case n==1: pick midpoint
                x_aux = (a + b) / 2.0;
            }
            reported_value = f(x_aux);
            is_auxiliary = 1;
        }

        #pragma omp critical
        {
            if (is_auxiliary)
                printf("Thread %d auxiliary (not added) contribution: %f\n", tid, reported_value);
            else {
                printf("Thread %d partial sum contribution: %f\n", tid, reported_value);
                total_sum += my_real_sum;
            }
        }
    } // end parallel

    double final_integral = total_sum * h;
    printf("\nFinal value of the integral = %f\n", final_integral);

    return 0;
}