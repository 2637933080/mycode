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
#define m 400
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

typedef struct LNode {
    stock data;            // 股票信息
    struct LNode* next;    // 指向下一结点
} LNode, *LinkList;

// 全局变量用于ASL计算
LinkList* global_H = nullptr;
int global_elementCount = 0;

void InitList(LinkList* H) {
    for (int i = 0; i < m; i++) {
        H[i] = new LNode;
        H[i]->next = nullptr;
    }
}

int Hash(string sname) {
    int sum = 0;
    for (int i = 0; i < sname.length(); i++) {
        sum += (i * i * int(sname[i]));
    }
    return sum % 397;
}

void ListInsert(LinkList* H, stock s, int& sumCmp) {
    int addr = Hash(s.sname);
    LNode* newNode = new LNode;
    newNode->data = s;
    newNode->next = nullptr;
    
    if (H[addr]->next == nullptr) {
        sumCmp += 1;
        H[addr]->next = newNode;
    } else {
        LNode* current = H[addr]->next;
        int cmpCount = 1;
        while (current->next != nullptr) {
            current = current->next;
            cmpCount++;
        }
        sumCmp += cmpCount;
        current->next = newNode;
    }
}

void ReadDetail(stock& s, string detailDir) {
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

int ReadFile(LinkList* H, int& sumCmp, string infoFilename, string detailDir) {
    global_H = H; // 保存散列表指针用于ASL计算
    
    ifstream infile(infoFilename.c_str());
    
    if (!infile.is_open()) {
        cout << "文件打开失败" << endl;
        return 0;
    }

    char buffer[5000];
    bool flag = false;
    int count = 0;
    
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
        ListInsert(H, temp, sumCmp);
        count++;
    }
    infile.close();
    
    global_elementCount = count; // 保存元素总数
    return count;
}

int SearchHL(LinkList* H, string key) {
    int addr = Hash(key);
    LNode* p = H[addr]->next;
    
    while (p != nullptr) {
        if (p->data.sname == key) {
            return addr;
        }
        p = p->next;
    }
    
    return -1;
}

double GetASL(int sumCmp, int count) {
    if (global_elementCount == 0) return 0.0;
    
    int totalSearchComparisons = 0;
    int actualElementCount = 0;
    
    // 遍历散列表的所有槽位
    for (int i = 0; i < 397; i++) {
        LNode* current = global_H[i]->next;
        int position = 1; // 元素在链表中的位置
        
        // 遍历该槽位的链表
        while (current != nullptr) {
            // 查找第k个元素需要k次比较
            totalSearchComparisons += position;
            actualElementCount++;
            position++;
            current = current->next;
        }
    }
    
    if (actualElementCount > 0) {
        return (double)totalSearchComparisons / actualElementCount;
    }
    
    return 0.0;
}

void Print(LinkList T, string sname) {
    LNode* p = T;
    
    while (p != nullptr) {
        if (p->data.sname == sname) {
            stock s = p->data;
            cout << "股票代码: " << s.stockcode << endl;
            cout << "股票简称: " << s.indusname << endl;
            cout << "英文名: " << s.sname << endl;
            cout << "行业编码: " << s.briname << endl;
            cout << "一级门类: " << s.pricate << endl;
            cout << "二级门类: " << s.seccate << endl;
            cout << "上市交易所: " << s.listexchange << endl;
            cout << "公司全称: " << s.wholecompany << endl;
            cout << "上市日期: " << s.launchdate << endl;
            cout << "省份: " << s.provin << endl;
            cout << "城市: " << s.city << endl;
            cout << "法人: " << s.legalper << endl;
            cout << "地址: " << s.addr << endl;
            cout << "网址: " << s.url << endl;
            cout << "邮箱: " << s.email << endl;
            cout << "电话: " << s.callnum << endl;
            cout << "主营业务: " << s.mainbus << endl;
            cout << "经营范围: " << s.scopeofbus << endl;

            for (int i = 0; i < s.detail_count; i++) {
                stockdetail detail = s.detail[i];
                cout << fixed << setprecision(3);
                cout << "日期: " << detail.date;
                cout << " 开盘价: " << detail.openprice;
                cout << " 收盘价: " << detail.closeprice;
                cout << " 最高价: " << detail.highest;
                cout << " 最低价: " << detail.lowest;
                cout << " 成交量: " << detail.volume;
                cout << " 成交额: " << detail.turnover;
                cout << " 换手率: " << detail.changerate;
                cout << " 涨跌幅: " << detail.risefallrate;
                cout << " 涨跌额: " << detail.risefallamount << endl;
            }
            return;
        }
        p = p->next;
    }
}
