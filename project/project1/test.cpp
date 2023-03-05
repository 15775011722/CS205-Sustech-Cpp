#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <math.h>

const char * scientific2FixedPoint(char arr[]);
const char * fixedPoint2scientific(char arr[]);
const char * align(char arr[], int bias);
int charIndex(char arr[], char c);

char *b;

int main()
{
    char arr1[] = "-2.1234e10";
    char arr2[] = "31.39999";
    const char *arr3 = fixedPoint2scientific(arr2);
    printf("arr3: %s\n", arr3);
    int l = strlen(arr3);
    int i;
    for (i = 0; i < l; i++)
    {
        printf("%c%c", arr3[i], '\0');
    }
    puts("");
    b = const_cast<char*>(arr3);
    printf("%s\n", b);
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

const char * fixedPoint2scientific(char arr[])
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
                // strcpy(arr, newArr);
                printf("%s\n", newArr);
                const char *outputArr = newArr;
                return outputArr;
            }
        }
        const char *outputArr = arr;
        return outputArr;
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
            // strcpy(arr, newArr);
            printf("%s\n", newArr);
            const char *outputArr = newArr;
            return outputArr;
            // return;
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
        printf("%s\n", newArr);
        // arr = (char *)malloc(strlen(newArr));
        // strcpy(arr, newArr);
        const char *outputArr = newArr;
        return outputArr;
    }
}

const char * scientific2FixedPoint(char arr[])
{
    int i, j, lenmax;
    int e = charIndex(arr, 'e');
    int dot = charIndex(arr, '.');
    bool isFractional = (arr[e+1] == '-');
    bool isNegative = (arr[0] == '-');
    int E = 0, len = strlen(arr);
    if(isFractional)
    {
        int k;
        for(i = len-1, j = 0, k = e+1; i > k; i--, j++)
            E += (arr[i]-'0') * pow(10, j);
        // dot == -1说明e前面是整数，此时转小数形式会多一个‘.’，因此需+1
        lenmax = (dot == -1) ? e + E + 1 : e + E;
        char newArr[lenmax+1];
        if(E == 0)
        {
            for(i = 0; i < lenmax; i++)
                newArr[i] = arr[i];
            newArr[lenmax] = '\0';
        }
        else
        {
            newArr[0] = isNegative ? '-' : '0';
            i = isNegative ? 1 : 0;
            newArr[i++] = '0';
            newArr[i++] = '.';
            for(j = E-1; j > 0; j--)
                newArr[i++] = '0';
            j = isNegative ? 1 : 0;
            while (j < e)
            {
                if (arr[j] != '.')
                    newArr[i++] = arr[j];
                j++;
            }
            newArr[lenmax] = '\0';
        }
        printf("%s\n", newArr);
        const char *outputArr = newArr;
        return outputArr;
    }
    else
    {
        for(i = len-1, j = 0; i > e; i--, j++)
            E += (arr[i]-'0') * pow(10, j);
        
        if (dot > 0)
        {
            lenmax = (e-dot-1) <= E ? dot+E : e;
            char newArr[lenmax+1];
            for (i = 0; i < dot; i++)
                newArr[i] = arr[i];
            if (lenmax == dot+E)
            {
                for(i = dot, j = e-1; i < j; i++)
                    newArr[i] = arr[i+1];
                for(i = e-1; i < lenmax; i++)
                    newArr[i] = '0';
            }
            else
            {
                for(i = dot, j = dot+E; i < j; i++)
                    newArr[i] = arr[i+1];
                
                newArr[dot+E] = '.';
                for(i = dot+E+1; i < lenmax; i++)
                    newArr[i] = arr[i];
            }
            newArr[lenmax] = '\0';
            printf("%s\n", newArr);
            const char *outputArr = newArr;
            return outputArr;
        }
        else
        {
            lenmax = e+E;
            char newArr[lenmax];
            for (i = 0; i < e; i++)
                newArr[i] = arr[i];
            for(i = e; i < lenmax; i++)
                newArr[i] = '0';
            // arr = newArr;
            newArr[lenmax] = '\0';
            printf("%s\n", newArr);
            const char *outputArr = newArr;
            return outputArr;
        }
    }
}

const char * align(char arr[], int bias)
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
    printf("%s\n", newArr);
    const char *outputArr = newArr;
    return outputArr;
}

