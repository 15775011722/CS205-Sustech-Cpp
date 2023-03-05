#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*
所有可能结果(同时也是优先级顺序)：
-1. 不知道出什么bug -> 输出：Unknown error!
0. 输入的字符串不足3(num1，operate，num2) -> 输出：Error input!
1. 输入的不是数字 -> 输出：The input cannot be interpret as numbers!
2. 输入的操作符不是+ - x / -> 输出：The operator cannot be recognized as + - x or /!
3. 除数是0 -> 输出：A number cannot be divied by zero.
4. 正常加减乘除 -> 输出：数字1 操作符 数字2 = 结果
*/

long double add(long double n1, long double n2);
long double subtract(long double n1, long double n2);
long double multiply(long double n1, long double n2);
long double divide(long double n1, long double n2);
bool isOperate(char op);
bool isNumber(char c);
bool isNumberStr(char arr[]);
int charIndex(char arr[], char ch);

int main(int argc, const char* argv[])
{
    const char *result = NULL;
    long double ans = 0;
    if(argc < 4)
    {
        char temp[] = "Error input!";
        result = temp;
    }
    else
    {
        if(strlen(argv[2]) > 1)
        {
            char temp[] = "Error input!";
            result = temp;
        }
        else
        {
            char *num1 = (char*)(argv[1]);
            char *num2 = (char*)(argv[3]);
            char op = argv[2][0];
            if(!isNumberStr(num1) || !isNumberStr(num2))
            {
                char temp[] = "The input cannot be interpret as numbers!";
                result = temp;
            }
            else if (!isOperate(op))
            {
                char temp[] = "The operator cannot be recognized as + - x or /!\0";
                result = temp;
            }
            else
            {
                long double arr3 = strtod(num1, NULL);
                long double arr4 = strtod(num2, NULL);
                switch (op)
                {
                case '+':
                    ans = add(arr3, arr4);
                    result = "-Correct Input!";
                    break;
                case '-':
                    ans = subtract(arr3, arr4);
                    result = "-Correct Input!";
                    break;
                case 'x':
                    ans = multiply(arr3, arr4);
                    result = "-Correct Input!";
                    break;
                case '/':
                    if(arr4==0)
                        result = "A number cannot be divied by zero.";
                    else
                    {
                        ans = divide(arr3, arr4);
                        result = "-Correct Input!";
                    }
                    break;
                default:
                    result = "Unknown error!";
                    break;
                }
            }
        }
    }
    if (result[0] != '-' && !isNumber(result[0]))
    {
        int l = strlen(result);
        int i;
        for (i = 0; i < l; i++)
        {
            printf("%c%c", result[i], '\0');
        }
        puts("");
    }
    else
    {
       printf("%s %s %s = %Lg\n", argv[1], argv[2], argv[3], ans); 
    }
    return 0;
}

int charIndex(char arr[], char ch)
{
    int idx = -1, len = strlen(arr);
    int i;
    for(i = 0; i < len; i++){
        if (arr[i] == ch)
        {
            idx = i;
            break;
        }
    }
    return idx;
}

bool isNumberStr(char arr[])
{
    int i;
    int l1 = strlen(arr);
    int dot1 = -1;
    bool isScientific1 = false;
    if (arr[0] != '-' && !isNumber(arr[0]))
        return false;
    for (i = 1; i < l1; i++)
    {
        if (!isNumber(arr[i]))
        {
            if(arr[i] == 'E')
                arr[i] = 'e';
            if(arr[i] == '.')
            {
                if(dot1 == -1)
                    dot1 == i;
                else
                    return false;
            }
            else if(arr[i] == 'e')
            {
                if(!isScientific1)
                    isScientific1 = true;
                else
                    return false;
            }
            else
            {
                if(arr[i] != '-' || arr[i-1] != 'e')
                {
                    return false;
                }  
            }
        } 
    }
    return true;
}

bool isNumber(char c)
{
    return c >= '0' && c <= '9';
}

bool isOperate(char op)
{
    return (op == '+') || (op == '-') || (op == 'x') || (op == '/');
}

long double add(long double n1, long double n2)
{
    return n1 + n2;
}
long double subtract(long double n1, long double n2)
{
    return n1 - n2;
}
long double multiply(long double n1, long double n2)
{
    return n1 * n2;
}
long double divide(long double n1, long double n2)
{
    return n1 / n2;
}
