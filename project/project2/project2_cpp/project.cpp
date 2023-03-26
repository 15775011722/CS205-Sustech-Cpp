#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <limits.h>
#include <random>
#include <ctime>
using namespace std;


union Data
{
    /* short运算时强转为int，所以干脆int存 */
    // short shortData;
    int intData;
    long long longData;
    float floatData;
    double doubleData;
};

void generatingRandomNumber(Data * vector, byte * vType, int size, int limit);
void printVector(Data * vector, byte * vType, int size, int limitSize, string name);
double dotProduct(Data * vector1, byte * v1Type, Data * vector2, byte *v2Type, int size);
int main() {
    Data * vector1 = NULL;
    Data * vector2 = NULL;
    byte * v1Type = NULL;
    byte *v2Type = NULL;
    int n, limit;
    double result = 0;
    string name1 = "vector1";
    string name2 = "vector2";
    cout << "Please input the size of vector: ";
    cin>>n;
    vector1 = (Data*)malloc(n * sizeof(Data));
    vector2 = (Data*)malloc(n * sizeof(Data));
    v1Type = (byte*)malloc(n * sizeof(byte));
    v2Type = (byte*)malloc(n * sizeof(byte));
    cout << "Do you want to limit the element type of vector?" << endl;
    cout << "Short(0), Integer(1), Long(2), Float(3), Double(4), No Limit(Any other number)\n: ";
    cin>>limit;
    cout << "Vectors are generating..." << endl;
    auto start = std::chrono::high_resolution_clock::now();
    // 生成随机数
    generatingRandomNumber(vector1, v1Type, n, limit);
    generatingRandomNumber(vector2, v2Type, n, limit);
    auto end = std::chrono::high_resolution_clock::now();
    cout << "The cost time of generating random number is ";
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    if(ms > 1000) cout << ms << "ms" << endl;
    else cout << us << "us" << endl;
    // 打印
    printVector(vector1, v1Type, n, 6, name1);
    printVector(vector2, v2Type, n, 6, name2);
    start = std::chrono::high_resolution_clock::now();
    // 点乘
    result = dotProduct(vector1, v1Type, vector2, v2Type, n);
    end = std::chrono::high_resolution_clock::now();
    cout << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "The dot product is: " << result << endl;
    cout << "The cost time of dot product is ";
    us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    if(ms > 1000) cout << ms << "ms" << endl;
    else cout << us << "us" << endl;
    free(v1Type);
    free(v2Type);
    free(vector1);
    free(vector2);
    return 0;
}

void generatingRandomNumber(Data * vector, byte * vType, int size, int limit) {
    Data *p = vector;
    byte *pType = vType;
    int type, symbol;
    int shortRange = SHRT_MAX - SHRT_MIN + 1;
    uniform_real_distribution<float> f(-1.8e19f, 1.8e19f);
    uniform_real_distribution<double> d(-1.3e+150, 1.3e+150);
    default_random_engine e(time(NULL));
    for (size_t i = 0; i < size; i++)
    {
        /* [0,5)-> short\int\long\float\double */
        type = rand() % 5;
        symbol = (rand() % 2) ? 1 : -1;
        switch (type)
        {
        case 0:
        case 1:
            p->intData = SHRT_MIN + rand() % shortRange;
            *pType = (byte) 0x01;
            break;
        case 2:
            p->longData = symbol * rand();
            *pType = (byte) 0x02;
            break;    
        case 3:
            p->floatData = f(e);
            *pType = (byte) 0x03;
            break;
        case 4:
            p->doubleData = d(e);
            *pType = (byte) 0x04;
            break;
        default:
            i--;
            break;
        }
        p++;
        pType++;
    }
    
}

void printVector(Data * vector, byte * vType, int size, int limitSize, string name) {
    Data *p = vector;
    byte *pType = vType;
    if (limitSize == 0) limitSize = 6;
    int example = min(size, limitSize);
    cout << name << ": [";
    int idx = 0;
    for (; idx < example;idx++) {
        switch(*pType) {
            case (byte)0x01:
                cout << p->intData << ",";
                break;
            case (byte)0x02:
                cout << p->longData << ",";
                break;
            case (byte)0x03:
                cout << p->floatData << ",";
                break;
            default:
                cout << p->doubleData << ",";
                break;
        }
        p++;
        pType++;
    }
    cout << " (size " << size << ")]" << endl;
}

double dotProduct(Data * vector1, byte * v1Type, Data * vector2, byte *v2Type, int size) {
    double result = 0;
    int xType;
    for (size_t i = 0; i < size; i++) {
        xType = (((int) v1Type[i])<<2) - (int) v2Type[i];
        switch (xType)
        {
        case 0:
            result += vector1[i].intData * vector2[i].doubleData;
            break;
        case 1:
            result += vector1[i].intData * vector2[i].floatData;
            break;
        case 2:
            result += vector1[i].intData * vector2[i].longData;
            break;
        case 3:
            result += vector1[i].intData * vector2[i].intData;
            break;
        case 4:
            result += vector1[i].longData * vector2[i].doubleData;
            break;
        case 5:
            result += vector1[i].longData * vector2[i].floatData;
            break;
        case 6:
            result += vector1[i].longData * vector2[i].longData;
            break;
        case 7:
            result += vector1[i].longData * vector2[i].intData;
            break;
        case 8:
            result += vector1[i].floatData * vector2[i].doubleData;
            break;
        case 9:
            result += vector1[i].floatData * vector2[i].floatData;
            break;
        case 10:
            result += vector1[i].floatData * vector2[i].longData;
            break;
        case 11:
            result += vector1[i].floatData * vector2[i].intData;
            break;
        case 12:
            result += vector1[i].doubleData * vector2[i].doubleData;
            break;
        case 13:
            result += vector1[i].doubleData * vector2[i].floatData;
            break;
        case 14:
            result += vector1[i].doubleData * vector2[i].longData;
            break;
        case 15:
            result += vector1[i].doubleData * vector2[i].intData;
            break;
        default:
            cout << "未知错误，出现在点乘时数组类型指针，超出[1,4]范围。" << endl;
            break;
        }
    }
    return result;
}
