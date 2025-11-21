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
	string date;//日期 
	double openprice;//开盘价
	double closeprice;//收盘价
	double highest;//最高价
	double lowest;//最低价
	double volume;//成交量
	double turnover;//成交额
	string changerate;//换手率
	string risefallrate;//涨跌幅
	double risefallamount;//涨跌额
}stockdetail;//股票的价格信息 

typedef struct stock{
	string stockcode;//股票代码
	string indusname;//股票简称
	string sname;//英文名 
	string briname;//行业编码
    string pricate;//一级门类（即行业大类）
    string seccate;//二级门类（即行业二级类）
    string listexchange;//上市交易所
    string wholecompany;//公司全称（即机构名称）
	string launchdate;//上市日期
	string provin;//省份
	string city;//城市
	string legalper;//法人
	string addr;//地址
	string url;//网址
	string email;//邮箱
	string callnum;//电话
	string mainbus;//主营业务
	string scopeofbus;//经营范围
	stockdetail *detail;//定义一个stockdetail类型的数组用来存放该股票每一日的相关价格信息  
	int detail_count;//detail数组的长度 
}stock;

typedef struct {
    stock* elem;                 // 指向数组的指针
    int length;                 // 数组的长度
} SqList;

typedef struct TNode {			//字典树 
    stock* stockPtr;              // 股票指针
    struct TNode* child[53];    // 子结点的指针数组，由26个小写字母，26个大写字母，1个空格组成
} TNode, *TrieTree;

void InitList(SqList& L) {
// 使用动态内存分配new进行初始化
    L.elem = new stock[MAXSIZE];
    L.length = 0;
}

void FreeList(SqList& L) 
{// 释放内存
    for (int i = 0; i < L.length; i++) {
        if (L.elem[i].detail != nullptr) {
            delete[] L.elem[i].detail;
        }
    }
    delete[] L.elem;
    L.length = 0;
}

TNode* InitTNode() 
{// 初始化字典树结点
    TNode* node = new TNode;
    node->stockPtr = nullptr;
    for (int i = 0; i < 53; i++) {
        node->child[i] = nullptr;
    }
    return node;
}

TNode* BuildTree(SqList& L) 
{// 构建基于链式存储的字典树
// 构建成功后返回指向根结点的指针
    TNode* root = InitTNode();
    for (int i = 0; i < L.length; i++) {
        TNode* p = root;
        for (int j = 0; j < L.elem[i].sname.length(); j++) {
            char ch = L.elem[i].sname[j];
            int x;
            if (ch == ' ')
                x = 52;
            else if (ch >= 'a' && ch <= 'z')
                x = ch - 'a';
            else if (ch >= 'A' && ch <= 'Z')
                x = ch - 'A' + 26;
            else
                continue;  // 跳过非法字符
                
            if (p->child[x] == nullptr) {
                p->child[x] = InitTNode();
            }
            p = p->child[x];
        }
        p->stockPtr = &L.elem[i];
    }
    return root;
}

TNode* SearchPrefixNode(TNode* root, string prefix) 
{// 查找前缀对应的节点
    TNode* p = root;
    for (int i = 0; i < prefix.length(); i++) {
        char ch = prefix[i];
        int x;
        if (ch == ' ')
            x = 52;
        else if (ch >= 'a' && ch <= 'z')
            x = ch - 'a';
        else if (ch >= 'A' && ch <= 'Z')
            x = ch - 'A' + 26;
        else
            return nullptr;  // 包含非法字符
            
        if (p->child[x] == nullptr) {
            return nullptr;  // 前缀不存在
        }
        p = p->child[x];
    }
    return p;
}

void ReadDetail(stock& s, string detailDir) 
{// 从文件夹Detail中读取Detail信息
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
        ss >> detail.closeprice;  // 修正：closepprice -> closeprice
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
// 从文件中读取中股票信息，将其按顺序存入L.elem指向的数组中，数组下标从1开始存储
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

void CollectStocks(TNode* node, stock** stocks, int& count) 
{// 收集所有以该结点为根的子树中的股票
    if (node == nullptr) return;
    
    // 如果当前节点是某个股票的结尾，添加到结果中
    if (node->stockPtr != nullptr) {
        stocks[count] = node->stockPtr;
        count++;
    }
    
    // 递归遍历所有子节点
    for (int i = 0; i < 53; i++) {
        if (node->child[i] != nullptr) {
            CollectStocks(node->child[i], stocks, count);
        }
    }
}

double GetASL(SqList& L) 
{// 计算查找成功时的平均查找长度ASL
    double totalLength = 0.0;
    int totalStocks = L.length;
    
    for (int i = 0; i < L.length; i++) {
        totalLength += L.elem[i].sname.length();
    }
    
    if (totalStocks == 0) return 0.0;
    return totalLength / totalStocks;
}

void Print(stock* s) 
{// 输出股票信息，double类型变量保留3位小数，详见输出样例
    if (s == nullptr) return;
    
    cout << "股票代码: " << s->stockcode << endl;
    cout << "股票简称: " << s->indusname << endl;
    cout << "英文名: " << s->sname << endl;
    cout << "行业编码: " << s->briname << endl;
    cout << "一级门类: " << s->pricate << endl;
    cout << "二级门类: " << s->seccate << endl;
    cout << "上市交易所: " << s->listexchange << endl;
    cout << "公司全称: " << s->wholecompany << endl;
    cout << "上市日期: " << s->launchdate << endl;
    cout << "省份: " << s->provin << endl;
    cout << "城市: " << s->city << endl;
    cout << "法人: " << s->legalper << endl;
    cout << "地址: " << s->addr << endl;
    cout << "网址: " << s->url << endl;
    cout << "邮箱: " << s->email << endl;
    cout << "电话: " << s->callnum << endl;
    cout << "主营业务: " << s->mainbus << endl;
    cout << "经营范围: " << s->scopeofbus << endl;

    // 输出详细信息
    for (int i = 0; i < s->detail_count; i++) {
        stockdetail detail = s->detail[i];
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
}
