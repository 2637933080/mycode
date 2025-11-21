#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace std;

// 定义最大数组大小
const int MAX_STOCK_DATA = 1000;
const int MAX_LABELS = MAX_STOCK_DATA - 1;
const int MAX_INDICATOR_SIZE = MAX_LABELS;
const int MAX_INDICATORS = 5;

typedef struct StockDetail {
    string date;          // 日期
    double openPrice;     // 开盘价
    double closePrice;    // 收盘价
    double highest;       // 最高价
    double lowest;        // 最低价
    double volume;        // 成交量
    double turnover;      // 成交额
    string changeRate;    // 换手率
    string riseFallRate;  // 涨跌幅
    double riseFallAmount;// 涨跌额
} StockDetail;

struct IndicatorScore {
    string name;       // 指标名称
    double corrScore;  // 皮尔逊相关系数（绝对值）
};

bool LoadStockFromFile(const string& filename, StockDetail detailsList[], int& count) {
    // 从文件加载股票数据，返回是否成功
    ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    string line;
    count = 0;
    
    while (getline(file, line) && count < MAX_STOCK_DATA) {
        if (line.empty()) continue;
        
        stringstream ss(line);
        string token;
        vector<string> tokens;
        
        while (getline(ss, token, ';')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 9) {
            detailsList[count].date = tokens[0];
            detailsList[count].openPrice = stod(tokens[1]);
            detailsList[count].closePrice = stod(tokens[2]);
            detailsList[count].highest = stod(tokens[3]);
            detailsList[count].lowest = stod(tokens[4]);
            detailsList[count].volume = stod(tokens[5]);
            detailsList[count].turnover = stod(tokens[6]);
            detailsList[count].changeRate = tokens[7];
            detailsList[count].riseFallRate = tokens[8];
            
            count++;
        }
    }
    
    file.close();
    return count > 1;
}

void CreatePriceLabels(const StockDetail detailsList[], int count, int labels[], int& labelCount) {
    // 创建价格变动标签（1上涨，0下跌）
    labelCount = 0;
    
    for (int i = count - 1; i > 0; i--) {
        double todayClose = detailsList[i].closePrice;
        double nextDayClose = detailsList[i-1].closePrice;
        
        if (nextDayClose > todayClose) {
            labels[labelCount] = 1;
        } else {
            labels[labelCount] = 0;
        }
        labelCount++;
        
        if (labelCount >= MAX_LABELS) break;
    }
}

double ComputeCorrelationScore(const double indicator[], const int labels[], int n, 
                              double& sumX, double& sumY, double& sumXY) {
   // 计算指标与标签的皮尔逊相关系数绝对值
    if (n <= 1) return 0.0;
    
    sumX = 0.0, sumY = 0.0, sumXY = 0.0;
    double sumX2 = 0.0, sumY2 = 0.0;
    
    for (int i = 0; i < n; i++) {
        sumX += indicator[i];
        sumY += labels[i];
        sumXY += indicator[i] * labels[i];
        sumX2 += indicator[i] * indicator[i];
        sumY2 += labels[i] * labels[i];
    }
    
    double numerator = n * sumXY - sumX * sumY;
    double denominator = sqrt((n * sumX2 - sumX * sumX) * (n * sumY2 - sumY * sumY));
    
    if (denominator == 0) return 0.0;
    
    double correlation = numerator / denominator;
    return fabs(correlation);
}

void GetRiseFallRates(const StockDetail detailsList[], int count, double indicator[], int& indicatorCount) {
    // 获取涨跌幅指标数据（%）
    indicatorCount = 0;
    
    for (int i = count - 1; i > 0; i--) {
        string rateStr = detailsList[i].riseFallRate;
        if (!rateStr.empty() && rateStr.back() == '%') {
            rateStr.pop_back();
        }
        try {
            indicator[indicatorCount] = stod(rateStr);
        } catch (...) {
            indicator[indicatorCount] = 0.0;
        }
        indicatorCount++;
        
        if (indicatorCount >= MAX_INDICATOR_SIZE) break;
    }
}

void GetVolumesInWanShou(const StockDetail detailsList[], int count, double indicator[], int& indicatorCount) {
    // 获取成交量指标数据（转换为万手）
    indicatorCount = 0;
    
    for (int i = count - 1; i > 0; i--) {
        indicator[indicatorCount] = detailsList[i].volume / 10000.0;
        indicatorCount++;
        
        if (indicatorCount >= MAX_INDICATOR_SIZE) break;
    }
}

void GetAmplitudes(const StockDetail detailsList[], int count, double indicator[], int& indicatorCount) {
    // 计算振幅指标数据（%）
    indicatorCount = 0;
    
    for (int i = count - 1; i > 0; i--) {
        double high = detailsList[i].highest;
        double low = detailsList[i].lowest;
        double open = detailsList[i].openPrice;
        
        if (open > 0) {
            indicator[indicatorCount] = ((high - low) / open) * 100.0;
        } else {
            indicator[indicatorCount] = 0.0;
        }
        indicatorCount++;
        
        if (indicatorCount >= MAX_INDICATOR_SIZE) break;
    }
}

void GetTurnoverRates(const StockDetail detailsList[], int count, double indicator[], int& indicatorCount) {
    // 获取换手率指标数据（%）
    indicatorCount = 0;
    
    for (int i = count - 1; i > 0; i--) {
        string rateStr = detailsList[i].changeRate;
        if (!rateStr.empty() && rateStr.back() == '%') {
            rateStr.pop_back();
        }
        try {
            indicator[indicatorCount] = stod(rateStr);
        } catch (...) {
            indicator[indicatorCount] = 0.0;
        }
        indicatorCount++;
        
        if (indicatorCount >= MAX_INDICATOR_SIZE) break;
    }
}

void GetTurnoverInQianWan(const StockDetail detailsList[], int count, double indicator[], int& indicatorCount) {
    indicatorCount = 0;
    // 获取成交额指标数据（转换为千万元）
    for (int i = count - 1; i > 0; i--) {
        indicator[indicatorCount] = detailsList[i].turnover / 10000000.0;
        indicatorCount++;
        
        if (indicatorCount >= MAX_INDICATOR_SIZE) break;
    }
}

void SortIndicatorScores(IndicatorScore scores[], int count, bool descending) {// 对指标相关系数进行排序（降序/升序）
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            bool shouldSwap = descending ? 
                (scores[j].corrScore < scores[j+1].corrScore) : 
                (scores[j].corrScore > scores[j+1].corrScore);
            
            if (shouldSwap) {
                IndicatorScore temp = scores[j];
                scores[j] = scores[j+1];
                scores[j+1] = temp;
            }
        }
    }
}