#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>
#include <limits.h>
#include <time.h>

//gcc -std=c99 -o bin/piedanna-fopenmp src/piedanna.c -lm -fopenmp

//Utils
struct array
{
    unsigned long size;
    unsigned long trueSize;
    long *val;
};

void subArray(struct array *array, unsigned long start, unsigned long end)
{
    array->size = end-start;
    memcpy(array->val, &array->val[start], (end-start) * sizeof(long));
}

void invertArray(struct array *array)
{
    #pragma omp parallel for
    for (unsigned long i = 0; i < array->size/2; i++) {
        long temp = array->val[i];
        array->val[i] = array->val[array->size - i - 1];
        array->val[array->size - i - 1] = temp;
    }
}

//Operations
long plus(long x, long y)
{
    return x + y;
}

long max(long x, long y)
{
    return x > y ? x : y;
}

//Initilialisation
void loadArrayFromFile(struct array *array, char fileName[])
{
    FILE *file = fopen(fileName, "r");

    if (!file) {
        printf("File %s cannot be opened\n", fileName);
        exit(0);
    }
    
    array->trueSize = 0;
    array->size = 16;
    array->val = malloc(array->size * sizeof(long));
    
    while (1) {
        long scan = fscanf(file, "%ld", &array->val[array->trueSize]);
        if (scan == EOF) break;
        array->trueSize++;
        if(array->trueSize > array->size-1) {
            array->size = array->size * 2;
            array->val = realloc(array->val, array->size * sizeof(long));
        }
    }

    if (array->size != array->trueSize) {
        #pragma omp parallel for
        for(unsigned long i = array->trueSize; i < array->size; i++) {
            array->val[i] = 0;
        }
    }

    fclose(file);
}

void initTmpArray(struct array *array, struct array *upwardArray, long neutral)
{
    unsigned long size = array->size*2;
    upwardArray->size = size;
    upwardArray->trueSize = array->trueSize;
    upwardArray->val = malloc(size * sizeof(long));

    #pragma omp parallel for
    for (unsigned long i = 0; i < size; i++) {
        if (i < size/2) {
            upwardArray->val[i] = neutral;
        } else {
            upwardArray->val[i] = array->val[i - array->size];
        }
    }
}

void initArray(struct array *array, unsigned long size, unsigned long trueSize, long neutral)
{
    array->size = size;
    array->trueSize = trueSize;
    array->val = malloc(size * sizeof(long));

    #pragma omp parallel for
    for (unsigned long i = 0; i < size; i++) {
        array->val[i] = neutral;
    }
}

//Core
void upward(struct array *array, unsigned long m, long (*operation)(long, long))
{
    for (long l = m - 1; l >= 0; l--) {
        #pragma omp parallel for
        for (unsigned long j = (unsigned long) pow(2, l); j <= ((unsigned long) pow(2, l+1) - 1); j++) {
            array->val[j] = operation(array->val[2 * j], array->val[2 * j + 1]);
        }
    }
}

void downward(struct array *tmpArray, struct array *array, long m, long (*operation)(long, long))
{
    for (long l = 1; l <= m; l++) {
        #pragma omp parallel for
        for (unsigned long j = (unsigned long) pow(2, l); j <= ((unsigned long) pow(2, l + 1) - 1); j++) {
            if ((j % 2) == 0) {
                array->val[j] = array->val[j / 2];
            } else {
                array->val[j] = operation(array->val[(j - 1) / 2], tmpArray->val[j - 1]);
            }
        }
    }
}

void final(struct array *tmpArray, struct array *array, long m, long (*operation)(long, long))
{
    #pragma omp parallel for
    for (unsigned long j = (unsigned long) pow(2, m); j <= ((unsigned long) pow(2, m + 1) - 1); j++) {
        array->val[j] = operation(tmpArray->val[j], array->val[j]);
    }
}

void prefix(struct array *q, struct array *prefixArray, long (*operation)(long, long), long neutral)
{
    struct array *tmpArray = malloc(sizeof(struct array));
    initTmpArray(q, tmpArray, neutral);
    
    unsigned long m = log2(q->size);
    upward(tmpArray, m, operation);

    initArray(prefixArray, q->size*2, q->trueSize, neutral);
    downward(tmpArray, prefixArray, m, operation);

    final(tmpArray, prefixArray, m, operation);

    free(tmpArray->val);
    free(tmpArray);

    subArray(prefixArray, prefixArray->size/2, prefixArray->size);
}

void suffix(struct array *q, struct array *suffix, long (*operation)(long, long), long neutral)
{
    invertArray(q);
    prefix(q, suffix, operation, neutral);
    invertArray(q);
    invertArray(suffix);
}

void buildM(struct array *q, struct array *pSum, struct array *sSum, struct array *sMax, struct array *pMax, struct array *m)
{
    initArray(m, q->size, q->trueSize, 0);

    long ms;
    long mp;
    
    for (unsigned long i = 0; i < q->trueSize; i++)
    {
        ms = pMax->val[i] - sSum->val[i];
        mp = sMax->val[i] - pSum->val[i];
        m->val[i] = ms + mp + q->val[i];
    }
}

void printResult(struct array *q, struct array *m)
{
    long maxval = 0;
    unsigned long bestIndex = 0;
    
    for (unsigned long i = 0; i < q->trueSize; i++)
    {
        if (m->val[i] > maxval) {
            maxval = m->val[i];
            bestIndex = i;
        }
    }
    
    printf("%li ", maxval);
    for (unsigned long i = bestIndex; i < m->trueSize && m->val[i] == maxval; i++)
    {
        printf("%li ", q->val[i]);
    }
    printf("\n");
}

void findMaxSubArray(char fileName[])
{
    //Initialisation
    struct array *q = malloc(sizeof(struct array));
    loadArrayFromFile(q, fileName);

    //PSUM
    struct array *pSum = malloc(sizeof(struct array));
    prefix(q, pSum, plus, 0);
    
    //SSUM
    struct array *sSum = malloc(sizeof(struct array));
    suffix(q, sSum, plus, 0);
    
    //SMAX
    struct array *sMax = malloc(sizeof(struct array));
    suffix(pSum, sMax, max, LONG_MIN);
    
    //PMAX
    struct array *pMax = malloc(sizeof(struct array));
    prefix(sSum, pMax, max, LONG_MIN);

    //M
    struct array *m = malloc(sizeof(struct array));
    buildM(q, pSum, sSum, sMax, pMax, m);

    free(pSum->val);
    free(pSum);
    free(sSum->val);
    free(sSum);
    free(sMax->val);
    free(sMax);
    free(pMax->val);
    free(pMax);

    //Print Result
    printResult(q,m);

    free(q->val);
    free(q);
    free(m->val);
    free(m);
}

int main(int argc, char **argv)
{
    findMaxSubArray(argv[1]);
    return 0;
}