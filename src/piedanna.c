#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//gcc -std=c99 -o bin/piedanna src/piedanna.c -lm

//Utils
struct array
{
    unsigned long size;
    long *val;
};

void printArray(char name[], struct array *array)
{
    printf("==== %s [%li] ====\n", name, array->size);
    for (unsigned long i = 0; i < array->size; ++i) {
        printf("%li ", array->val[i]);
    }
    printf("\n");
}

void subArray(struct array *array, unsigned long start, unsigned long end)
{
    array->size = end-start;
    array->val = array->val + start;
}

void invertArray(struct array *array)
{
    long tmp = 0;
    for (unsigned long i = 0; i < array->size / 2; i++)
    {
        tmp = array->val[i];
        array->val[i] = array->val[array->size - i - 1];
        array->val[array->size - i - 1] = tmp;
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
void loadArrayFromFile(struct array *array, unsigned long maxSize, char fileName[])
{
    array->val = malloc(maxSize*sizeof(unsigned long));

    FILE *file = fopen(fileName, "r");

    if (!file) {
        printf("File %s cannot be opened\n", fileName);
        exit(0);
    }

    while(fscanf(file, "%ld", &array->val[array->size]) != EOF) {
        array->size++;
    }

    array->val = realloc(array->val, array->size * sizeof(unsigned long));

    fclose(file);    
}

void initTmpArray(struct array *array, struct array *upwardArray, long neutral)
{
    unsigned long size = array->size*2;
    upwardArray->size = size;
    upwardArray->val = malloc(size * sizeof(long));

    for (unsigned long i = 0; i < size; i++)
    {
        if (i < size/2) {
            upwardArray->val[i] = neutral;
        } else {
            upwardArray->val[i] = array->val[i - array->size];
        }
    }
}

void initArray(struct array *array, unsigned long size, long neutral)
{
    array->size = size;
    array->val = malloc(size * sizeof(long));

    for (unsigned long i = 0; i < size; i++)
    {
        array->val[i] = neutral;
    }
}

//Core
void upward(struct array *array, unsigned long m, long (*operation)(long, long))
{
    for (long l = m - 1; l >= 0; l--) {
        for (unsigned long j = pow(2, l); j <= (pow(2, l+1) - 1); j++) {
            array->val[j] = operation(array->val[2 * j], array->val[2 * j + 1]);
        }
    }
}

void downward(struct array *tmpArray, struct array *array, long m, long (*operation)(long, long))
{
    for (long l = 1; l <= m; l++) {
        for (unsigned long j = pow(2, l); j <= (pow(2, l + 1) - 1); j++) {
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
    for (unsigned long j = pow(2, m); j <= (pow(2, m + 1) - 1); j++) {
        array->val[j] = operation(tmpArray->val[j], array->val[j]);
    }
}

void prefix(struct array *q, struct array *prefixArray, long (*operation)(long, long), long neutral)
{
    struct array *tmpArray = malloc(sizeof(struct array));
    initTmpArray(q, tmpArray, neutral);
    
    unsigned long m = log2(q->size);
    upward(tmpArray, m, operation);

    initArray(prefixArray, q->size*2, neutral);
    downward(tmpArray, prefixArray, m, operation);

    final(tmpArray, prefixArray, m, operation);

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
    initArray(m, q->size, 0);

    long ms;
    long mp;
    
    for (unsigned long i = 0; i <= q->size; i++)
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
    for (unsigned long i = 0; i <= m->size; i++)
    {
        if (m->val[i] > maxval) {
            maxval = m->val[i];
            bestIndex = i;
        }
    }

    printf("%li ", maxval);
    for (unsigned long i = bestIndex; m->val[i] == maxval; i++)
    {
        printf("%li ", q->val[i]);
    }
    printf("\n");
}

void findMaxSubArray(char fileName[])
{
    //Initialisation
    unsigned long maxSize = 100000;
    struct array *q = malloc(sizeof(struct array));
    loadArrayFromFile(q, maxSize, fileName);

    //PSUM
    struct array *pSum = malloc(sizeof(struct array));
    prefix(q, pSum, plus, 0);

    //SSUM
    struct array *sSum = malloc(sizeof(struct array));
    suffix(q, sSum, plus, 0);
    
    //SMAX
    struct array *sMax = malloc(sizeof(struct array));
    suffix(pSum, sMax, max, 0);
    
    //PMAX
    struct array *pMax = malloc(sizeof(struct array));
    prefix(sSum, pMax, max, 0);

    //M
    struct array *m = malloc(sizeof(struct array));
    buildM(q, pSum, sSum, sMax, pMax, m);

    //Print Result
    printResult(q,m);
}

int main(int argc, char **argv)
{
    // Calculate the time taken by fun() 
    clock_t t; 
    t = clock(); 
    findMaxSubArray(argv[1]);
    t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC;
  
    printf("Time: %f\n", time_taken); 
    return 0; 
}