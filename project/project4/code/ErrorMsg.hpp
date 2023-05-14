#pragma once
#include <iostream>
#include <stdlib.h>
using namespace std;


#define SHAPE_NOT_MATCH 0
#define ILLEGAL_MODIFY 1

#define ERROR_DESCRIPTION(type, description) \
    case type: \
        return description; \


static
string errorDescription(int type)
{
    switch (type)
    {
    ERROR_DESCRIPTION(SHAPE_NOT_MATCH, "矩阵大小不匹配!");
    ERROR_DESCRIPTION(ILLEGAL_MODIFY, "非法的结构修改!");
    default:
        return "未知错误!";
    }
}

static
void handleError(int type)
{
    cout << "Error: " << errorDescription(type) << endl;
    exit(1);
}
