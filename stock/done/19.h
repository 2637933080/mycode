#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace std;

// 定义最大数组大小
const int MAX_SAMPLES = 1000;
const int MAX_FEATURES = 3;
const int MAX_NODE_COUNT = 5;

struct Sample {
    string currentDate;
    double features[MAX_FEATURES];
    int label;
    int featureCount;
};

// 节点类型：特征节点 / 标签节点
enum NodeType { FEATURE, LABEL };

// 边的参数结构体（存储高斯分布的均值和方差）
struct EdgeParams {
    double mean = 0.0;       // 均值（初始化为0）
    double variance = 1e-6;  // 方差（默认最小阈值，避免除以0）
    bool exists = false;     // 标记边是否存在
};

// 贝叶斯网络图结构体
struct BayesianGraph {
    string nodeNames[MAX_NODE_COUNT] = {"涨跌幅", "振幅", "成交额", "跌", "涨"};
    NodeType nodeTypes[MAX_NODE_COUNT] = {FEATURE, FEATURE, FEATURE, LABEL, LABEL};
    int nodeCount = MAX_NODE_COUNT;
     // 邻接矩阵：存储特征节点到标签节点的高斯分布参数
    EdgeParams adj[MAX_NODE_COUNT][MAX_NODE_COUNT];
    // 先验概率：P(跌)和P(涨)
    double prior[2] = {0.0, 0.0};

    // 构造函数：初始化邻接矩阵（网络拓扑结构）（特征→标签的连接）
    BayesianGraph() {
        for (int i = 0; i < 3; ++i) {        // 遍历所有特征节点
            for (int j = 3; j < 5; ++j) {     // 连接到标签节点（索引3和4）
                adj[i][j].exists = true;      // 建立特征到标签的有向边
            }
        }
    }

    int FindNodeIndex(NodeType type, int index = -1) const {
        // 查找节点索引
        if (type == FEATURE) {
            if (index >= 0 && index < 3) {
                return index;
            }
            return -1;
        } else { // LABEL
            if (index == 0) return 3; // 跌
            if (index == 1) return 4; // 涨
            return -1;
        }
    }

    void ComputePriorProbabilities(const Sample samples[], int sampleCount) {
        // 计算先验概率
        int count0 = 0, count1 = 0; // 统计跌和涨的样本数量
        // 遍历所有样本，统计标签分布
        for (int i = 0; i < sampleCount; ++i) {
            if (samples[i].label == 0) count0++;
            else if (samples[i].label == 1) count1++;
        }
        // 计算先验概率：P(跌) = count0/total, P(涨) = count1/total
        prior[0] = (double)count0 / sampleCount;
        prior[1] = (double)count1 / sampleCount;
        
        // 避免概率为0
        if (prior[0] == 0) prior[0] = 1e-6;
        if (prior[1] == 0) prior[1] = 1e-6;
    }

    void ComputeEdgeMeans(const Sample samples[], int sampleCount) {
        // 计算边的均值
        // 对于每个特征节点到标签节点的边，计算条件均值
        for (int featureIdx = 0; featureIdx < 3; ++featureIdx) {
            for (int labelIdx = 0; labelIdx < 2; ++labelIdx) {
                double sum = 0.0;
                int count = 0;
                
                // 计算给定标签下该特征的均值
                for (int i = 0; i < sampleCount; ++i) {
                    if (samples[i].label == labelIdx) {
                        sum += samples[i].features[featureIdx];
                        count++;
                    }
                }
                
                if (count > 0) {
                int nodeFrom = featureIdx;
                int nodeTo = (labelIdx == 0) ? 3 : 4;  // 3=跌节点，4=涨节点
                adj[nodeFrom][nodeTo].mean = sum / count;  // 存储条件均值
            }
        }
    }
}


    void ComputeEdgeVariances(const Sample samples[], int sampleCount) {
        // 计算边的方差 σ²|label
        for (int featureIdx = 0; featureIdx < 3; ++featureIdx) {
            for (int labelIdx = 0; labelIdx < 2; ++labelIdx) {
                double mean = 0.0;
                double sumSq = 0.0;
                int count = 0;
                
                // 先重新计算均值（确保一致性）
                for (int i = 0; i < sampleCount; ++i) {
                    if (samples[i].label == labelIdx) {
                        mean += samples[i].features[featureIdx];
                        count++;
                    }
                }
                if (count > 0) mean /= count;
                
                // 计算方差：Σ(x - μ)²
                for (int i = 0; i < sampleCount; ++i) {
                    if (samples[i].label == labelIdx) {
                        double diff = samples[i].features[featureIdx] - mean;
                        sumSq += diff * diff;
                    }
                }
                
                if (count > 1) {
                    int nodeFrom = featureIdx;
                    int nodeTo = (labelIdx == 0) ? 3 : 4;
                    // 无偏估计：除以(n-1)
                    adj[nodeFrom][nodeTo].variance = sumSq / (count - 1);
                }
                
                // 避免方差为0导致的除零错误，确保数值稳定性
                if (adj[featureIdx][(labelIdx == 0) ? 3 : 4].variance < 1e-6) {
                    adj[featureIdx][(labelIdx == 0) ? 3 : 4].variance = 1e-6;
                }
            }
        }
    }

    void Train(const Sample samples[], int sampleCount) {
        // 训练网络
        ComputePriorProbabilities(samples, sampleCount);
        ComputeEdgeMeans(samples, sampleCount);
        ComputeEdgeVariances(samples, sampleCount);
    }

    double CalculateLogProbability(const double features[], int label) const {
        // 计算对数概率
        // 1. 先验概率项：log(P(label))
    double logProb = log(label == 0 ? prior[0] : prior[1]);
    
    // 2. 似然项：Σlog(P(feature_i|label))使用对数避免数值下溢
    for (int i = 0; i < 3; ++i) {
        int nodeTo = (label == 0) ? 3 : 4;
        double mean = adj[i][nodeTo].mean;       // 条件均值 μ
        double variance = adj[i][nodeTo].variance; // 条件方差 σ²
        double x = features[i];                  // 当前特征值
        
        // 高斯概率密度函数的对数形式：
        // log(N(x|μ,σ²)) = -0.5*log(2πσ²) - (x-μ)²/(2σ²)
        double exponent = -(x - mean) * (x - mean) / (2 * variance);
        double normalizer = -0.5 * log(2 * M_PI * variance);
        logProb += normalizer + exponent;  // 累加对数似然
    }
    
    return logProb;
}

        
    int Predict(const double features[]) const {
    // 分别计算属于"跌"和"涨"类的对数后验概率
    double logProb0 = CalculateLogProbability(features, 0);  // log(P(跌|features))
    double logProb1 = CalculateLogProbability(features, 1);  // log(P(涨|features))
    
    // 选择概率较大的类别（对数域比较等价于原始概率比较）
    return (logProb1 > logProb0) ? 1 : 0;
}
};

Sample ParseSampleLine(const string& line) {
    // 解析样本数据行
    Sample sample;
    stringstream ss(line);
    string token;
    
    // 解析格式：当前日期;次日日期;涨跌幅;振幅;成交额;次日涨跌
    if (getline(ss, token, ';')) sample.currentDate = token;
    if (getline(ss, token, ';')); // 跳过次日日期
    if (getline(ss, token, ';')) sample.features[0] = stod(token); // 涨跌幅
    if (getline(ss, token, ';')) sample.features[1] = stod(token); // 振幅
    if (getline(ss, token, ';')) sample.features[2] = stod(token); // 成交额
    if (getline(ss, token, ';')) {
        if (token == "-1") {
            sample.label = -1; // 最新数据无标签
        } else {
            sample.label = stoi(token);// 解析标签（0或1）
        }
    }
    
    sample.featureCount = 3;
    return sample;
}

int LoadTrainData(const string& filename, Sample samples[]) {
    // 加载训练数据
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("无法打开文件: " + filename);
    }
    
    string line;
    int count = 0;
    
    getline(file, line); // 跳过CSV表头
    
    // 逐行读取并解析训练数据
    while (getline(file, line) && count < MAX_SAMPLES) {
        if (line.empty()) continue;
        
        line.erase(line.find_last_not_of("\r\n") + 1); // 清理行尾
        
        try {
            samples[count] = ParseSampleLine(line); // 解析样本
            count++;
        } catch (...) {
            continue; // 容错处理：跳过解析失败的行
        }
    }
    
    file.close();
    return count; // 返回成功加载的样本数量
}
