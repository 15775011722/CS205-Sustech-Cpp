#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

char *result = NULL;
char *arr1 = NULL;
char *arr2 = NULL;
char *newNum = NULL;


/* 扩展到一样长，然后对应位相加 */
void add();
void align(char arr[], int bias);
void classifier(char num1[], char num2[], char op);
bool isOperate(char op);
bool isNumber(char c);
bool isNumberStr(char arr[]);
void fixedPoint2scientific(char arr[], int cao);
int charIndex(char arr[], char ch);
int main(int argc, const char* argv[])
{
    
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
            char *num1 = const_cast<char*>(argv[1]);
            char *num2 = const_cast<char*>(argv[3]);
            char op = argv[2][0];
            classifier(num1, num2, op);
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
       printf("%s %s %s = %s\n", argv[1], argv[2], argv[3], result); 
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
    return (op == '+') || (op == '-') || (op == '*') || (op == '/');
}

void classifier(char num1[], char num2[], char op)
{
    if(!isNumberStr(num1) || !isNumberStr(num2))
    {
        char temp[] = "The input cannot be interpret as numbers!";
        result = temp;
    }
    else if (!isOperate(op))
    {
        char temp[] = "The operator cannot be recognized as + - * or /!\0";
        result = temp;
    }
    else
    {

        if(charIndex(num1, 'e') != -1)
        {
            arr1 = num1;
        }    
        else
        {
            fixedPoint2scientific(arr1, 1);
        }
        if(charIndex(num2, 'e') != -1)
        {
            arr2 = num2;
        }    
        else
        {
            // // 不知道为啥得这么写，还不能printf
            // char a[strlen(arr1)+1] = {'\0'};
            // for(int i = 0, j = strlen(arr1); i < j; i++)
            //     a[i] = arr1[i];
            // // printf("%s\n", a);
            fixedPoint2scientific(arr2, 2);
        }
        switch (op)
        {
        case '+':
            add();
            break;
        default:
            char temp[] = "Unknown error!";
            result = temp;
            break;
        }
    }
}

void align(char arr[], int bias)
{
    int i, j, k, tmp, lenmax;
    int e = charIndex(arr, 'e');
    int dot = charIndex(arr, '.');
    bool isFractional = (arr[e+1] == '-');
    bool isNegative = (arr[0] == '-');
    int E = 0, len = strlen(arr);
    k = isFractional ? e+1 : e;
    for(i = len-1, j = 0; i > k; i--, j++)
        E += (arr[i]-'0') * pow(10, j);
    // 如-3 -> -5; 5 -> 3
    E += isFractional ? bias : -bias;
    // 由于总是变大的，所以小数点右移bias位
    // 先计算是否长度变长，是否从小数变成整数(要删去‘.’)
    // 只有当小数点后的数少于bias时长度会增加(bias-(e-dot-1)) 默认dot存在
    // 因此e前面的长度是 e(<) or e-1(=) or bias+dot(>)  默认dot存在
    lenmax = 0;
    if(bias < (e-dot-1))
        lenmax = e;
    else if(bias == (e-dot-1))
        lenmax = e-1;
    else
        lenmax = bias+dot;
    // 需要注意的是，当dot不存在时长度直接为e+bias
    if(dot == -1)
        lenmax = e + bias;
    tmp = lenmax;
    // 然后加上‘e’和可能有的-号和后面的指数就完成了对齐
    lenmax++;
    lenmax += isFractional ? 1 : ((E < 0) ? 1 : 0);
    if(E == 0)
        lenmax++;
    j = (E < 0) ? -E : E;
    while (j > 0)
    {
        j = j / 10;
        lenmax++;
    }
    char newArr[lenmax+1];
    k = -1;
    if(bias < (e-dot-1))
    {
        k = dot+bias;
        newArr[k] = '.';
    }
    for(i = 0, j = 0; i < e; i++, j++)
    {
        if(arr[i] == '.')
        {
            j--;
            continue;
        }

        if(j == k)
        {
            i--;
            continue;
        }
        newArr[j] = arr[i];
    }
    for(; j < tmp; j++)
        newArr[j] = '0';
    newArr[j++] = 'e';
    if(isFractional)
        newArr[j] = '-';
    else if(E < 0)
    {
        newArr[j] = '-';
        E = -E;
    }
    j = lenmax - 1;
    if(E == 0)
        newArr[j] = '0';
    while (E > 0)
    {
        newArr[j--] = (char)(E % 10)+'0';
        E = E / 10;
    }
    newArr[lenmax] = '\0';
    newNum = newArr;
}

void add()
{
    char *num1 = arr1;
    char *num2 = arr2;
    // printf("%s\n%s\n", num1, num2);
    int bigOne, i, j, k, bias;
    double E1, E2, res;
    bool num1IsBig = true;
    int len1 = strlen(num1), len2 = strlen(num2);
    int e1 = charIndex(num1, 'e');
    int e2 = charIndex(num2, 'e');
    char arr1[len1-e1-1], arr2[len2-e1-1];
    char arr3[e1], arr4[e2];
    for(i = e1+1, j = 0; i < len1; i++, j++)
        arr1[j] = num1[i];
    for(i = e2+1, j = 0; i < len2; i++, j++)
        arr2[j] = num2[i];
    E1 = strtod(arr1, NULL);
    E2 = strtod(arr2, NULL);
    num1IsBig = (E1 > E2);
    if(num1IsBig)
    {
        align(num1, (int)(E1-E2));
        char *tempNum = const_cast<char*>(newNum);
        e1 = charIndex(tempNum, 'e');
        for(i = 0, j = 0; i < e1; i++, j++)
            arr3[j] = tempNum[i];
        for(i = 0, j = 0; i < e2; i++, j++)
            arr4[j] = num2[i];
        res = strtod(arr3, NULL)+strtod(arr4, NULL);
    }
    else
    {
        align(num2, (int)(E2-E1));
        char *tempNum = const_cast<char*>(newNum);
        e2 = charIndex(tempNum, 'e');
        for(i = 0, j = 0; i < e1; i++, j++)
            arr3[j] = num1[i];
        for(i = 0, j = 0; i < e2; i++, j++)
            arr4[j] = tempNum[i];
        res = strtod(arr3, NULL)+strtod(arr4, NULL);
    }
    // printf("%lf\n", result);
    int E = 0;
    bool isNegative = (res < 0);
    res = fabs(res);
    if(res < 1)
    {
        while (res < 1)
        {
            res *= 10;
            E--;
        }
    }
    else
    {
        while (res > 10)
        {
            res /= 10;
            E++;
        }
    }
    E += num1IsBig ? E2 : E1;
    char tempArr[328];
    if(isNegative)
        tempArr[0] = '-';
    i = isNegative ? 1 : 0;
    tempArr[i++] = (int)res + '0';
    res -= (int)res;
    if(res > 0)
        tempArr[i++] = '.';
    while (res > 0)
    {
        res *= 10;
        tempArr[i++] = (int)res + '0';
        res -= (int)res;
    }
    tempArr[i++] = 'e';
    isNegative = (E < 0);
    if(isNegative)
        tempArr[i++] = '-';
    E = abs(E);
    j = E, k = 0;
    while (j > 0)
    {
        j /= 10;
        k++;
    }
    for(j = k+i-1;j >= i;j--)
    {
        tempArr[j] = (char)(E % 10)+'0';
        E = E / 10;
    }
    tempArr[k+i] = '\0';
    result = tempArr;
}

char* subtract(char num1[], char num2[])
{return NULL;}
char* multiply(char num1[], char num2[])
{return NULL;}
char* divide(char num1[], char num2[])
{return NULL;}

void fixedPoint2scientific(char arr[], int cao)
{
    int E, newLen, oldLen = strlen(arr);
    int dot = charIndex(arr, '.');
    bool isNegative = (arr[0] == '-');
    int tempIndex = isNegative ? 1 : 0;
    bool isFractional = (arr[tempIndex] == '0') && (arr[tempIndex+1] == '.');
    bool canBeInteger = true;
    // 再遍历到头之前有一个是1~9就说明有'.'
    if(isFractional)
    {
        for(int i = 0, j = oldLen-1;i < j; i++)
        {
            if(arr[i] > '0' && arr[i] <= '9')
            {
                canBeInteger = false;
                break;
            }
        }
    }
    else
    {
        for(int i = oldLen-1;i > tempIndex; i--)
        {
            if(arr[i] > '0' && arr[i] <= '9')
            {
                canBeInteger = false;
                break;
            }
        }
    }
    dot = (dot == -1) ? oldLen : dot;
    if(isFractional)
    {
        for(int i = dot+1; i < oldLen; i++)
        {
            if(arr[i] != '0')
            {
                E = i - dot;
                // oldLen-i+1是E前面数字的长度,包含‘.’
                // 循环是为了计算E后面数字的长度
                // 同时为‘E’预留一个位置，所以+1
                // 考虑到E后面还有一个‘-’，所以再+1
                newLen = oldLen - i + 3;
                // 但是要是化为科学计数法后小数点没了，那么
                newLen -= canBeInteger ? 1 : 0;
                int j = E;
                while (j > 0)
                {
                    j = j / 10;
                    newLen++;
                }
                // 如果是负数，还得加一个符号
                j = isNegative ? 1 : 0;
                newLen += j;
                // +1留一位给\0
                char newArr[newLen+1];
                newArr[0] = isNegative ? '-' : arr[i];
                newArr[j++] = arr[i];
                if(!canBeInteger)
                {
                    newArr[j++] = '.';
                    for(i += 1; i < oldLen; i++, j++)
                        newArr[j] = arr[i];
                }
                newArr[j++] = 'e';
                newArr[j] = '-';
                j = newLen - 1;
                while (E > 0)
                {
                    newArr[j--] = (char)(E % 10)+'0';
                    E = E / 10;
                }
                newArr[newLen] = '\0';
                if(cao == 1)
                    arr1 = newArr;
                else
                    arr2 = newArr;
            }
        }
        if(cao == 1)
            arr1 = arr;
        else
            arr2 = arr;
    }
    else
    {
        int i, j, k;
        E = isNegative ? dot - 2 : dot - 1;
        // 没得改的情况也得强行改
        if(E == 0)
        {
            // +2为的是结尾的e0，+1为的是\0
            char newArr[oldLen+2+1];
            for(i = 0; i < oldLen; i++)
                newArr[i] = arr[i];
            newArr[oldLen] = 'e';
            newArr[oldLen+1] = '0';
            newArr[oldLen+2] = '\0';
            if(cao == 1)
                arr1 = newArr;
            else
                arr2 = newArr;
        }
        // 初步的长度是原本的长度+一个‘e’
        // 考虑到输入整数没有'.'则需要再补充+1
        newLen = (dot == oldLen) ? oldLen + 2 : oldLen + 1;
        // 但又考虑到要是变成科学计数法也没有‘.’,需要再减回来-1
        newLen -= canBeInteger ? 1 : 0;
        // 再加上E后面指数部分的长度
        j = E;
        while (j > 0)
        {
            j = j / 10;
            newLen++;
        }
        // 如果末尾是0还需要删掉,事实上只有当dot=oldlen(没有小数)的情况下才可能出现
        // 提前记录后面迭代的次数
        tempIndex = oldLen;
        if(dot == oldLen)
        {
            for(i = oldLen-1; i > 0; i--)
            {
                if (arr[i] != '0')
                    break;
                newLen--;
                tempIndex--;
            }
        }
        // +1给\0
        char newArr[newLen+1];
        newArr[0] = arr[0];
        newArr[1] = arr[1];
        j = isNegative ? 2 : 1;
        if(!canBeInteger)
        {
            newArr[j++] = '.';
            for(int k = j - 1; k < tempIndex; j++, k++)
            {
                if(arr[k] == '.')
                {
                    j--;
                    continue;
                }
                newArr[j] = arr[k];
            }
        }
        newArr[j] = 'e';
        j = newLen - 1;
        while (E > 0)
        {
            newArr[j--] = (char)(E % 10)+'0';
            E = E / 10;
        }
        newArr[newLen] = '\0';
        if(cao == 1)
            arr1 = newArr;
        else
            arr2 = newArr;
    }
}

