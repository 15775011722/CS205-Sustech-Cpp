# CS205 C/C++ Program Design Project3

> Name:章志轩
>
> SID:12010526
>
> Time:2023/04/20

***Abstract: 卷积神经网络（CNNs）是一种流行的图像和视频处理的深度学习技术。本项目将使用C语言实现卷积操作，并尝试提高运行运算效率。***

## I. 项目介绍

卷积、旋积或褶积(英语：Convolution)是通过两个函数f和g生成第三个函数的一种数学运算，其本质是一种特殊的积分变换，表征函数f与g经过翻转和平移的重叠部分函数值乘积对重叠长度的积分。
$$
卷积公式：(f*g)(t)=∫_{-∞}^{∞}f(τ)g(τ-t)dτ
$$
然而数学中的卷积和卷积神经网络中的卷积严格意义上是两种不同的运算。数学中卷积，主要是为了诸如信号处理、求两个随机变量和的分布等而定义的运算，所以需要根据问题的需求而确定是否”翻转”。自然运算方式是卷积核与原始的矩阵乘积，需要围绕着中心元素进行180度旋转后才是对应的元素。而卷积神经网络中“卷积”，是为了提取图像的特征，其实只借鉴了“加权求和”的特点，同时卷积核参数本身就是trainable的参数，可以人为设置也可以根据数据训练学习，那么不翻转也不会有问题。

![image-20230420203727103](https://user-images.githubusercontent.com/83866527/233762058-882a91b8-4568-4028-bca0-7a9e6ede0482.png)

$$
计算机中的卷积： g(x,y)=w(x,y)∗f(x,y)=Σ_{s=-K/2}^{K/2}Σ_{t=-K/2}^{K/2}w(s,t)f(x−s,y−t)
$$
卷积神经网络专为将图像数据映射到输出变量而设计(现已经用在众多不同领域)。它们特别擅长发掘二维图像的内部表征，可用于学习位置和尺寸不变的结构。这使得它们特别擅长处理具有空间关系组件的数据。事实证明，在涉及图像处理的应用场合，CNN 模型能够带来出色结果和超高计算效率。在本项目中，使用编程语言实现卷积操作，将有助于我加深对卷积的理解，为我在深度学习方面的学习打下扎实的基础。 卷积的实现包括 **自定义输入、卷积核形状** 以及 **padding和strides自由设置**。

## II. 环境配置&语言

系统环境Win10系统，AMD 锐龙 5 4600U处理器；

IDE：VScode-x64-1.75.1

语言：C（gcc编译器）

## III. 代码设计与实现

#### 数据结构

卷积需要设计严格的输入输出，以及卷积核的大小。通常卷积的输入(通常是图片)是个三维矩阵：长(width)，宽(height)与通道(channel)。对于通道的解释，在三原色图片中，不同像素点由三个颜色组成(红绿蓝)，通过调节各自的亮度值来组合成复杂的颜色。因此三原色RGB图片就拥有三个通道。因此输入是[i_c,i_w,i_h]。卷积核的大小可以由用户自己设置，但通常卷积核的通道数与输入一致，因此卷积核的大小为[k_c,k_w,k_h]。此时对输入进行卷积操作，不同通道单独视作一次卷积操作，输出一个[i_w-k_w+1,i_h-k_h+1]的矩阵，再将每个矩阵累加在一起，就形成了单通道的输出[1,o_w,o_h] (o_w=i_w-k_w+1,o_h=i_h-k_h+1)。但为了方便扩展，我设计了批量输入(n)，当有n个输入就有n个输出(i_n=o_n)。为 **统一数据类型**，卷积核也有n，不过卷积核的n代表的是过滤器(filter)的数量(就是n组卷积核)。过滤器涉及到输出的通道数，即k_n=o_c。最终，卷积的数据结构为：

输入矩阵：[i_n, i_c, i_w, i_h]

卷积核矩阵：[k_n, k_c, k_w, k_h]

输出矩阵：[o_n, o_c, o_w, o_h] (o_n=i_n, o_c=k_n, o_w=i_w-k_w+1,o_h=i_h-k_h+1)

![数据结构](https://user-images.githubusercontent.com/83866527/233762073-0d8e5719-b568-4f3b-a75a-48d1c625bc5f.png)

(图片取自：https://blog.csdn.net/weixin_44064434/article/details/123647168)

为此我自定义了 **Tensor结构体** 来保存这些参数：

```c
typedef struct {
    float *data; // 数据指针
    struct {
        size_t n;
        size_t channel;
        size_t width;
        size_t height;
    } shape;
} Tensor;
```

#### 算法设计&实现

根据上面的数据结构，用户可以自定义输入的形状和大小，以及通过卷积核来控制输出的大小。面对复杂问题，我的思路是先实现 **简单且通用** 的二维卷积，然后再通过调用二维卷积算法，实现复杂卷积计算。

#### 二维卷积

二维卷积的设计逻辑是：二层循环读取输入矩阵,每次跳转幅度为strides（此时不考虑padding），然后以读取到的当前点为顶点扩展成与卷积核相同大小的子矩阵，与卷积核进行运算。运算同样是按卷积核大小二层循环读取。因此算法的复杂度为：O((i_w - k_w)/strides\*(i_h - k_h)/strides\*k_w\*k_h)

实现(注：由于输出需要合并通道结果，所以此处`*output+=tmp`。同时我有将不能修改的参数设置成const，但此处展示时删去不影响阅读)：

```c
int conv2D(float *image, int i_w, int i_h, float *kernel, int k_w, int k_h, float *output, int strides) {
    float tmp = 0.0;
    for (int i = 0, max_i = i_w - k_w; i <= max_i; i += strides) {
        for (int j = 0, max_j = i_h - k_h; j <= max_j; j += strides) {
            tmp = 0.0;
            for(int r = 0; r < k_w; r++) {
                for (int c = 0; c < k_h; c++) {
                    tmp += (*(image+(r+i)*i_h+c+j)) * (*(kernel+r*k_h+c));
                }
            }
            *output += tmp; // 输出要合并通道结果
            output++;
        }
    }
    return 1;
}
```

#### 复杂卷积

实现了简单且通用的二维卷积，复杂卷积就只需要在外层套三个循环，一层是o_n循环计算每一个输出，二层是o_c循环计算每个输出的每个通道，三层循环是k_c循环计算并求和不同输入通道的结果。同时由于前面的二维卷积没有考虑padding，是因为在我的设计中，需要进行预处理将input_data融合padding扩展成新的输入矩阵将扩展边缘设成0，形状：((i_w+2padding)*(i_h+2padding))。综合算法复杂度为：
$$
O(o_n*(o_c*k_c*conv2D+i_c*(i_w+2padding)*(i_h+2padding)))\\\\=O(i_n*(k_n*k_c*k_w*k_h*(i_w-k_w)*(i_h-k_h)/strides^2+cost\ of\ new\ martrix))\\\\
=O(i_n*(K*(i_w-k_w)*(i_h-k_h)/strides^2+i_c*(i_w+2padding)*(i_h+2padding)))
$$
实现：

```c
void convolution(Tensor *inputs, Tensor *kernel, Tensor *outputs, int padding, int strides) {
    "参数设置(如kernel_pointer)"
    ...
    "根据padding生成新输入数组，更新i_w和i_h的值"
    i_w += padding*2; i_h += padding*2;
    float (*input_data)[i_w][i_h];
    for (size_t n = 0; n < o_n; n++) {
        "重新设计单次输入的矩阵[i_c][i_w][i_h]"
        for("i_c"){for("i_w"){for("i_h"){...}}}
        kernel_pointer = kernel->data;
        for (size_t c = 0; c < o_c; c++) {
            inputs_pointer = &input_data[0][0][0];
            for(int i = 0; i < k_c; i++) {
                conv2D(inputs_pointer, i_w, i_h, 
                       kernel_pointer, k_w, k_h, 
                       outputs_pointer, strides);
                inputs_pointer += inputs_martrix_size;
                kernel_pointer += kernel_martrix_size;
            }
            outputs_pointer += outputs_martrix_size;
        }
    }
    free(input_data); // 如果申请了，需要释放内存
}
```

#### 其它辅助方法

为测试卷积准确性以及性能，我设计了文件输入输出函数，以及随机数生成函数：

```c
int read_file(const char *path, float *array);
int write_file(const char *path, float *array, const size_t o_n, const size_t o_c, const size_t o_w, const size_t o_h);
void generatingRandomNumber(const size_t size);
```

## IV. 性能分析与优化

#### 准确性测试

不同形状，不同大小测试，结果均正确。此处展示一例：

输入：padding=1,strides=1,

inputs[1,1,3,3]={1,2,3,4,5,6,7,8,9}, 

kernel[1,1,3,3]={1,2,1,0,0,0,-1,-2,-1}

输出：outputs[1,1,3,3]={-13,-20,-17,-18,-24,-18,13,20,17}

![image-20230421000703837](https://user-images.githubusercontent.com/83866527/233762101-ecafe7e4-f06b-4a8f-b80f-4b81b8997395.png)

#### 效率分析

测试使用的是<time.h>库，调用clock()函数计算时间差。由于本机性能一般，且clock函数计算的是CPU周期数(与实际时间有偏差)，所以测试结果可能偏大。以下是测试结果：

| Inputs形状         | Kernel形状   | padding | strides | 耗时(10次取平均) |
| ------------------ | ------------ | ------- | ------- | ---------------- |
| [3, 3, 1000, 1000] | [1, 1, 1, 1] | 0       | 1       | 49.539ms         |
| [3, 3, 1000, 1000] | [1, 1, 3, 3] | 0       | 1       | 118.9718ms       |
| [3, 3, 1000, 1000] | [1, 1, 5, 5] | 0       | 1       | 252.9686ms       |
| [3, 3, 1000, 1000] | [2, 3, 5, 5] | 2       | 2       | 365.956ms        |

可以看出，随着卷积核增大，卷积耗时非线性增加。大卷积包含特征更多，小卷积运算速度更快，因此设置合理的卷积很重要。

#### 优化方案

##### 程序预热

对于低延迟应用程序，预热系统中的关键路径有利于提高运行效率。设置单元测试，在程序启动时运行它们来预热代码。即使代码已经预热，也必须确保您的 CPU 缓存也保持温暖。在阻塞操作后，可以看到性能显着下降，例如网络 IO。所以，此处我将提前进行卷积操作进行预热，然后再进行正式运算。以下是测试结果：

| Inputs形状         | Kernel形状   | padding | strides | 原时间     | 预热后时间 |
| ------------------ | ------------ | ------- | ------- | ---------- | ---------- |
| [3, 3, 1000, 1000] | [1, 1, 1, 1] | 0       | 1       | 49.539ms   | 41.7835ms  |
| [3, 3, 1000, 1000] | [1, 1, 3, 3] | 0       | 1       | 118.9718ms | 106.6388ms |
| [3, 3, 1000, 1000] | [1, 1, 5, 5] | 0       | 1       | 252.9686ms | 244.6746ms |
| [3, 3, 1000, 1000] | [2, 3, 5, 5] | 2       | 2       | 365.956ms  | 355.3545ms |

可以看到，效率上确实有所提升，速度提升了8~12ms，符合预测。

##### 逻辑优化

在算法实现中，我为了调用简单二维卷积，将输入的数据与padding结合生成新的数据。但是这本身也是极大 的开销(时间和空间上都是)。若是去除这部分开销，算法运行效率应当会有提升。以下是测试结果：

| Inputs形状         | Kernel形状   | padding | strides | 预热后时间 | 预热+优化 |
| ------------------ | ------------ | ------- | ------- | ---------- | --------- |
| [3, 3, 1000, 1000] | [1, 1, 1, 1] | 0       | 1       | 41.7835ms  | 15.0388ms |
| [3, 3, 1000, 1000] | [1, 1, 3, 3] | 0       | 1       | 106.6388ms | 79.9766ms |
| [3, 3, 1000, 1000] | [1, 1, 5, 5] | 0       | 1       | 244.6746ms | 216.331ms |
| [3, 3, 1000, 1000] | [2, 3, 5, 5] | 0       | 2       | 355.3545ms | 325.153ms |

可以看到，算法在预热优化的基础上进一步提升，可见构造新数据带来的开销挺明显的，虽然使得代码简单易懂，但随着inputs层数以及大小增加，开销会越来越大。代码复杂度与时间/空间复杂度之间的权衡需要程序员自己把控。

##### OpenMP&SIMD

OpenMP在多线程库中的一个突出特点就是很容易使用`#pragma omp parallel for`等将单线程改造成多线程，但为了追求更高的效率，可能也需要在逐线程细化分配任务与循环并行化两种风格之间灵活切换。此处用在卷积运算中将颇为实用。在卷积运算中可以执行并行的地方有很多，比如并行多个输入、并行不同通道，并行矩阵运算。但CPU资源是有限的，并行并不是越多越好。此处我增加过滤器个数来增加输出的通道并进行omp，以下是测试结果：

| Inputs形状         | Kernel形状   | padding | strides | 无omp       | omp        |
| ------------------ | ------------ | ------- | ------- | ----------- | ---------- |
| [3, 3, 1000, 1000] | [2, 3, 5, 5] | 0       | 2       | 325.153ms   | 323.3525ms |
| [3, 3, 1000, 1000] | [4, 3, 5, 5] | 0       | 2       | 661.6925ms  | 651.145ms  |
| [3, 3, 1000, 1000] | [6, 3, 5, 5] | 0       | 2       | 981.1865ms  | 975.4203ms |
| [3, 3, 1000, 1000] | [8, 3, 5, 5] | 0       | 2       | 1,305.081ms | 1294.815ms |

可以看到，omp对算法优化有限，原因是代码设计不够好，不同线程频繁抢占同一资源导致互相阻塞，结果劣化。所以使用omp时需要考虑代码的合理性，是否适合并行。同样的另一种优化方式就是SIMD(AVX2/NEON)。这是汇编层面上的优化，同时取出多个数据进行运算，达到常数级优化。但它同样需要设计代码设置修改代码，使其符合同时提取同时运算的要求，这进一步提高代码的复杂度。

##### O3

O1，O2，O3是指优化标识，gcc编译器在没有优化标识时会产生可调试代码，每条指令之间将是独立的(可设断点查看变量)。当有优化标识时，gcc会在保证程序与源程序语义等价的前提之下尝试改变程序结构，以满足某些目标。

O0： 不做任何优化(默认选项)。

O1：对程序做部分编译优化，编译器会尝试减小生成代码的尺寸，以及缩短执行时间。

O2：尝试更多的寄存器级的优化以及指令级的优化。

O3：在O2的基础上进行更多的优化，例如使用伪寄存器网络，普通函数的内联，以及针对循环的更多优化。

在本次优化测试中，我对我的代码进行O3优化。以下是测试结果：

| Inputs形状         | Kernel形状   | padding | strides | 无O3      | O3        |
| ------------------ | ------------ | ------- | ------- | --------- | --------- |
| [3, 3, 1000, 1000] | [1, 1, 1, 1] | 0       | 1       | 15.0388ms | 6.5886ms  |
| [3, 3, 1000, 1000] | [1, 1, 3, 3] | 0       | 1       | 79.9766ms | 20.839ms  |
| [3, 3, 1000, 1000] | [1, 1, 5, 5] | 0       | 1       | 216.331ms | 40.907ms  |
| [3, 3, 1000, 1000] | [2, 3, 5, 5] | 0       | 2       | 325.153ms | 61.9095ms |

可以看到，执行时间大幅度降低，效率显著提高。另一方面，优化效果好也说明代码可改进空间大，说明我的代码逻辑过于直接，缺少足够的优化。

##### 优化总结

以下是优化前和优化后的执行时间对比：

| Inputs形状         | Kernel形状   | padding | strides | 无任何优化 | 全优化    | 增幅倍率 |
| ------------------ | ------------ | ------- | ------- | ---------- | --------- | -------- |
| [3, 3, 1000, 1000] | [1, 1, 1, 1] | 0       | 1       | 49.539ms   | 6.5886ms  | 7.52     |
| [3, 3, 1000, 1000] | [1, 1, 3, 3] | 0       | 1       | 118.9718ms | 20.839ms  | 5.71     |
| [3, 3, 1000, 1000] | [1, 1, 5, 5] | 0       | 1       | 252.9686ms | 40.907ms  | 6.18     |
| [3, 3, 1000, 1000] | [2, 3, 5, 5] | 2->0    | 2       | 365.956ms  | 61.9095ms | 5.91     |

## V. 总结&收获的经验

本次项目，我了解了卷积的原理和实现，并充分思考了代码的优化方案，从中收获了很多。对于代码优化，可以进行逻辑上的优化(比如修改循环顺序使得CPU的MMU hit率上升)、资源利用上的优化(如并行)或者硬件上的优化(如程序预热)。但所有的优化都是基于**对代码以及对计算机的理解**，盲目进行优化反而会导致效率降低以及答案错误。同时这也告诉我一个代码思路，就是在设计时就尽可能往优化方向靠拢，设计更能被优化的程序。编程学习博大精深，知识互相关联又互不相同，共同组成计算机科学大厦。今后我会更加勤勉学习。

## VI. 源码
https://github.com/15775011722/CS205-Sustech-Cpp/blob/main/project/project3/CS205%20CC%2B%2B%20Program%20Design%20Project3.md
