#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <stdexcept>
#define MAXSIZE 10000
using namespace std;

bool EntityRecognition(const char* S, const char* T) {
    int lenS = strlen(S);
    int lenT = strlen(T);
    bool hasMatch = false;
    
    // 遍历文本的每个可能起始位置
    for (int pos = 0; pos <= lenS; pos++) {
        int i = pos;  // 文本指针
        int j = 0;    // 规则指针
        bool matched = true;
        string entity = "";  // 存储提取的实体
        
        while (j < lenT && i < lenS) {
            if (T[j] == '*') {
                // * 匹配一个完整的中文字符（3个字节）
                if (i + 2 < lenS) {
                    // 提取*对应的中文字符
                    for (int k = 0; k < 3; k++) {
                        entity += S[i + k];
                    }
                    i += 3;
                    j++;
                } else {
                    matched = false;
                    break;
                }
            } else {
                // 普通字符需要精确匹配
                if (S[i] == T[j]) {
                    i++;
                    j++;
                } else {
                    matched = false;
                    break;
                }
            }
        }
        
        // 检查是否完全匹配
        if (matched && j == lenT) {
            hasMatch = true;
            cout << entity << endl;
            // 继续从下一个位置搜索
        }
    }
    
    return hasMatch;
}