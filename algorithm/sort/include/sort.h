#ifndef __SORT_H_
#define __SORT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define ERROR -1
#define PROCESSED 0

#define MAX 100 

#define SORT 5 

#define INSERTION 1
#define MERGE 2
#define URMERGE 3
#define BUBBLE 4
#define SELECT 5

int load_file(char *filename, int *number, int *array);
void swap(int *left, int *right);
void show_data(int *array, int capacity);
void shuffle(int *array, int capacity);

void insertion_sort(int *array, int capacity);
void mergesort(int *array, int left, int right);
void mergesort_unr(int *array, int length);
void bubble_sort(int *array, int capacity);
void selection_sort(int *array, int len);

#endif
