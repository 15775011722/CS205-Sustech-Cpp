#include <iostream>
using namespace std;

//采用declval,表达式 declval<>可以在不需要使用默认构造函数
//（或者其它任意操作）的情况下为类型 T生成一个值
// 以下操作就是将T1与T2相加，得到的数据类型赋给Type
// 就是说，float+int == float
template<typename T1,typename T2>
struct ResultT
{
	using Type = decltype(std::declval<T1>() + std::declval<T2>());
};
 
//添加别名模板
template<typename T1, typename T2>
using Result = typename ResultT<T1, T2>::Type;

// // 例子
// template <typename T>
// class A{
// private:
    
// public:
//     T a;
//     A();
//     A(T a);
//     template<typename T1, typename T2>
//     friend A<Result<T1, T2>>& operator+ (A<T1>& a, A<T2>& b);
//     A& operator=(const A& b) 
//     {
//         this->a = b.a;
//         return *this;
//     }
    
// };

// template<typename T>
// A<T>::A() : a(1) {}

// template<typename T>
// A<T>::A(T a) : a(a) {}

// template<typename T1, typename T2>
// A<Result<T1, T2>>& operator+ (A<T1>& a, A<T2>& b)
// {
//     cout << "abcd" << endl;
//     A<Result<T1, T2>> *tmp = new A(a.a + b.a);
//     return *tmp;
// }

// int main()
// {
//     A<short> a(1);
//     A<float> b(11);
//     A<float> d = b + a; //  重要！！无参构造器实例赋值可能会报错！也就是a()不行但a可以！
//     A<int> e;
//     d = b+e;
//     cout << d.a << endl;
// }
