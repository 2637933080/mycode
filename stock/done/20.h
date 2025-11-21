#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <iomanip>
#include <cstring>

using namespace std;

#define K_NEIGHBORS 3
#define MAX_DAYS 1000

struct TradingDay {
    string date;
    double change;
    double amplitude;
    double amountMil;
    double nextChange;
};

struct NeighborNode {
    double distance;
    double nextChange;
    NeighborNode* next;

    NeighborNode(double d = 0, double nc = 0) : distance(d), nextChange(nc), next(nullptr) {}

    static NeighborNode* createSentinelList() {
        return new NeighborNode();
    }

    void insertOrdered(NeighborNode* dummy, double dist, double nc) {
        NeighborNode* node = new NeighborNode(dist, nc);
        NeighborNode* p = dummy;
        while (p->next && p->next->distance < dist) p = p->next;
        node->next = p->next;
        p->next = node;
    }

    void trimList(NeighborNode* dummy) {
        NeighborNode* p = dummy;
        int count = 0;
        while (p->next) {
            count++;
            if (count > K_NEIGHBORS) {
                NeighborNode* del = p->next;
                p->next = nullptr;
                while (del) {
                    NeighborNode* tmp = del->next;
                    delete del;
                    del = tmp;
                }
                return;
            }
            p = p->next;
        }
    }

    double calculateAverage(NeighborNode* dummy) {
        NeighborNode* p = dummy->next;
        if (!p) return 0.0;
        double sum = 0.0;
        int cnt = 0;
        while (p) {
            sum += p->nextChange;
            cnt++;
            p = p->next;
        }
        return cnt ? (sum / cnt) : 0.0;
    }

    void freeList(NeighborNode* dummy) {
        NeighborNode* p = dummy;
        while (p) {
            NeighborNode* tmp = p->next;
            delete p;
            p = tmp;
        }
    }
};

void FindFeatureMinMax(const double features[][3], int featureCount, double minVal[], double maxVal[]) {
    for (int j = 0; j < 3; j++) {
        minVal[j] = 1e300;
        maxVal[j] = -1e300;
    }
    for (int i = 0; i < featureCount; i++) {
        for (int j = 0; j < 3; j++) {
            if (features[i][j] < minVal[j]) minVal[j] = features[i][j];
            if (features[i][j] > maxVal[j]) maxVal[j] = features[i][j];
        }
    }
}

void NormalizeFeatures(double features[][3], int featureCount) {
    double minVal[3], maxVal[3];
    FindFeatureMinMax(features, featureCount, minVal, maxVal);
    for (int i = 0; i < featureCount; i++) {
        for (int j = 0; j < 3; j++) {
            if (fabs(maxVal[j] - minVal[j]) < 1e-12)
                features[i][j] = 0.0;
            else
                features[i][j] = (features[i][j] - minVal[j]) / (maxVal[j] - minVal[j]);
        }
    }
}

double EuclideanDistance(const double a[], const double b[]) {
    double sum = 0.0;
    for (int i = 0; i < 3; ++i) sum += (a[i] - b[i]) * (a[i] - b[i]);
    return sqrt(sum);
}

bool ParseTradingLine(const string& line, int lineNum, TradingDay& day, int& label) {
    stringstream ss(line);
    string token;
    string arr[6];
    int idx = 0;
    while (getline(ss, token, ';')) {
        if (idx < 6) arr[idx++] = token;
    }
    if (idx != 6) return false;

    day.date = arr[0];
    // 注意：文件第3列是“当日涨跌幅”，第4列振幅，第5列成交额，第6列是次日涨跌标签
    day.change = atof(arr[2].c_str());
    day.amplitude = atof(arr[3].c_str());
    day.amountMil = atof(arr[4].c_str());
    day.nextChange = 0.0; // 先占位，实际数值将由 SetNextChanges 依据相邻行设置
    label = atoi(arr[5].c_str());
    return true;
}

void SetNextChanges(TradingDay days[], const int labels[], int dayCount) {
    // 数据按“最新在前”的顺序存储：
    // 对于索引 i (i >= 1)，days[i] 的次日涨跌数值等于 days[i-1] 的当日涨跌（days[i-1].change）。
    // days[0] 是最新行，其次日数值未知（要预测）。
    for (int i = 0; i < dayCount; ++i) {
        if (i >= 1) {
            days[i].nextChange = days[i - 1].change;
        } else {
            days[i].nextChange = 0.0; // 目标样本，不参与训练（或视为未知）
        }
    }
}

bool LoadDataFromFile(const string& filename, TradingDay days[], int& dayCount, int labels[]) {
    ifstream fin(filename.c_str());
    if (!fin.is_open()) return false;

    string line;
    if (!getline(fin, line)) return false; // 先读表头（可能只有一行表头）

    dayCount = 0;
    while (getline(fin, line)) {
        if (line.size() < 1) continue;
        if (!ParseTradingLine(line, dayCount + 2, days[dayCount], labels[dayCount]))
            return false;
        dayCount++;
        if (dayCount >= MAX_DAYS) break;
    }

    // 读取完所有行后，设置每条记录的 nextChange（训练目标）
    SetNextChanges(days, labels, dayCount);
    return true;
}

void ExtractTrainData(const TradingDay days[], int dayCount, double features[][3], double nextChanges[], int& featureCount) {
    featureCount = 0;
    // days[0] 是目标样本（最新），不作为训练样本。训练集从 i=1 开始。
    for (int i = 1; i < dayCount; ++i) {
        features[featureCount][0] = days[i].change;
        features[featureCount][1] = days[i].amplitude;
        features[featureCount][2] = days[i].amountMil;
        nextChanges[featureCount] = days[i].nextChange;
        featureCount++;
        if (featureCount >= MAX_DAYS) break;
    }
}

void NormalizeTargetFeature(const TradingDay days[], double targetFeatures[]) {
    // 保留为简单拷贝；在 PredictNextChange 中会用训练集的 min/max 对其归一化
    targetFeatures[0] = days[0].change;
    targetFeatures[1] = days[0].amplitude;
    targetFeatures[2] = days[0].amountMil;
}

double PredictNextChange(const TradingDay days[], int dayCount) {
    double features[MAX_DAYS][3];
    double nextChanges[MAX_DAYS];
    int featureCount = 0;

    ExtractTrainData(days, dayCount, features, nextChanges, featureCount);
    if (featureCount == 0) return 0.0;

    // 先计算训练集的 min/max（用于对训练集和目标都做同样的归一化）
    double minVal[3], maxVal[3];
    FindFeatureMinMax(features, featureCount, minVal, maxVal);

    // 对训练特征进行归一化（使用 FindFeatureMinMax 内计算得到的 min/max）
    for (int i = 0; i < featureCount; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (fabs(maxVal[j] - minVal[j]) < 1e-12)
                features[i][j] = 0.0;
            else
                features[i][j] = (features[i][j] - minVal[j]) / (maxVal[j] - minVal[j]);
        }
    }

    // 准备并归一化目标特征（使用相同的 min/max）
    double targetRaw[3];
    NormalizeTargetFeature(days, targetRaw);
    double target[3];
    for (int j = 0; j < 3; ++j) {
        if (fabs(maxVal[j] - minVal[j]) < 1e-12)
            target[j] = 0.0;
        else
            target[j] = (targetRaw[j] - minVal[j]) / (maxVal[j] - minVal[j]);
    }

    // KNN：按距离维护最小的 K_NEIGHBORS 个邻居（使用链表）
    NeighborNode* dummy = NeighborNode::createSentinelList();
    for (int i = 0; i < featureCount; ++i) {
        double dist = EuclideanDistance(features[i], target);
        dummy->insertOrdered(dummy, dist, nextChanges[i]);
        dummy->trimList(dummy);
    }

    double result = dummy->calculateAverage(dummy);
    dummy->freeList(dummy);
    return result;
}