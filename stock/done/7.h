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
// 开放地址法散列表的存储表示
    stock* key;
    int length;
} HashTable;

void InitHT(HashTable& HT) 
{// 散列表初始化
    HT.key = new stock[m];
    for (int i = 0; i < m; i++) {
        HT.key[i].sname = "";  // 用空字符串表示空位置
        HT.key[i].detail = nullptr;
        HT.key[i].detail_count = 0;
    }
    HT.length = 0;
}

int Hash(string sname) {
// 实现散列函数：字符串sname中各字符的下标（从0开始）的平方乘以字符对应的ASCII码值，相加后与299取余
    int sum = 0;
    for (int i = 0; i < sname.length(); i++)
        sum += ((i) * (i) * int(sname[i]));
    return sum % 397;
}

void HTInsert(HashTable& HT, stock s, int& sumCmp) 
{// 往散列表中插入新的股票s
// 在插入的过程中统计总的比较次数sumCmp
    int addr = Hash(s.sname);
    int i = 0;
    
    while (i < m) {
        int index = (addr + i) % m;
        sumCmp++;  // 比较次数加1
        
        if (HT.key[index].sname == "" || HT.key[index].sname == "DELETED") {
            // 找到空位置或已删除位置，插入股票
            HT.key[index] = s;
            HT.length++;
            return;
        }
        i++;
    }
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

void ReadFile(HashTable& HT, int& sumCmp, string infoFilename, string detailDir) {
// 从文件中读取中股票信息，将其按顺序存入散列表HT中
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
        HTInsert(HT, temp, sumCmp);
    }
    infile.close();
}

void Print(HashTable HT, int pos) 
{// 输出股票信息，double类型变量保留3位小数，详见输出样例
    if (pos < 0 || pos >= m || HT.key[pos].sname == "") {
        return;
    }
    
    stock s = HT.key[pos];
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

int SearchHash(HashTable HT, string key) 
{// 使用线性探测法，在散列表HT中查找股票英文名称等于key的元素
// 若找到，则返回散列表的单元标号，否则返回-1
    int addr = Hash(key);
    int i = 0;
    
    while (i < m) {
        int index = (addr + i) % m;
        
        if (HT.key[index].sname == "") {
            return -1;  // 遇到空位置，查找失败
        }
        
        if (HT.key[index].sname == key) {
            return index;  // 找到目标
        }
        
        i++;
    }
    
    return -1;  // 遍历完整个表，查找失败
}

double GetASL(HashTable HT, int sumCmp) 
{// 返回基于开放地址法的散列查找的ASL
    if (HT.length == 0) return 0.0;
    return (double)sumCmp / HT.length;
}