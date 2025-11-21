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
#include <map>
#include <set>
#define MVNum 30000
using namespace std;

// 定义关系数组，用于表示边的类型
string Relationship[] = {"所属机构","上市于","具有行业编码","属于行业大类","属于行业二级类","位于省份","位于城市","具有注册地址","主营业务为"};

// 定义顶点数组，用于表示实体类型和内容
string Entity[]= {"股票简称","机构","上市交易所","行业编码","行业大类","行业二级类","省份","城市","地址","主营业务"};

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

map<string, int> entityMap;  // 实体内容 -> 顶点索引
map<string, int> relationMap; // 关系内容 -> 关系类型编号

int LocateEntity(string str) {
    // 返回str在Entity数组中的位置
    for (int i = 0; i < 10; i++) {
        if (Entity[i] == str) {
            return i;
        }
    }
    return -1;
}

int LocateRelationship(string str) {
    // 返回str在Relationship数组中的位置
    for (int i = 0; i < 9; i++) {
        if (Relationship[i] == str) {
            return i;
        }
    }
    return -1;
}

void InitALGraph(ALGraph& G) {
    // InitALGraph函数，初始化邻接表 初始化顶点数和边数为0
    G.vexnum = 0;
    G.arcnum = 0;
    for (int i = 0; i < MVNum; i++) {
        G.vertices[i].entity = -1;
        G.vertices[i].info = "";
        G.vertices[i].firstarc = nullptr;
    }
    
    // 初始化关系映射
    for (int i = 0; i < 9; i++) {
        relationMap[Relationship[i]] = i;
    }
}

void CreateAdjList(ALGraph& G, string filename) {
    // 从filename中按顺序读取实体存入邻接表
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }
    
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        // 去掉可能的换行符
        line.erase(line.find_last_not_of("\n\r\t") + 1);
        
        // 实体文件格式：实体内容 实体类型
        size_t pos = line.find_last_of(' ');
        if (pos == string::npos) continue;
        
        string content = line.substr(0, pos);
        string type = line.substr(pos + 1);
        
        // 去掉前后空格
        content.erase(0, content.find_first_not_of(" \t"));
        content.erase(content.find_last_not_of(" \t") + 1);
        type.erase(0, type.find_first_not_of(" \t"));
        type.erase(type.find_last_not_of(" \t") + 1);
        
        int entityType = LocateEntity(type);
        if (entityType != -1) {
            // 检查是否已存在该实体
            if (entityMap.find(content) == entityMap.end()) {
                G.vertices[G.vexnum].entity = entityType;
                G.vertices[G.vexnum].info = content;
                G.vertices[G.vexnum].firstarc = nullptr;
                entityMap[content] = G.vexnum;
                G.vexnum++;
            }
        }
    }
    file.close();
}

int LocateVex(ALGraph& G, string str) {
    // 返回str在AdjList中的位置
    if (entityMap.find(str) != entityMap.end()) {
        return entityMap[str];
    }
    return -1;
}

void CreateUDG(ALGraph& G, string filename) {
    // 从filename中按顺序三元组存入邻接表
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }
    
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        // 去掉可能的换行符
        line.erase(line.find_last_not_of("\n\r\t") + 1);
        
        // 解析三元组格式：头实体 关系 尾实体
        size_t pos1 = line.find(' ');
        if (pos1 == string::npos) continue;
        
        size_t pos2 = line.find(' ', pos1 + 1);
        if (pos2 == string::npos) continue;
        
        string head = line.substr(0, pos1);
        string relation = line.substr(pos1 + 1, pos2 - pos1 - 1);
        string tail = line.substr(pos2 + 1);
        
        // 去掉前后空格
        head.erase(0, head.find_first_not_of(" \t"));
        head.erase(head.find_last_not_of(" \t") + 1);
        relation.erase(0, relation.find_first_not_of(" \t"));
        relation.erase(relation.find_last_not_of(" \t") + 1);
        tail.erase(0, tail.find_first_not_of(" \t"));
        tail.erase(tail.find_last_not_of(" \t") + 1);
        
        int headIndex = LocateVex(G, head);
        int tailIndex = LocateVex(G, tail);
        int relationType = -1;
        
        if (relationMap.find(relation) != relationMap.end()) {
            relationType = relationMap[relation];
        }
        
        if (headIndex != -1 && tailIndex != -1 && relationType != -1) {
            // 正向边：头实体 -> 尾实体
            ArcNode* newArc1 = new ArcNode();
            newArc1->adjvex = tailIndex;
            newArc1->relationship = relationType;
            newArc1->nextarc = G.vertices[headIndex].firstarc;
            G.vertices[headIndex].firstarc = newArc1;
            
            // 反向边：尾实体 -> 头实体
            ArcNode* newArc2 = new ArcNode();
            newArc2->adjvex = headIndex;
            newArc2->relationship = relationType;
            newArc2->nextarc = G.vertices[tailIndex].firstarc;
            G.vertices[tailIndex].firstarc = newArc2;
            
            G.arcnum += 2;
        }
    }
    file.close();
}

void SearchByEntity(ALGraph &G, const string &entityName) {
    // 使用知识图谱进行股票推荐
    // 查找包含输入信息的所有顶点
    vector<int> matchedVertices;
    for (int i = 0; i < G.vexnum; i++) {
        if (G.vertices[i].info.find(entityName) != string::npos) {
            matchedVertices.push_back(i);
        }
    }
    
    // 对于每个匹配的顶点，找到相关的股票简称
    set<int> stockVertices; // 使用set去重
    for (int vertexIndex : matchedVertices) {
        // 从当前顶点出发，找到所有连接的股票简称顶点
        ArcNode* p = G.vertices[vertexIndex].firstarc;
        while (p != nullptr) {
            int adjVertexIndex = p->adjvex;
            // 如果相邻顶点是股票简称，则加入结果集
            if (G.vertices[adjVertexIndex].entity == 0) {
                stockVertices.insert(adjVertexIndex);
            }
            p = p->nextarc;
        }
    }
    
    // 根据图片修正输出顺序：主营业务(8)、注册地址(7)、城市(6)、省份(5)、行业二级类(4)、行业大类(3)、行业编码(2)、上市交易所(1)、所属机构(0)
    int outputOrder[] = {8, 7, 6, 5, 4, 3, 2, 1, 0};
    
    // 对于每个股票简称，按照指定顺序输出其属性
    for (int stockIndex : stockVertices) {
        string stockName = G.vertices[stockIndex].info;
        
        // 按照指定的顺序输出属性
        for (int i = 0; i < 9; i++) {
            int targetRelType = outputOrder[i];
            ArcNode* p = G.vertices[stockIndex].firstarc;
            
            while (p != nullptr) {
                if (p->relationship == targetRelType) {
                    string relationName;
                    string targetInfo = G.vertices[p->adjvex].info;
                    
                    // 根据图片修正关系名称映射
                    switch (targetRelType) {
                        case 8: relationName = "主营业务为"; break;
                        case 7: relationName = "具有注册地址"; break;
                        case 6: relationName = "位于城市"; break;
                        case 5: relationName = "位于省份"; break;
                        case 4: relationName = "属于行业二级类"; break;
                        case 3: relationName = "属于行业大类"; break;
                        case 2: relationName = "具有行业编码"; break;
                        case 1: relationName = "上市于"; break;
                        case 0: relationName = "所属机构"; break;
                    }
                    
                    cout << stockName << "->" << relationName << "->" << targetInfo << endl;
                }
                p = p->nextarc;
            }
        }
    }
}