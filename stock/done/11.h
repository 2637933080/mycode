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
#define MAXSIZE 5000  // 定义顺序表最大容量
using namespace std;

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

void InitList(SqList &L) {
// 初始化顺序表
    L.elem = new stock[MAXSIZE];
    L.length = 0;
}

void FreeList(SqList &L) {
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

void ReadFile(SqList &L, string infoFilename, string detailDir) {
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
                case 0: temp.stockcode = ss.substr(5); break;
                case 1: temp.indusname = ss.substr(13); break;
                case 2: temp.sname = ss.substr(10); break;
                case 3: temp.wholecompany = ss.substr(13); break;
                case 4: temp.listexchange = ss.substr(16); break;
                case 5: temp.briname = ss.substr(13); break;
                case 6: temp.pricate = ss.substr(13); break;
                case 7: temp.seccate = ss.substr(16); break;
                case 8: temp.launchdate = ss.substr(13); break;
                case 9: temp.provin = ss.substr(7); break;
                case 10: temp.city = ss.substr(7); break;
                case 11: temp.legalper = ss.substr(7); break;
                case 12: temp.addr = ss.substr(7); break;
                case 13: temp.url = ss.substr(7); break;
                case 14: temp.email = ss.substr(7); break;
                case 15: temp.callnum = ss.substr(7); break;
                case 16: temp.mainbus = ss.substr(13); break;
                case 17: temp.scopeofbus = ss.substr(13); break;
            }
            type++;
        }
        
        ReadDetail(temp, detailDir);
        L.elem[L.length] = temp;
        L.length++;
    }
    infile.close();
}

void HeapAdjust(SqList &L, int s, int m, int &kcn, int &rmn) {
// 堆调整函数：调整子树成为大根堆
    stock rc = L.elem[s];
    rmn++;
    
    for (int j = 2 * s; j <= m; j *= 2) {
        if (j < m) {
            kcn++;
            if (L.elem[j].detail[0].volume < L.elem[j + 1].detail[0].volume) {
                j++;
            }
        }
        
        kcn++;
        if (rc.detail[0].volume >= L.elem[j].detail[0].volume) {
            break;
        }
        
        L.elem[s] = L.elem[j];
        rmn++;
        s = j;
    }
    
    L.elem[s] = rc;
    rmn++;
}

void CreatHeap(SqList &L, int &kcn, int &rmn) {
// 建堆函数：将无序序列构建成大根堆
    if (L.length <= 1) return;
    
    for (int i = L.length / 2; i >= 1; i--) {
        HeapAdjust(L, i, L.length, kcn, rmn);
    }
}

