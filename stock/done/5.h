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

typedef struct BSTNode {
    stock data;                  // 股票信息
    struct BSTNode* lchild;     // 左孩子指针
    struct BSTNode* rchild;     // 右孩子指针
} BSTNode, *BSTree;

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

int Partition(SqList& L, int low, int high, int& kcn, int& rmn) {
    stock pivot = L.elem[low];
    rmn++; // 枢轴暂存
    
    while (low < high) {
        // 从右向左找小于pivot的元素
        while (low < high) {
            kcn++;
            if (L.elem[high].sname < pivot.sname) {
                break;
            }
            high--;
        }
        L.elem[low] = L.elem[high];
        rmn++;
        
        // 从左向右找大于pivot的元素
        while (low < high) {
            kcn++;
            if (L.elem[low].sname > pivot.sname) {
                break;
            }
            low++;
        }
        L.elem[high] = L.elem[low];
        rmn++;
    }
    
    L.elem[low] = pivot;
    rmn++;
    return low;
}

void QSort(SqList& L, int low, int high, int& kcn, int& rmn) {
    if (low < high) {
        int pivotloc = Partition(L, low, high, kcn, rmn);
        QSort(L, low, pivotloc - 1, kcn, rmn);
        QSort(L, pivotloc + 1, high, kcn, rmn);
    }
}

void QuickSort(SqList& L, int& kcn, int& rmn) {
    kcn = 0;
    rmn = 0;
    if (L.length > 0) {
        QSort(L, 0, L.length - 1, kcn, rmn);
    }
}

void InitBSTree(BSTree& T) {
    // 二叉排序树初始化
    T = nullptr;
}

void InsertBST(BSTree& T, stock e) {
    // 当二叉排序树T中不存在关键字等于e.sname的数据元素时，则插入该元素
    if (T == nullptr) {
        T = new BSTNode;
        T->data = e;
        T->lchild = T->rchild = nullptr;
    } else if (e.sname < T->data.sname) {
        InsertBST(T->lchild, e);
    } else if (e.sname > T->data.sname) {
        InsertBST(T->rchild, e);
    }
    // 如果相等，则不插入（已存在）
}

void buildTree(BSTree& T, SqList L) {
    // 根据L中的信息构建按英文名称二叉排序树 
    InitBSTree(T);
    for (int i = 0; i < L.length; i++) {
        InsertBST(T, L.elem[i]);
    }
}

void Print(BSTNode* T) {
    // 输出股票信息，double类型变量保留3位小数，详见输出样例
    if (T == nullptr) return;
    
    stock s = T->data;
    
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

    // 输出详细信息
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
}

BSTNode* SearchBST(BSTree& T, string sname) {
    // 查找对应股票，查找成功返回对应的树结点，否则返回NULL
    BSTNode* p = T;
    while (p != nullptr) {
        if (sname == p->data.sname) {
            return p;
        } else if (sname < p->data.sname) {
            p = p->lchild;
        } else {
            p = p->rchild;
        }
    }
    return nullptr;
}

int GetSumCmp(BSTree T, int sumCmp) {
    // 统计查找成功时的总比较次数
    if (T == nullptr) return 0;
    
    // 计算当前节点的深度（比较次数）
    int depth = 1;
    BSTNode* p = T;
    while (p != nullptr) {
        depth++;
        p = p->lchild; // 任意路径，这里只是估算
    }
    
    // 递归计算左右子树
    return depth + GetSumCmp(T->lchild, sumCmp) + GetSumCmp(T->rchild, sumCmp);
}

double GetASL(BSTree& T, int count) {
    // 返回基于二叉排序树查找的ASL
    if (T == nullptr || count == 0) return 0;
    
    // 计算所有节点的深度之和
    int totalDepth = 0;
    
    // 使用层次遍历计算每个节点的深度
    queue<pair<BSTNode*, int>> q; // 节点和深度
    q.push(make_pair(T, 1));
    
    while (!q.empty()) {
        pair<BSTNode*, int> front = q.front();
        q.pop();
        
        BSTNode* node = front.first;
        int depth = front.second;
        
        totalDepth += depth;
        
        if (node->lchild != nullptr) {
            q.push(make_pair(node->lchild, depth + 1));
        }
        if (node->rchild != nullptr) {
            q.push(make_pair(node->rchild, depth + 1));
        }
    }
    
    // ASL = 总深度 / 节点数
    return (double)totalDepth / count;
}