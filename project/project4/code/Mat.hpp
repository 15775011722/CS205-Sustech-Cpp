#pragma once
#include "ErrorMsg.hpp"
#include "TypeSel.hpp"
#include <memory>
#include <string.h>
#include <iostream>
using namespace std;


template <typename _Tp> 
class Mat
{
private:
    shared_ptr<_Tp> data;  // data的保存格式为(channel, row, col),也就是说相当于
                           // 是拥有channel个元素的数组，每个元素是row*col的矩阵
                           // 如此设计是针对题目中的CNN网络。与Opencv中的Mat保存形
                           // 式不同，channel分离是为了方便运算，deep learning大多
                           // 如此保存，但Opencv的保存格式是每个元素的所有通道值放一
                           // 起，因为通道是作为元素属性而不是维度。尽管更符合定义但不
                           // 适合训练，还是分开通道好运算。
    long long rows;
    long long cols;
    long long step;   // step是步长，step[i]代表矩阵维度-(i+1)后的大小，
                      // 由于此处仅是2维矩阵，故直接用step代替step[0]作为一行大小
                      // 同时step被用来计算元素在矩阵中位置，所以这里就不用原版的byte了，
                      // 改为对应元素类型的个数，方便计算偏差
                      // step[1]在此处指元素类型大小，直接用elementSize代替
    int elementSize;
    long long channels;
    long long size;

public:
	Mat();
    Mat(const long long rows, const long long cols, const long long channels);
    Mat(const long long rows, const long long cols, const long long channels, _Tp *data);
    Mat(const Mat& m);
    // 强转设计构造器
    template<typename T2>
    Mat(const Mat<T2>& m) {
        this->rows = m.getRows();
        this->cols = m.getCols();
        this->step = m.getStep();
        this->size = m.getSize();
        this->channels = m.getChannels();
        this->elementSize = sizeof(_Tp);
        this->data = shared_ptr<_Tp>(new _Tp[this->size]);
        T2 *tmp = m.getData().get();
        _Tp *tmp2 = this->data.get();
        for (long long i = 0; i < m.getSize(); i++)
        {
            tmp2[i] = (_Tp)(tmp[i]);
        }
    }
    ~Mat()
    {
        // cout << "释放内存~" << endl;
    }

    Mat& operator = (const Mat& m);
    template<typename T1, typename T2>
    friend Mat<Result<T1, T2>>& operator + (const Mat<T1>& m1, const Mat<T2>& m2);
    template<typename T1, typename T2>
    friend Mat<Result<T1, T2>>& operator - (const Mat<T1>& m1, const Mat<T2>& m2);
    template<typename T1, typename T2>
    friend Mat<Result<T1, T2>>& operator * (const Mat<T1>& m1, const Mat<T2>& m2);
    template<typename T1, typename T2>
    friend bool operator == (const Mat<T1>& m1, const Mat<T2>& m2);
    template<typename T1, typename T2>
    friend bool operator != (const Mat<T1>& m1, const Mat<T2>& m2);
 
    long long getRows() const;
    long long getCols() const;
    long long getChannels() const;
    long long getStep() const;
    long long getSize() const;
    int getElementSize() const;
    shared_ptr<_Tp> getData() const;
    void setData(_Tp *data, long long length);
    void display();
};

template<typename _Tp>
Mat<_Tp>::Mat() 
: rows(0), cols(0), step(0), channels(0), size(0), 
elementSize(sizeof(_Tp)), data(shared_ptr<_Tp>(new _Tp[0]))
{}

template<typename _Tp>
Mat<_Tp>::Mat(const long long rows, const long long cols, const long long channels)
{
    this->rows = rows;
    this->cols = cols;
    this->step = rows;
    this->channels = channels;
    this->size = rows * cols * channels;
    this->elementSize = sizeof(_Tp);
    this->data = shared_ptr<_Tp>(new _Tp[rows * cols * channels]());
    memset(this->data.get(), 0, rows * cols * channels);
}

template<typename _Tp>
Mat<_Tp>::Mat(const long long rows, const long long cols, const long long channels, _Tp *data)
{
    this->rows = rows;
    this->cols = cols;
    this->step = rows;
    this->channels = channels;
    this->size = rows * cols * channels;
    this->elementSize = sizeof(_Tp);
    this->data = shared_ptr<_Tp>(data);
}

template<typename _Tp>
Mat<_Tp>::Mat(const Mat& m)
{
    this->data = shared_ptr<_Tp>(new _Tp[0]);
    *this = m;
}

template<typename _Tp>
Mat<_Tp>& Mat<_Tp>::operator = (const Mat& m)
{
    if (this->data == m.data) 
    {
        return *this;
    }
    this->rows = m.getRows();
    this->cols = m.getCols();
    this->step = m.getStep();
    this->channels = m.getChannels();
    this->size = m.getSize();
    this->elementSize = m.getElementSize();
    this->data = shared_ptr<_Tp>(m.getData());
    return *this;
}

template<typename T1, typename T2>
Mat<Result<T1, T2>>& operator + (const Mat<T1>& m1, const Mat<T2>& m2)
{
    if (m1.rows != m2.rows || m1.cols != m2.cols || m1.channels != m2.channels)
    {
        handleError(SHAPE_NOT_MATCH);
    }
    Mat<Result<T1, T2>> *matrix = new Mat<Result<T1, T2>>(m1.rows, m1.cols, m1.channels);
    // SIMD--AVX2
    // 在这里使用并不方便，因为数据类型是不确定的，同样拿256bit，
    // 可能一个Mat只能拿4个数，另一个可以拿32个数
    // 但要符合运算，另一个也只能拿4个数，且要对齐补0
    // 这样会导致代码复杂，不利于维护。
    Result<T1, T2> *sum = matrix->data.get();
    T1 *p1 = m1.data.get();
    T2 *p2 = m2.data.get();
    long long remainder = (matrix->size >> 3) << 3;
    if (remainder != 0) {
        // #pragma omp parallel for schedule(dynamic)
        for (long long i = 0; i < matrix->size; i+=8)
        {
            sum[i] = (Result<T1, T2>)(p1[i] + p2[i]);
            sum[i+1] = (Result<T1, T2>)(p1[i+1] + p2[i+1]);
            sum[i+2] = (Result<T1, T2>)(p1[i+2] + p2[i+2]);
            sum[i+3] = (Result<T1, T2>)(p1[i+3] + p2[i+3]);
            sum[i+4] = (Result<T1, T2>)(p1[i+4] + p2[i+4]);
            sum[i+5] = (Result<T1, T2>)(p1[i+5] + p2[i+5]);
            sum[i+6] = (Result<T1, T2>)(p1[i+6] + p2[i+6]);
            sum[i+7] = (Result<T1, T2>)(p1[i+7] + p2[i+7]);
        }
    }
    for (long long i = remainder; i < matrix->size; i++)
    {
        sum[i] = (Result<T1, T2>)(p1[i] + p2[i]);
    }
    return *matrix;
}

template<typename T1, typename T2>
Mat<Result<T1, T2>>& operator - (const Mat<T1>& m1, const Mat<T2>& m2)
{
    if (m1.rows != m2.rows || m1.cols != m2.cols || m1.channels != m2.channels)
    {
        handleError(SHAPE_NOT_MATCH);
    }
    Mat<Result<T1, T2>> *matrix = new Mat<Result<T1, T2>>(m1.rows, m1.cols, m1.channels);
    Result<T1, T2> *sum = matrix->data.get();
    T1 *p1 = m1.data.get();
    T2 *p2 = m2.data.get();
    long long remainder = (matrix->size >> 3) << 3;
    if (remainder != 0) {
        // #pragma omp parallel for schedule(dynamic)
        for (long long i = 0; i < matrix->size; i+=8)
        {
            sum[i] = (Result<T1, T2>)(p1[i] - p2[i]);
            sum[i+1] = (Result<T1, T2>)(p1[i+1] - p2[i+1]);
            sum[i+2] = (Result<T1, T2>)(p1[i+2] - p2[i+2]);
            sum[i+3] = (Result<T1, T2>)(p1[i+3] - p2[i+3]);
            sum[i+4] = (Result<T1, T2>)(p1[i+4] - p2[i+4]);
            sum[i+5] = (Result<T1, T2>)(p1[i+5] - p2[i+5]);
            sum[i+6] = (Result<T1, T2>)(p1[i+6] - p2[i+6]);
            sum[i+7] = (Result<T1, T2>)(p1[i+7] - p2[i+7]);
        }
    }
    for (long long i = remainder; i < matrix->size; i++)
    {
        sum[i] = (Result<T1, T2>)(p1[i] - p2[i]);
    }
    return *matrix;
}

template<typename T1, typename T2>
Mat<Result<T1, T2>>& operator * (const Mat<T1>& m1, const Mat<T2>& m2)
{
    if (m1.cols != m2.rows || m1.channels != m2.channels)
    {
        handleError(SHAPE_NOT_MATCH);
    }
    Mat<Result<T1, T2>> *matrix = new Mat<Result<T1, T2>>(m1.rows, m2.cols, m1.channels);
    Result<T1, T2> (*sum)[matrix->rows][matrix->cols] 
        = (Result<T1, T2> (*)[matrix->rows][matrix->cols])matrix->data.get();
    T1 (*p1)[m1.rows][m1.cols] = (T1 (*)[m1.rows][m1.cols])m1.data.get();
    T2 (*p2)[m2.rows][m2.cols] = (T2 (*)[m2.rows][m2.cols])m2.data.get();
    long long remainder = (matrix->cols >> 3) << 3;  // 除8再成8，相当于减去余数
    // #pragma omp parallel for schedule(dynamic)
    for (long long c = 0; c < matrix->channels; c++)
    {
        for (long long i = 0; i < m1.rows; i++)
        {
            for (long long j = 0; j < m1.cols; j++)
            {
                if (remainder != 0) {
                    for (long long k = 0; k < m2.cols; k+=8)
                    {
                        sum[c][i][k] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k]);
                        sum[c][i][k+1] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k+1]);
                        sum[c][i][k+2] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k+2]);
                        sum[c][i][k+3] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k+3]);
                        sum[c][i][k+4] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k+4]);
                        sum[c][i][k+5] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k+5]);
                        sum[c][i][k+6] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k+6]);
                        sum[c][i][k+7] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k+7]);
                    }
                }
                for (long long k = remainder; k < matrix->cols; k++)
                {
                    sum[c][i][k] += (Result<T1, T2>)(p1[c][i][j] * p2[c][j][k]);
                }
            }
        }
    }
    return *matrix;
}

template<typename T1, typename T2>
bool operator == (const Mat<T1>& m1, const Mat<T2>& m2)
{
    bool sameType = is_same<T1, T2>::value;
    bool sameShape = (m1.rows==m2.rows) && (m1.cols==m2.cols) && (m1.channels==m2.channels);
    if (!(sameType && sameShape))
        return false;
    T1 *p1 = m1.data.get();
    T2 *p2 = m2.data.get();
    for (long long i = 0; i < m1.size; i++)
    {
        if (p1[i] != p2[i])
            return false;
    }
    return true;  
}

template<typename T1, typename T2>
bool operator != (const Mat<T1>& m1, const Mat<T2>& m2)
{
    return !(m1 == m2);
}

template<typename _Tp>
void Mat<_Tp>::display() 
{
    _Tp (*data)[this->rows][this->cols] = 
                (_Tp (*)[this->rows][this->cols])this->data.get();
    cout << "[";
    for (long long c = 0; c < this->channels; c++)
    {
        cout << "[";
        for (long long i = 0; i < this->rows; i++)
        {
            for (long long j = 0; j < this->cols; j++)
            {
                cout << data[c][i][j] << " ";
            }
            cout << "\n ";
        }
        cout << "]" << endl;
    }
    cout << "]" << endl;
}

template<typename _Tp>
long long Mat<_Tp>::getRows() const
{
    return this->rows;
}

template<typename _Tp>
long long Mat<_Tp>::getCols() const
{
    return this->cols;
}

template<typename _Tp>
long long Mat<_Tp>::getChannels() const
{
    return this->channels;
}

template<typename _Tp>
long long Mat<_Tp>::getStep() const
{
    return this->step;
}

template<typename _Tp>
long long Mat<_Tp>::getSize() const
{
    return this->size;
}

template<typename _Tp>
int Mat<_Tp>::getElementSize() const
{
    return this->elementSize;
}

template<typename _Tp>
shared_ptr<_Tp> Mat<_Tp>::getData() const
{
    return this->data;
}

template<typename _Tp>
void Mat<_Tp>::setData(_Tp *data, long long length)
{
    if(size != length)
    {
        handleError(ILLEGAL_MODIFY);
    }
    this->data = shared_ptr<_Tp>(data);
}
