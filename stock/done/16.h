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
#define MVNum 30000
using namespace std;

// 定义边结点结构体
typedef struct ArcNode {
    int adjvex;                     // 该边所指向顶点的位置
    int relationship;               // 表示边的类型，即关系的类型，对应为数组下标
    struct ArcNode* nextarc;        // 下一条边
} ArcNode;

// 定义顶点结点结构体
typedef struct VNode {
    int entity;                     // 表示顶点的类型，即实体的类型，对应为数组下标
    string info;                    // 表示顶点的内容，即实体的内容
    ArcNode* firstarc;              // 指向第一条依附该顶点的边的指针
} VNode, AdjList[MVNum];

// 定义图的结构体
typedef struct {
    AdjList vertices;               // 邻接表
    int vexnum, arcnum;             // 图的当前顶点数和边数
} ALGraph;

// 定义关系数组，用于表示边的类型（根据文档中的9种关系）
string Relationship[] = {"所属机构","上市于","具有行业编码","属于行业大类","属于行业二级类","位于省份","位于城市","具有注册地址","主营业务为"};

// 定义顶点数组，用于表示实体类型和内容（根据文档中的10种实体）
string Entity[] = {"股票简称","机构","上市交易所","行业编码","行业大类","行业二级类","省份","城市","地址","主营业务"};

int LocateVex(ALGraph& G, string str) {
    // 返回str在AdjList中的位置，查找顶点在邻接表中的位置
    for (int i = 0; i < G.vexnum; i++) {
        if (G.vertices[i].info == str)
            return i;// 找到返回索引
    }
    return -1;// 未找到返回-1
}

int LocateEntity(string str) {
    // 返回str在Entity数组中的位置，查找实体在Entity数组中的位置
    for (int i = 0; i < 10; i++) {
        if (Entity[i] == str)
            return i;
    }
    return -1;
}

int LocateRelationship(string str) {
    // 返回str在Relationship数组中的位置，查找关系在Relationship数组中的位置
    for (int i = 0; i < 9; i++) {
        if (Relationship[i] == str)
            return i;
    }
    return -1;
}

void InitALGraph(ALGraph& G) {
    G.vexnum = 0;      // 顶点数清零
    G.arcnum = 0;      // 边数清零
    for (int i = 0; i < MVNum; i++) {
        G.vertices[i].firstarc = NULL;  // 每个顶点的边链表初始化为空
        G.vertices[i].entity = -1;      // 实体类型标记为未设置
        G.vertices[i].info = "";         // 顶点内容清空
    }
}

void CreateAdjList(ALGraph& G, string filename) {
    // 从filename中按顺序读取实体存入邻接表
    ifstream in;
    in.open(filename.c_str());
    if (!in.is_open()) {
        cerr << "无法打开实体文件: " << filename << endl;
        return;
    }
    
    string ch, type;
    int i = 0;
    while (in >> ch >> type) {
        // 去除换行符和空白字符
        ch.erase(ch.find_last_not_of(" \n\r\t") + 1);
        type.erase(type.find_last_not_of(" \n\r\t") + 1);
        
        G.vertices[i].info = ch;            // 存储顶点内容
        G.vertices[i].entity = LocateEntity(type); // 存储实体类型编号
        i++;
    }
    G.vexnum = i;// 更新图的顶点总数
    in.close();
}

void CreateUDG(ALGraph& G, string filename) {
    // 从filename中按顺序三元组存入邻接表
    ifstream in;
    in.open(filename.c_str());
    if (!in.is_open()) {
        cerr << "无法打开关系文件: " << filename << endl;
        return;
    }
    
    string ch1, type, ch2;// 头实体、关系类型、尾实体
    int i = 0;
    while (in >> ch1 >> type >> ch2) {
        // 去除换行符和空白字符
        ch1.erase(ch1.find_last_not_of(" \n\r\t") + 1);
        type.erase(type.find_last_not_of(" \n\r\t") + 1);
        ch2.erase(ch2.find_last_not_of(" \n\r\t") + 1);
        
        int k = LocateRelationship(type); // 关系类型下标
        int p = LocateVex(G, ch1);         // 头实体位置
        int q = LocateVex(G, ch2);         // 尾实体位置
        
        if (p != -1 && q != -1 && k != -1) {
            // 正向边：头实体->尾实体
            ArcNode* p1 = new ArcNode;
            p1->adjvex = q;                // 该边指向顶点q
            p1->relationship = k;          // 确定边的关系类型
            p1->nextarc = G.vertices[p].firstarc;
            G.vertices[p].firstarc = p1;   // 头插法插入到对应边表
            
            // 反向边：尾实体->头实体
            ArcNode* p2 = new ArcNode;
            p2->adjvex = p;                // 该边指向顶点p
            p2->relationship = k;          // 确定边的关系类型
            p2->nextarc = G.vertices[q].firstarc;
            G.vertices[q].firstarc = p2;   // 头插法插入到对应边表
            
            i++;
        }
    }
    G.arcnum = i * 2; // 每条三元组添加两条边
    in.close();
}

void PrintGraph(ALGraph& G) {
    // 按格式输出知识图谱
    for (int i = 0; i < G.vexnum; i++) {
        // 遍历与该顶点相关联的所有边
        for (ArcNode* p = G.vertices[i].firstarc; p != NULL; p = p->nextarc) {
            // 打印顶点信息、关系类型索引和指向的顶点信息
            cout << G.vertices[i].info << " "
                 << p->relationship << " "
                 << G.vertices[p->adjvex].info << endl;
        }
    }
}
