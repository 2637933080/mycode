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

typedef struct {
    string relation;    //关系名称
    string rule[10];    //规则数组
} Relation;

int BF(string text, string pattern) {
    // BF字符串匹配算法
    int n = text.length(); // 文本长度
    int m = pattern.length(); // 模式串长度
    
    // 外层循环：控制文本的匹配起始位置
    for (int i = 0; i <= n - m; i++) {
        int j;
        // 内层循环：从当前位置i开始，逐个字符比较文本和模式串
        for (j = 0; j < m; j++) {
            if (text[i + j] != pattern[j]) {
                break; // 遇到不匹配的字符，跳出内层循环，从下一个位置重新开始
            }
        }
        // 如果内层循环完整执行完毕（j==m），说明找到完全匹配
        if (j == m) {
            return i; // 返回模式串在文本中的起始位置
        }
    }
    return -1; // 遍历完所有可能位置仍未找到，返回-1表示未匹配
}

void findAllMatches(string text, string pattern, int positions[], int &count) {
    // 查找模式串在文本中的所有匹配位置
   count = 0; // 初始化匹配次数计数器
    int n = text.length();
    int m = pattern.length();
    int pos = 0; // 当前搜索的起始位置
    
    // 循环搜索，直到文本末尾
    while (pos <= n - m) {
        // 在text从pos开始的子串中搜索pattern
        int foundPos = BF(text.substr(pos), pattern);
        if (foundPos != -1) {
            // 记录找到的位置（需要加上偏移量pos）
            positions[count++] = pos + foundPos;
            // 将搜索起始位置移动到本次匹配位置之后，继续查找
            pos = pos + foundPos + 1;
        } else {
            break; // 未找到，结束搜索
        }
    }
}

bool RelationExtraction(char* text, char* entity1, char* entity2, Relation* r) {
     string t(text);   // 将C风格字符串转换为C++ string便于处理
    string e1(entity1); // 实体1
    string e2(entity2); // 实体2
    bool foundRelation = false; // 是否找到关系的标志
    
    // 查找两个实体在文本中的所有位置
    int pos1[100], pos2[100]; // 存储实体位置的数组
    int count1 = 0, count2 = 0; // 实体出现的次数
    
    // 查找实体e1和e2在文本t中的所有出现位置
    findAllMatches(t, e1, pos1, count1);
    findAllMatches(t, e2, pos2, count2);
    
    // 如果至少有一个实体在文本中不存在，直接返回false
    if (count1 == 0 || count2 == 0) {
        return false;
    }
    
    // 情况1：检查实体1出现在实体2之前的所有组合
    for (int i = 0; i < count1; i++) {
        for (int j = 0; j < count2; j++) {
            // 条件：实体1需在实体2之前，且两者距离不超过200字符（防止无关匹配）
            if (pos1[i] >= pos2[j] || pos2[j] - pos1[i] > 200) {
                continue; // 不满足条件，跳过当前组合
            }
            
            // 提取两个实体之间的文本片段
            string between = t.substr(pos1[i] + e1.length(), 
                                     pos2[j] - (pos1[i] + e1.length()));
            
            // 用所有预定义的关系规则检查此文本片段
            for (int k = 0; k < 10 && r[k].relation != ""; k++) { // 遍历每种关系
                for (int m = 0; m < 10 && r[k].rule[m] != ""; m++) { // 遍历关系的每种表达规则
                    string rulePattern = r[k].rule[m];
                    
                    // 如果关系关键词出现在两个实体之间的文本中
                    if (between.find(rulePattern) != string::npos) {
                        // 输出找到的关系：实体1-关系-实体2
                        cout << e1 << "-" << r[k].relation << "-" << e2 << endl;
                        foundRelation = true;
                        return true; // 找到一个关系后就返回
                    }
                }
            }
        }
    }
    
    // 情况2：检查实体2出现在实体1之前的所有组合（逻辑同情况1，顺序相反）
    for (int i = 0; i < count1; i++) {
        for (int j = 0; j < count2; j++) {
            if (pos2[j] >= pos1[i] || pos1[i] - pos2[j] > 200) {
                continue;
            }
            
            string between = t.substr(pos2[j] + e2.length(), 
                                     pos1[i] - (pos2[j] + e2.length()));
            
            for (int k = 0; k < 10 && r[k].relation != ""; k++) {
                for (int m = 0; m < 10 && r[k].rule[m] != ""; m++) {
                    string rulePattern = r[k].rule[m];
                    
                    if (between.find(rulePattern) != string::npos) {
                        cout << e1 << "-" << r[k].relation << "-" << e2 << endl;
                        foundRelation = true;
                        return true;
                    }
                }
            }
        }
    }
    
    return foundRelation; // 返回是否找到关系
}

void InitRelation(Relation *r){
    //关系初始化函数 
    // 初始化第一个关系：属于行业大类
    r[0].relation = "属于行业大类";
    r[0].rule[0] = "行业类别为";
    r[0].rule[1] = "属于";
    r[0].rule[2] = ""; // 空字符串表示规则结束

    // 初始化第二个关系：所属机构
    r[1].relation = "所属机构";
    r[1].rule[0] = "所属机构为";
    r[1].rule[1] = "的公司是";
    r[1].rule[2] = "隶属";
    r[1].rule[3] = "";

    // 初始化第三个关系：具有注册地址
    r[2].relation = "具有注册地址";
    r[2].rule[0] = "位于";
    r[2].rule[1] = "坐落于";
    r[2].rule[2] = "总部在";
    r[2].rule[3] = "";
    
    // 初始化第四个关系：主营业务为
    r[3].relation = "主营业务为";
    r[3].rule[0] = "包括";
    r[3].rule[1] = "专注于";
    r[3].rule[2] = "主营业务为";
    r[3].rule[3] = "";
    
    // 标记关系规则数组的结束
    r[4].relation = "";
}
