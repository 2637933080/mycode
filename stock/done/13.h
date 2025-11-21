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
#define m 300
using namespace std;

typedef struct stockdetail {
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
typedef struct stock {
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
	stockdetail* detail;//定义一个stockdetail类型的数组用来存放该股票每一日的相关价格信息  
	int detail_count;//detail数组的长度 
}stock;
typedef struct BiTNode {
	stock data;
	struct BiTNode* lchild, * rchild;
	string disition;
} BiTNode, * BiTree;
typedef struct {
	stock* elem;                 // 指向数组的指针
	int length;                 // 数组的长度
} SqList;

void InitList(SqList& L) {
	// 初始化顺序表
	L.elem = new stock[MAXSIZE + 1];  // 下标从1开始使用
	L.length = 0;
}

void FreeList(SqList& L) {
	// 释放顺序表内存
	for (int i = 1; i <= L.length; i++) {
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

void InitTree(BiTree& BT)
{//初始化二叉树
	BT = new BiTNode;
	BT->lchild = NULL;
	BT->rchild = NULL;
	BT->disition = "二级门类为计算机、通信和其他电子设备制造业";

	BiTree b1 = new BiTNode;
	b1->disition = "最近一天跌涨幅>0";
	b1->lchild = NULL;
	b1->rchild = NULL;

	BiTree b2 = new BiTNode;
	b2->disition = "最近一天跌涨幅<=0";
	b2->lchild = NULL;
	b2->rchild = NULL;

	BiTree b3 = new BiTNode;
	b3->disition = "最近一天成交量>1000万股";
	b3->lchild = NULL;
	b3->rchild = NULL;

	BiTree b4 = new BiTNode;
	b4->disition = "最近一天成交量<=1000万股";
	b4->lchild = NULL;
	b4->rchild = NULL;

	BiTree b5 = new BiTNode;
	b5->disition = "二级门类不为计算机、通信和其他电子设备制造业";
	b5->lchild = NULL;
	b5->rchild = NULL;

	BT->lchild = b1;
	BT->rchild = b5;
	b1->lchild = b3;
	b1->rchild = b4;
	b3->lchild = b2;
	b3->rchild = NULL;
	b4->lchild = NULL;
	b4->rchild = NULL;
	b5->lchild = NULL;
	b5->rchild = NULL;
}

void insertNodeToTree(BiTree& root, BiTree& node)
{//兄弟结点插入二叉树
	if (root == nullptr) {
		root = node;
		return;
	}

	// 如果当前节点没有右孩子，直接插入为右孩子
	if (root->rchild == nullptr) 
    {
		root->rchild = node;
	}
	else {
		// 否则递归插入到右子树
		insertNodeToTree(root->rchild, node);
	}
}

void createDecisionTree(BiTree& BT, SqList L)
{//构建决策树
    // 清空现有数据
    if (BT->lchild && BT->lchild->lchild) {
        BT->lchild->lchild->lchild = nullptr;
    }
    if (BT->lchild && BT->lchild->rchild) {
        BT->lchild->rchild->lchild = nullptr;
        BT->lchild->rchild->rchild = nullptr;
    }

    // 为每个股票创建节点并插入到决策树
    for (int i = 1; i <= L.length; i++) {
        BiTree stockNode = new BiTNode;
        stockNode->data = L.elem[i];
        stockNode->lchild = nullptr;
        stockNode->rchild = nullptr;
        stockNode->disition = "";

        // 根据条件判断插入位置
        if (L.elem[i].seccate == "计算机、通信和其他电子设备制造业") {
            // 二级门类满足条件，进入左子树
            if (L.elem[i].detail_count > 0 && L.elem[i].detail[0].risefallamount > 0) {
                // 最近一天跌涨幅>0，进入b1的左子树（b3节点）
                if (L.elem[i].detail_count > 0 && L.elem[i].detail[0].volume > 10000000) {
                    // 最近一天成交量>1000万股，插入到b3的左子树
                    insertNodeToTree(BT->lchild->lchild->lchild, stockNode);
                } else {
                    // 最近一天成交量<=1000万股，插入到b3的右子树
                    insertNodeToTree(BT->lchild->lchild->rchild, stockNode);
                }
            } else {
                // 最近一天跌涨幅<=0，进入b1的右子树（b4节点）
                insertNodeToTree(BT->lchild->rchild, stockNode);
            }
        } else {
            // 二级门类不满足条件，进入右子树b5
            insertNodeToTree(BT->rchild, stockNode);
        }
    }
}

BiTree FindNodeByName(BiTree BT, string name)
{// 根据股票名递归找到对应TNode结点，若不存在则返回NULL
	if (BT == nullptr) return nullptr;

	// 检查当前节点是否是目标股票
	if (!BT->data.indusname.empty() && BT->data.indusname == name) {
		return BT;
	}

	// 递归在左右子树中查找
	BiTree leftResult = FindNodeByName(BT->lchild, name);
	if (leftResult != nullptr) return leftResult;

	BiTree rightResult = FindNodeByName(BT->rchild, name);
	return rightResult;
}

BiTree FindOriginalFather(BiTree BT, BiTree t)
{// 查找二叉树中一个节点在决策树结构中的父结点
	if (BT == nullptr || t == nullptr) return nullptr;

	// 如果当前节点的左孩子或右孩子是目标节点，返回当前节点
	if (BT->lchild == t || BT->rchild == t) {
		return BT;
	}

	// 递归在左右子树中查找
	BiTree leftResult = FindOriginalFather(BT->lchild, t);
	if (leftResult != nullptr) return leftResult;

	BiTree rightResult = FindOriginalFather(BT->rchild, t);
	return rightResult;
}

void FindClass(BiTree& BT, string name)
{// 根据股票名，输出其从底到根的类别信息
    BiTree stockNode = FindNodeByName(BT, name);
    if (stockNode == nullptr) {
        cout << "未找到股票: " << name << endl;
        return;
    }

    // 直接检查股票数据来确定条件
    stock s = stockNode->data;
    vector<string> results;
    
    // 检查二级门类
    if (s.seccate == "计算机、通信和其他电子设备制造业") {
        results.push_back("二级门类为计算机、通信和其他电子设备制造业");
    } else {
        results.push_back("二级门类不为计算机、通信和其他电子设备制造业");
        // 如果不是目标门类，直接输出并返回
        cout << results[0] << endl;
        return;
    }
    
    // 检查涨跌幅
    if (s.detail_count > 0) {
        if (s.detail[0].risefallamount > 0) {
            results.push_back("最近一天跌涨幅>0");
            
            // 只有在涨跌幅>0时才检查成交量
            if (s.detail[0].volume > 10000000) {
                results.push_back("最近一天成交量>1000万股");
            } else {
                results.push_back("最近一天成交量<=1000万股");
            }
        } else {
            results.push_back("最近一天跌涨幅<=0");
            // 当跌涨幅<=0时，不输出成交量条件
        }
    } else {
        results.push_back("无交易数据");
    }
    
    // 逆序输出
    for (int i = results.size() - 1; i >= 0; i--) {
        cout << results[i] << endl;
    }
}