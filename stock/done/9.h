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
#define MAXSIZE 5000
using namespace std;

typedef struct stockdetail{
    string date;           // 日期
    double openprice;      // 开盘价
    double closeprice;     // 收盘价
    double highest;        // 最高价
    double lowest;         // 最低价
    double volume;         // 成交量
    double turnover;       // 成交额
    string changerate;     // 换手率
    string risefallrate;   // 涨跌幅
    double risefallamount; // 涨跌额
} stockdetail;

typedef struct stock{
    string stockcode;      // 股票代码
    string indusname;      // 股票简称
    string sname;          // 英文名
    string briname;        // 行业编码
    string pricate;        // 一级门类（即行业大类）
    string seccate;        // 二级门类（即行业二级类）
    string listexchange;   // 上市交易所
    string wholecompany;   // 公司全称（即机构名称）
    string launchdate;     // 上市日期
    string provin;         // 省份
    string city;           // 城市
    string legalper;       // 法人
    string addr;           // 地址
    string url;            // 网址
    string email;          // 邮箱
    string callnum;        // 电话
    string mainbus;        // 主营业务
    string scopeofbus;     // 经营范围
    stockdetail *detail;   // 详细价格信息数组
    int detail_count;      // 数组长度
} stock;

typedef struct {
    stock* elem;           // 指向数组的指针
    int length;           // 数组的长度
} SqList;

// 链表节点结构（用于存储筛选后的股票信息）
struct StockNode {
    string stockcode;       // 股票代码
    string indusname;       // 股票简称
    string induscode;       // 行业编码
    string date;            // 交易日期
    double risefallamount;  // 涨跌额
    StockNode* next;        // 指向下一个节点的指针
};

void InitList(SqList& L) {
    // 使用动态内存分配new进行初始化
    L.elem = new stock[MAXSIZE];
    L.length = 0;
}

void FreeList(SqList& L) {
    // 释放内存
    for (int i = 0; i < L.length; i++) {
        if (L.elem[i].detail != nullptr) {
            delete[] L.elem[i].detail;
        }
    }
    delete[] L.elem;
    L.length = 0;
}

void ReadDetail(stock& s, string detailDir) {
    // 从文件夹Detail中读取Detail信息
    string filename = detailDir + "/" + s.stockcode + ".txt";
    ifstream file(filename.c_str());
    
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
    // 从文件中读取股票信息，将其按顺序存入L.elem指向的数组中
    ifstream infile(infoFilename.c_str());
    
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

StockNode* SplitStockListByRiseFall(SqList& L, const string& induscode, const string& targetdate) {
    // 初始化四个指针，用于管理两个链表
    StockNode* negativeHead = nullptr;    // 负值组头指针
    StockNode* negativeTail = nullptr;    // 负值组尾指针
    StockNode* nonNegativeHead = nullptr; // 非负值组头指针
    StockNode* nonNegativeTail = nullptr; // 非负值组尾指针
    
    // 第一层遍历：遍历所有股票
    for (int i = 0; i < L.length; i++) {
        stock s = L.elem[i];
        
        // 第一级筛选：行业编码匹配
        if (s.briname != induscode) {
            continue;// 行业不匹配，跳过该股票
        }
        
        // 检查该股票是否有详细信息
        if (s.detail == nullptr || s.detail_count == 0) {
            continue;// 无交易数据，跳过
        }
        
         // 第二层遍历：遍历该股票的所有交易记录
        for (int j = 0; j < s.detail_count; j++) {
            stockdetail detail = s.detail[j];
            
            // 第二级筛选：交易日期匹配
            if (detail.date != targetdate) {
                continue;// 日期不匹配，跳过该记录
            }
            
             // 创建新节点并填充数据
            StockNode* newNode = new StockNode;
            newNode->stockcode = s.stockcode;
            newNode->indusname = s.indusname;
            newNode->induscode = s.briname;
            newNode->date = detail.date;
            newNode->risefallamount = detail.risefallamount;
            newNode->next = nullptr;
            
             // 核心分组逻辑：根据涨跌额正负分配到不同链表
            if (detail.risefallamount < 0) {
                // 负值组 - 插入到负值组尾部
                if (negativeHead == nullptr) {
                    negativeHead = newNode;// 链表为空，新节点作为头尾
                    negativeTail = newNode;
                } else {
                    negativeTail->next = newNode;// 链表非空，追加到尾部
                    negativeTail = newNode;
                }
            } else {
                // 插入非负值组链表尾部
                if (nonNegativeHead == nullptr) {
                    nonNegativeHead = newNode;
                    nonNegativeTail = newNode;
                } else {
                    nonNegativeTail->next = newNode;
                    nonNegativeTail = newNode;
                }
            }
        }
    }
    
    // 连接两个分组：负值组在前，非负值组在后
    if (negativeHead == nullptr) {
        return nonNegativeHead; // 没有负值股票，直接返回非负组
    } else {
        negativeTail->next = nonNegativeHead; // 连接两个链表
        return negativeHead; // 返回合并后的链表头
    }
}