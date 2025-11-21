#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
using namespace std;

#define MAXSIZE 5000  // 定义最大股票数量

// 股票每日交易详情结构体
typedef struct stockdetail {
    string date;            // 交易日期
    double openprice;       // 开盘价
    double closeprice;      // 收盘价
    double highest;         // 最高价
    double lowest;          // 最低价
    double volume;          // 成交量
    double turnover;        // 成交额
    string changerate;      // 换手率
    string risefallrate;    // 涨跌幅
    double risefallamount;  // 涨跌额
} stockdetail;

// 股票基本信息结构体
typedef struct stock {
    string stockcode;       // 股票代码
    string indusname;       // 股票简称
    string sname;           // 英文名
    string briname;         // 行业编码（用于分类的关键字段）
    string pricate;         // 一级门类（即行业大类）
    string seccate;         // 二级门类（即行业二级类）
    string listexchange;    // 上市交易所
    string wholecompany;    // 公司全称（即机构名称）
    string launchdate;      // 上市日期
    string provin;          // 省份
    string city;            // 城市
    string legalper;        // 法人
    string addr;            // 地址
    string url;             // 网址
    string email;           // 邮箱
    string callnum;         // 电话
    string mainbus;         // 主营业务
    string scopeofbus;      // 经营范围
    stockdetail *detail;    // 指向每日交易详情数组的指针
    int detail_count;       // 交易详情记录数量
} stock;

// 顺序表结构（存储股票集合）
typedef struct {
    stock *elem;            // 指向股票数组的指针
    int length;             // 当前存储的股票数量
} SqList;

void InitList(SqList& L) {
// 初始化顺序表
    L.elem = new stock[MAXSIZE];
    L.length = 0;
}

void FreeList(SqList& L) {
// 释放顺序表内存
    for (int i = 0; i < L.length; i++) {
        if (L.elem[i].detail != nullptr) {
            delete[] L.elem[i].detail;
        }
    }
    delete[] L.elem;
    L.length = 0;
}

void ReadDetail(stock &s, string detailDir) {
// 从Detail文件夹中读取Detail信息 
    string filename = detailDir + "/" + s.stockcode + ".txt";
    ifstream file;
    file.open(filename.c_str());
    
    if (!file.is_open()) {
        s.detail_count = 0;
        s.detail = nullptr;
        return;
    }

    vector<stockdetail> details;
    string line;
    
    while (getline(file, line)) {
        if (line.empty()) continue;

        for (char& c : line) {
            if (c == ';') c = ' ';
        }

        stockdetail detail;
        stringstream ss(line);

        ss >> detail.date;
        ss >> detail.openprice;
        ss >> detail.closeprice;
        ss >> detail.highest;
        ss >> detail.lowest;
        ss >> detail.volume;
        ss >> detail.turnover;
        ss >> detail.changerate;
        ss >> detail.risefallrate;
        ss >> detail.risefallamount;

        details.push_back(detail);
    }
    file.close();

    s.detail_count = details.size();
    s.detail = new stockdetail[s.detail_count];
    for (int i = 0; i < details.size(); i++) {
        s.detail[i] = details[i];
    }
}

void ReadFile(SqList& L, string infoFilename, string detailDir) {
// 从文件中读取股票信息，将其按顺序存入L.elem指向的数组中，数组下标从1开始存储
    ifstream infile;
    infile.open(infoFilename.c_str());
    
    if (!infile.is_open()) {
        cout << "文件打开失败" << endl;
        return;
    }

    char buffer[5000];
    bool flag = false;
    
    while (infile.getline(buffer, sizeof(buffer), '#')) {
        stock temp;
        stringstream streams(buffer);
        string ss;
        int type = 0;
        
        if (flag) {
            getline(streams, ss, '\n');
        }
        flag = true;
        
        while (getline(streams, ss, '\n')) {
            if (ss.empty()) continue;
            
            switch (type) {
                case 0:  // 股票代码
                    temp.stockcode = ss.substr(5);
                    break;
                case 1:  // 股票简称
                    temp.indusname = ss.substr(13);
                    break;
                case 2:  // 英文名
                    temp.sname = ss.substr(10);
                    break;
                case 3:  // 公司全称
                    temp.wholecompany = ss.substr(13);
                    break;
                case 4:  // 上市交易所
                    temp.listexchange = ss.substr(16);
                    break;
                case 5:  // 行业编码
                    temp.briname = ss.substr(13);
                    break;
                case 6:  // 一级门类
                    temp.pricate = ss.substr(13);
                    break;
                case 7:  // 二级门类
                    temp.seccate = ss.substr(16);
                    break;
                case 8:  // 上市日期
                    temp.launchdate = ss.substr(13);
                    break;
                case 9:  // 省份
                    temp.provin = ss.substr(7);
                    break;
                case 10: // 城市
                    temp.city = ss.substr(7);
                    break;
                case 11: // 法人
                    temp.legalper = ss.substr(7);
                    break;
                case 12: // 地址
                    temp.addr = ss.substr(7);
                    break;
                case 13: // 网址
                    temp.url = ss.substr(7);
                    break;
                case 14: // 邮箱
                    temp.email = ss.substr(7);
                    break;
                case 15: // 电话
                    temp.callnum = ss.substr(7);
                    break;
                case 16: // 主营业务
                    temp.mainbus = ss.substr(13);
                    break;
                case 17: // 经营范围
                    temp.scopeofbus = ss.substr(13);
                    break;
                default:
                    break;
            }
            type++;
        }
        
        ReadDetail(temp, detailDir);
        L.elem[L.length] = temp;
        L.length++;
    }
    infile.close();
}

void Init_UFSet(int UFSet[], int size) {
// 初始化并查集
    for (int i = 0; i < size; i++) {
        UFSet[i] = -1;  // 每个元素初始化为根节点，值为负数表示集合大小
    }
}

void Union_UFSet(int UFSet[], int Root1, int Root2) {
// 合并两个子集（Root2成为新根）
    if (UFSet[Root1] > UFSet[Root2]) {
        // Root2的集合更大，将Root1合并到Root2
        UFSet[Root2] += UFSet[Root1];
        UFSet[Root1] = Root2;
    } else {
        // Root1的集合更大，将Root2合并到Root1
        UFSet[Root1] += UFSet[Root2];
        UFSet[Root2] = Root1;
    }
}

int CompressionFind(int UFSet[], int x) {
// 带路径压缩的查找
    if (UFSet[x] < 0) {
        return x;  // 找到根节点
    } else {
        // 路径压缩：将x的父节点直接设置为根节点
        UFSet[x] = CompressionFind(UFSet, UFSet[x]);
        return UFSet[x];
    }
}

void ClassifyByIndustry(SqList& L, int parent[], string industries[], int firstOccurrence[], int& industryCount) {
// 按行业分类股票（使用并查集）
    Init_UFSet(parent, L.length);
    industryCount = 0;
    
    // 使用map记录每个行业编码第一次出现的位置
    map<string, int> industryMap;
    
    for (int i = 0; i < L.length; i++) {
        string currentIndustry = L.elem[i].briname;
        
        if (industryMap.find(currentIndustry) == industryMap.end()) {
            // 新的行业编码，记录第一次出现的位置
            industryMap[currentIndustry] = i;
            industries[industryCount] = currentIndustry;
            firstOccurrence[industryCount] = i;
            industryCount++;
        } else {
            // 已存在的行业编码，合并到对应的集合
            int root1 = CompressionFind(parent, i);
            int root2 = CompressionFind(parent, industryMap[currentIndustry]);
            
            if (root1 != root2) {
                Union_UFSet(parent, root1, root2);
            }
        }
    }
}

double CalculateTotalChangeRate(SqList& L, int parent[], string targetIndustry, string targetDate) {
// 计算指定行业在指定日期的总换手率
    double totalChangeRate = 0.0;
    int targetRoot = -1;
    
    // 找到目标行业的根节点
    for (int i = 0; i < L.length; i++) {
        if (L.elem[i].briname == targetIndustry) {
            targetRoot = CompressionFind(parent, i);
            break;
        }
    }
    
    if (targetRoot == -1) {
        return 0.0;  // 未找到目标行业
    }
    
    // 遍历所有股票，计算属于目标行业的股票的换手率总和
    for (int i = 0; i < L.length; i++) {
        if (CompressionFind(parent, i) == targetRoot) {
            // 该股票属于目标行业
            stock s = L.elem[i];
            
            // 查找指定日期的交易记录
            for (int j = 0; j < s.detail_count; j++) {
                if (s.detail[j].date == targetDate) {
                    // 将换手率字符串转换为double
                    string changeRateStr = s.detail[j].changerate;
                    double changeRate = 0.0;
                    
                    // 处理换手率字符串（可能包含%等字符）
                    if (!changeRateStr.empty()) {
                        // 移除可能的%符号
                        size_t percentPos = changeRateStr.find('%');
                        if (percentPos != string::npos) {
                            changeRateStr = changeRateStr.substr(0, percentPos);
                        }
                        
                        // 转换为double
                        stringstream ss(changeRateStr);
                        ss >> changeRate;
                    }
                    
                    totalChangeRate += changeRate;
                    break;  // 找到该日期的记录后跳出内层循环
                }
            }
        }
    }
    
    return totalChangeRate;
}