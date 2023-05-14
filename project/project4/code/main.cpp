#include "Mat.hpp"
#include <iostream>
using namespace std;

int main()
{
    cout << "构造器演示" << endl;
    Mat<short> t1;
    Mat<float> t2(3, 3, 3);
    float *data1 = new float[t2.getSize()]();
    float *data2 = new float[t2.getSize()]();
    for (int i = 0;i < t2.getSize(); i++)
    {
        data1[i] = 0.1+i;
        data2[i] = i;
    }
    t2.setData(data1, 27);
    Mat<float> t3(3, 3, 3);
    t3.setData(data2, 27);
    // Mat<float> t3(3, 3, 3, data2); // 有bug勿用！
    Mat<float> t4(t3);
    Mat<double> t5(t4);
    

    // 操作符演示
    cout << "操作符演示" << endl;
    t2 = t2 - t3;
    t2.display();
    Mat<float> c = t2 + t3;
    c.display();
    c = t2 * t3;
    c.display();
    if (t2 == t3)
    {
        cout << "t2 is same as t3" << endl;
    }
    else
    {
        cout << "t2 is different from t3" << endl;
    }
    Mat<float> *z = new Mat<float>(3, 3, 3);
    z->display();
    delete z;
}