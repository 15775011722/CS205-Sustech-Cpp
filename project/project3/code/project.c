#pragma GCC optimize(3)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h> // Include header for OpenMP

#define LINE_MAX 1024
#define STRUCTURES_LENGTH 10
#define SEPARATOR " "
// #define STRUCTURES_FILE "data_structures.txt"
// #define KERNEL_FILE "kernel.txt"
// #define INPUTS_FILE "inputs.txt"
// #define OUTPUTS_FILE "outputs.txt"
#define STRUCTURES_FILE "3_structures.txt" // 1,2,3,big
#define KERNEL_FILE "3_kernel.txt"  // 1,2,3,big
#define OUTPUTS_FILE "3_outputs.txt" // 1,2,3,big
#define INPUTS_FILE "big_inputs.txt"


typedef struct {
    float *data;
    struct {
        size_t n;      // 输入/输出的数量，或卷积核的过滤器数量
        size_t channel;// 输入/输出/卷积核通道数
        size_t width;  // 输入/输出/卷积核宽
        size_t height; // 输入/输出/卷积核高
    } shape;
} Tensor;

/* 该方法包含内存申请，请在使用完后释放 */
Tensor *create_tensor(float *data, const size_t n, const size_t channel, const size_t width, const size_t height);
int read_file(const char *path, float *array);
int write_file(const char *path, float *array, const size_t o_n, const size_t o_c, const size_t o_w, const size_t o_h);
void convolution(Tensor *inputs, Tensor *kernel, Tensor *outputs, int padding, int strides);
int conv2D(float *image, int i_w, int i_h, float *kernel, int k_w, int k_h, float *output, int strides);

int main() {
    // printf("0\n");
    // return 0;
    // 读取数据结构
    float data_structures[STRUCTURES_LENGTH] = {0}; 
    read_file(STRUCTURES_FILE, &data_structures[0]);
    // 构建数据
    int padding = data_structures[0], strides = data_structures[1];
    size_t i_n = data_structures[2], i_c = data_structures[3], i_w = data_structures[4], i_h = data_structures[5];
    size_t k_n = data_structures[6], k_c = data_structures[7], k_w = data_structures[8], k_h = data_structures[9];
    size_t o_n = i_n, o_c = k_n, o_w = (i_w+(padding<<1)-k_w)/strides+1, o_h = (i_h+(padding<<1)-k_h)/strides+1;
    size_t i_size = i_n*i_c*i_w*i_h, k_size = k_n*k_c*k_w*k_h, o_size = o_n*o_c*o_w*o_h;
    float *inputs = (float*)malloc(i_size*sizeof(float));
    float *kernel = (float*)malloc(k_size*sizeof(float));
    float *outputs = (float*)malloc(o_size*sizeof(float));
    memset(outputs, 0.0, sizeof(o_size*sizeof(float)));
    // 读取inputs和卷积核
    read_file(INPUTS_FILE, inputs);
    read_file(KERNEL_FILE, kernel);
    // 创建Tensor变量
    Tensor *in = create_tensor(inputs, i_n, i_c, i_w, i_h);
    Tensor *ke = create_tensor(kernel, k_n, k_c, k_w, k_h);
    Tensor *out = create_tensor(outputs, o_n, o_c, o_w, o_h);
    // 预热
    convolution(in, ke, out, padding, strides);
    // 计时&运行卷积操作
    clock_t start, end;
    double duration;
    start = clock();
    convolution(in, ke, out, padding, strides);
    end = clock();
    duration = (double)(end - start) / 1000;// / CLOCKS_PER_SEC;
    // conv2D(in->data, in->shape.width, in->shape.height, 
    //        ke->data, ke->shape.width, ke->shape.height, 
    //        outputs, strides);
    // 输出结果，当结果<30时才打印在终端(多了就不好看了)
    if (o_size < 30) {
        for(int i = 0; i < o_size; i++) {
            printf("%f ", outputs[i]);
            if((i+1) % o_h == 0)
                printf("\n");
            if((i+1) % (o_w*o_h) == 0)
                printf("\n");
        }
    }
    write_file(OUTPUTS_FILE, outputs, o_n, o_c, o_w, o_h);
    printf("耗时：%fms\n", duration);
    // 释放内存
    free(in);
    free(ke);
    free(out);
    free(inputs);
    free(kernel);
    free(outputs);
    return 0;
}

Tensor *create_tensor(float *data, const size_t n, const size_t channel, const size_t width, const size_t height) {
    /* alloc memory */
    size_t size = n * channel * width * height;
    Tensor *tensor = (Tensor*)malloc(sizeof(float*)+4*sizeof(size_t));
    tensor->data = data;
    tensor->shape.n = n, tensor->shape.channel = channel, 
    tensor->shape.width = width, tensor->shape.height = height;
    return tensor;
}

int read_file(const char *path, float *array) {
    FILE *fp = fopen(path, "r");
    char* temp;
    if(fp == NULL) {
		printf("文件读取失败。\n");
		return 1;
	}
    char buf[LINE_MAX] = {0};
    size_t line_num = 0, line_len = 0;
    // 读行
    while (fgets(buf, LINE_MAX, fp)) {
        line_len = strlen(buf);
        // 去除末尾换行
        if (buf[line_len-1] == '\n')
            buf[--line_len] = '\0';
        if(buf[line_len-1] == '\r')
            buf[--line_len] = '\0';
        // 长度为0，空行
        if (line_len == 0) 
            continue;
        // 分割字符串
        temp = strtok(buf, SEPARATOR);
        while (temp) {
            if(sizeof(temp) < sizeof(char)) 
                continue;
            // *array = atoi(temp);  // char*转int
            *array = atof(temp);  //  char*转float
            array++;
            temp = strtok(NULL, SEPARATOR);
        }
        memset(buf, 0, sizeof(buf));  // 清空缓存
    }
	fclose(fp);
    return 0;
}

int write_file(const char *path, float *array, const size_t o_n, const size_t o_c, const size_t o_w, const size_t o_h) {
    FILE *fp = fopen(path, "w");
    if(fp == NULL) {
		printf("文件读取失败。\n");
		return -1;
	}
    size_t one_block_size = o_w*o_h;
    size_t size = o_n*o_c*one_block_size;
    for (size_t i = 1; i <= size; i++) {
        fprintf(fp, "%f ", *array);
        if(i % o_h == 0)
            fprintf(fp, "\n");
        if(i % one_block_size == 0)
            fprintf(fp, "\n");
        array++;
    }
	fclose(fp);
    return 1;
}

void convolution(Tensor *inputs, Tensor *kernel, Tensor *outputs, int padding, int strides) {
    size_t i_w = inputs->shape.width+(padding<<1), i_h = inputs->shape.height+(padding<<1), i_c = inputs->shape.channel;
    size_t k_w = kernel->shape.width, k_h = kernel->shape.height, k_c = kernel->shape.channel;
    size_t o_w = outputs->shape.width, o_h = outputs->shape.height, o_c = outputs->shape.channel, o_n = outputs->shape.n;
    // float (*input_data)[i_w][i_h] = (float (*)[i_w][i_h])malloc(i_c*i_w*i_h*sizeof(float));
    float (*input_data)[i_w][i_h] = (float (*)[i_w][i_h])inputs->data;
    // float input_data[i_c][i_w][i_h];
    int inputs_martrix_size = i_w*i_h;
    int kernel_martrix_size = k_w*k_h;
    int kernel_size = k_c*kernel_martrix_size;
    int outputs_martrix_size = o_w*o_h;
    int outputs_size = o_c*outputs_martrix_size;
    float *tmp = inputs->data;
    float *inputs_pointer = inputs->data, 
          *kernel_pointer = kernel->data, 
          *outputs_pointer= outputs->data;
    // #pragma omp parallel for
    for (size_t n = 0; n < o_n; n++) {
        // 当没有padding时可以注释掉，有一定的加速
        for(size_t c = 0; c < i_c; c++) {
            for(int w = -padding, row = 0, max_w=i_w-padding; w < max_w; w++, row++) {
                for(int h = -padding, col = 0, max_h=i_h-padding; h < max_h; h++, col++) {
                    if(w < 0 || w >= inputs->shape.width || h < 0 || h >= inputs->shape.height) {
                        input_data[c][row][col] = 0.0;
                    }
                    else {
                        input_data[c][row][col] = *tmp;
                        tmp++;
                        // input_data[c][row][col] = *(inputs->data+temp++);
                    }
                    // printf("input_data[%ld][%ld][%ld]=%f\n",c,row,col, input_data[c][row][col]);
                }
            }
        }
        kernel_pointer = kernel->data;  // kernel指针需要重新指回第一个kernel
        // #pragma omp parallel for num_threads(2)
        for (size_t c = 0; c < o_c; c++) {
            inputs_pointer = &input_data[0][0][0];  // inputs指针指向的是padding变换后的inputs
            for(int i = 0; i < k_c; i++) {
                conv2D(inputs_pointer, i_w, i_h, 
                       kernel_pointer, k_w, k_h, 
                       outputs_pointer, strides);
                inputs_pointer += inputs_martrix_size;
                kernel_pointer += kernel_martrix_size;
            }
            outputs_pointer += outputs_martrix_size;  // 整个大循环就是按outputs的来的，所以outputs指针会一直+，而不会重新出现=符号
        }
    }
    // free(input_data);
}

int conv2D(float *image, int i_w, int i_h, float *kernel, int k_w, int k_h, float *output, int strides) {
    float tmp = 0.0;
    // printf("max_i=%d, max_j=%d\n", i_w-k_w, i_h-k_h);
    for (int i = 0, max_i = i_w - k_w; i <= max_i; i += strides) {
        for (int j = 0, max_j = i_h - k_h; j <= max_j; j += strides) {
            tmp = 0.0;
            for(int r = 0; r < k_w; r++) {
                for (int c = 0; c < k_h; c++) {
                    // printf("image[%d][%d]*kernel[%d][%d]=%f*%f=%f\n", r+i, c+j, 
                    // r, c, *(image+(r+i)*i_h+c+j), *(kernel+r*k_h+c), 
                    // *(image+(r+i)*i_h+c+j) * *(kernel+r*k_h+c));
                    tmp += (*(image+(r+i)*i_h+c+j)) * (*(kernel+r*k_h+c));
                }
            }
            // printf("=%f\n", tmp);
            *output += tmp;
            output++;
        }
    }
    return 1;
}

