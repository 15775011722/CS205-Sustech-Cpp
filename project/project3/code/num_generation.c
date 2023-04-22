#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define RANGE 1000000.0
#define SEPARATOR " "
// #define MY_FILE "big_inputs.txt"
// #define SIZE 3*3*1000*1000
/*2 2
3 3 1000 1000
2 3 5 5*/

#define MY_FILE "big_kernel.txt"
#define SIZE 2*3*5*5
/*2 2
3 3 1000 1000
2 3 5 5*/

// #define MY_FILE "1_kernel.txt"
// #define SIZE 1*1*1*1;
/*0 1
3 3 1000 1000
1 1 1 1*/

// #define MY_FILE "2_kernel.txt"
// #define SIZE 1*1*3*3
/*0 1
3 3 1000 1000
1 1 3 3*/

// #define MY_FILE "3_kernel.txt"
// #define SIZE 1*1*5*5
/*0 1
3 3 1000 1000
1 1 5 5*/

//  注：这个值得根据read_file的buf长度以及浮点数整数小数保存范围来决定
#define LINE_MAX_NUM 70 // 每个浮点数有6个整数位，6个小数位，加小数点和空格=14，1024/14≈73,所以取71,循环从0开始再-1

void generatingRandomNumber(size_t size);
int write_file(char *path, float *array, size_t size);
int main() {
    generatingRandomNumber(SIZE);
    return 0;
}

void generatingRandomNumber(size_t size) {
    float *random_array = (float*)malloc(size*sizeof(float));
    for (size_t i = 0; i < size; i++) {
        random_array[i] = ((float)rand()/(float)(RAND_MAX)) * RANGE;
    }
    write_file(MY_FILE, random_array, size);
    free(random_array);
}

int write_file(char *path, float *array, size_t size) {
    FILE *fp = fopen(path, "w");
    if(fp == NULL) {
		printf("文件读取失败。\n");
		return -1;
	}
    for (size_t i = 0; i < size; i++) {
        fprintf(fp, "%f ", *array);
        if ((i % LINE_MAX_NUM) == 0) {
            fprintf(fp, "\n");
        }
        array++;
    }
	fclose(fp);
    return 1;
}
