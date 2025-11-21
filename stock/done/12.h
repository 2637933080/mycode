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
#include <functional>
#define MAXSIZE 5000
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
    string briname;         // 行业编码
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
    stockdetail *detail;    // 每日交易详情数组指针
    int detail_count;       // 交易详情数量
} stock;

// 日期-成交量结构体 
typedef struct DailyVolume {
    string date;            // 交易日期
    double volume;          // 成交量
} DailyVolume;

// 顺序表结构
typedef struct {
    stock* elem;            // 股票数组指针
    int length;             // 当前股票数量
} SqList;

// 顺序表结构
typedef struct {
    DailyVolume* elem;      // 日期-成交量数组指针
    int length;             // 当前日期-成交量数量 
} SqListDailyVolume;

void InitList(SqList &L) {
// 使用动态内存分配new进行初始化
    L.elem = new stock[MAXSIZE];
    L.length = 0;
    int idx = 0;
    while (idx < MAXSIZE) {
        L.elem[idx].detail = nullptr;
        L.elem[idx].detail_count = 0;
        idx++;
    }
}

void FreeList(SqList &L) {
// 释放内存
    if (!L.elem) return;
    int idx = 1;
    while (idx <= L.length) {
        if (L.elem[idx].detail != nullptr) {
            delete[] L.elem[idx].detail;
            L.elem[idx].detail = nullptr;
            L.elem[idx].detail_count = 0;
        }
        idx++;
    }
    delete[] L.elem;
    L.elem = nullptr;
    L.length = 0;
}

void InitDailyVolumeList(SqListDailyVolume &DL) {
// 使用动态内存分配new进行初始化
    DL.elem = new DailyVolume[MAXSIZE];
    DL.length = 0;
    int pos = 0;
    while (pos < MAXSIZE) {
        DL.elem[pos].date = "";
        DL.elem[pos].volume = 0.0;
        pos++;
    }
}

void FreeDailyVolume(SqListDailyVolume &DL) {
// 释放内存
    if (DL.elem) {
        delete[] DL.elem;
        DL.elem = nullptr;
        DL.length = 0;
    }
}

void ReadDetail(stock& s, string detailDir) {
    // 从文件夹Detail中读取Detail信息 
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
    // 从文件中读取股票信息，将其按顺序存入L.elem指向的数组中
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

void MergeSort(SqListDailyVolume& DL, int& kcn, int& rmn) {
// 对DL做归并排序，并计算kcn和rmn
    if (DL.length <= 1) return;
    vector<DailyVolume> mergeTemp(DL.length + 1);
    long long cmpCount = 0;
    function<void(int, int)> mergeSort = [&](int left, int right) {
        if (left >= right) return;
        int mid = (left + right) / 2;
        mergeSort(left, mid);
        mergeSort(mid + 1, right);
        int iLeft = left, iRight = mid + 1, iMerge = left;
        while (iLeft <= mid && iRight <= right) {
            cmpCount++;
            if (DL.elem[iLeft].volume <= DL.elem[iRight].volume) {
                mergeTemp[iMerge++] = DL.elem[iLeft++];
            } else {
                mergeTemp[iMerge++] = DL.elem[iRight++];
            }
        }
        while (iLeft <= mid) mergeTemp[iMerge++] = DL.elem[iLeft++];
        while (iRight <= right) mergeTemp[iMerge++] = DL.elem[iRight++];
        for (int t = left; t <= right; t++) {
            DL.elem[t] = mergeTemp[t];
            rmn++;
        }
    };
    mergeSort(1, DL.length);
    kcn = (int)cmpCount;
}

void industryDailyVolume(SqList &L, SqListDailyVolume& DL, string briname, int &kcn, int &rmn) {
// 按日期累加行业briname中所有股票的每日成交量，按日期-成交量对存入DL中，并按成交量从小到大排序	
    unordered_map<string, double> volSums;
    for (int stockIdx = 1; stockIdx <= L.length; stockIdx++) {
        if (L.elem[stockIdx].briname != briname) continue;
        stock &currStock = L.elem[stockIdx];
        for (int detailIdx = 0; detailIdx < currStock.detail_count; detailIdx++) {
            string currDate = currStock.detail[detailIdx].date;
            double currVol = currStock.detail[detailIdx].volume;
            volSums[currDate] += currVol;
        }
    }
    vector<pair<string, double>> dvPairs;
    dvPairs.reserve(volSums.size());
    for (auto &entry : volSums) dvPairs.emplace_back(entry.first, entry.second);

    // 按日期升序排列（不把 std::sort 的内部比较计入 kcn）
    sort(dvPairs.begin(), dvPairs.end(), [](const pair<string, double>& a, const pair<string, double>& b) {
        return a.first.compare(b.first) > 0;
    });

    DL.length = 0;
    for (size_t idx = 0; idx < dvPairs.size(); idx++) {
        DL.elem[idx + 1].date = dvPairs[idx].first;
        DL.elem[idx + 1].volume = dvPairs[idx].second;
    }
    DL.length = (int)dvPairs.size();

    // 归并排序并统计（MergeSort 只统计归并阶段的比较与写回）
    kcn = 0;
    rmn = 0;
    MergeSort(DL, kcn, rmn);
}