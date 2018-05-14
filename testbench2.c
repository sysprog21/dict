#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "testbench2.h"
#define FILE_IN "cities5000.txt"
#define PREFIX_SIZE 3
static inline __attribute__((always_inline))
void get_cycles(unsigned *high, unsigned *low)
{
    asm volatile ("CPUID\n\t"
                  "RDTSC\n\t"
                  "mov %%edx, %0\n\t"
                  "movl %%eax, %1\n\t": "=r" (*high), "=r" (*low)::"%rax","%rbx","%rcx","%rdx"
                 );
}

static inline __attribute__((always_inline))
void get_cycles_end(unsigned *high, unsigned *low)
{
    asm volatile("RDTSCP\n\t"
                 "mov %%edx, %0\n\t"
                 "mov %%eax, %1\n\t"
                 "CPUID\n\t": "=r" (*high), "=r" (*low)::"%rax","%rbx","%rcx","%rdx"
                );
}

static inline __attribute__((always_inline))
uint64_t diff_in_cycles(unsigned high1, unsigned low1,
                        unsigned high2, unsigned low2)
{
    uint64_t start,end;
    start = (((uint64_t) high1 << 32) | low1);
    end = (((uint64_t) high2 << 32) | low2);
    return end - start;
}

static inline __attribute__((unused))
double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec - t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}
enum { WRDMAX = 256, LMAX = 1024 };
void prefix_search_testbench(const tst_node *root,char *outfile)
{
    char *sgl[LMAX] = {NULL};
    int sidx = 0, i = 1;
    char word[WRDMAX] = "";
    char search_word[PREFIX_SIZE] = "";
    uint64_t timec;
    unsigned timec_high1, timec_low1, timec_high2, timec_low2;
    FILE *in = fopen(FILE_IN, "r");
    FILE *out = fopen(outfile, "a");
    if (!in || !out) {
        if (!in) {
            fprintf(stderr, "error: file open failed '%s'.\n", FILE_IN);
            fclose(in);
        }
        if (!out) {
            fprintf(stderr, "error: file open failed '%s'.\n", outfile);
            fclose(out);
        }
        return;
    }
    while (fscanf(in, "%s", word) != EOF) {
        if (strlen(word) < 4) continue;
        strncpy(search_word, word, 3);


//        strncpy(search_word, word, PREFIX_SIZE);
        get_cycles(&timec_high1, &timec_low1);
        tst_search_prefix(root,search_word, sgl, &sidx, LMAX);
        get_cycles_end(&timec_high2, &timec_low2);
        timec = diff_in_cycles(timec_high1, timec_low1, timec_high2, timec_low2);
        fprintf(out, "%d %lu\n", i, timec);
        i++;
    }

    fclose(in);
    fclose(out);

    return;
}

