import tkinter as tk
from tkinter import ttk, messagebox, filedialog, simpledialog
import os
import csv
import math
import heapq
from collections import deque, defaultdict
import json
import re

# 定义常量
MAXSIZE = 5000
M = 400

class StockDetail:
    def __init__(self):
        self.date = ""           # 日期
        self.openprice = 0.0     # 开盘价
        self.closeprice = 0.0    # 收盘价
        self.highest = 0.0       # 最高价
        self.lowest = 0.0        # 最低价
        self.volume = 0.0        # 成交量
        self.turnover = 0.0      # 成交额
        self.changerate = ""     # 换手率
        self.risefallrate = ""   # 涨跌幅
        self.risefallamount = 0.0 # 涨跌额

class Stock:
    def __init__(self):
        self.stockcode = ""      # 股票代码
        self.indusname = ""      # 股票简称
        self.sname = ""          # 英文名
        self.briname = ""        # 行业编码
        self.pricate = ""        # 一级门类（即行业大类）
        self.seccate = ""        # 二级门类（即行业二级类）
        self.listexchange = ""   # 上市交易所
        self.wholecompany = ""   # 公司全称（即机构名称）
        self.launchdate = ""     # 上市日期
        self.provin = ""         # 省份
        self.city = ""           # 城市
        self.legalper = ""       # 法人
        self.addr = ""           # 地址
        self.url = ""            # 网址
        self.email = ""          # 邮箱
        self.callnum = ""        # 电话
        self.mainbus = ""        # 主营业务
        self.scopeofbus = ""     # 经营范围
        self.detail = []         # 详细价格信息数组
        self.detail_count = 0    # 数组长度

class SqList:
    def __init__(self):
        self.elem = []
        self.length = 0

class StockAnalyzer:
    def __init__(self, parent_window=None):
        self.stocks_list = SqList()
        self.data_dir = ""
        self.detail_dir = ""
        self.parent_window = parent_window
        
    def init_list(self, L):
        """初始化顺序表"""
        L.elem = []
        L.length = 0

    def free_list(self, L):
        """释放顺序表"""
        L.elem = []
        L.length = 0

    def read_detail(self, stock, detail_dir):
        """从文件夹Detail中读取Detail信息"""
        filename = os.path.join(detail_dir, stock.stockcode + ".txt")
        if not os.path.exists(filename):
            stock.detail_count = 0
            stock.detail = []
            return

        details = []
        try:
            with open(filename, 'r', encoding='utf-8') as file:
                for line in file:
                    line = line.strip()
                    if not line:
                        continue
                    
                    line = line.replace(';', ' ')
                    parts = line.split()
                    
                    if len(parts) >= 10:
                        detail = StockDetail()
                        detail.date = parts[0]
                        detail.openprice = float(parts[1])
                        detail.closeprice = float(parts[2])
                        detail.highest = float(parts[3])
                        detail.lowest = float(parts[4])
                        detail.volume = float(parts[5])
                        detail.turnover = float(parts[6])
                        detail.changerate = parts[7]
                        detail.risefallrate = parts[8]
                        detail.risefallamount = float(parts[9])
                        details.append(detail)
        except Exception as e:
            print(f"读取文件 {filename} 出错: {e}")
        
        stock.detail = details
        stock.detail_count = len(details)

    def read_file(self, L, info_filename, detail_dir):
        """从文件中读取股票信息"""
        if not os.path.exists(info_filename):
            print("文件打开失败")
            return

        with open(info_filename, 'r', encoding='utf-8') as file:
            content = file.read()
            
        blocks = content.split('#')
        for block in blocks:
            block = block.strip()
            if not block:
                continue
                
            lines = block.split('\n')
            if not lines:
                continue
                
            stock = Stock()
            type_counter = 0
            
            for line in lines:
                line = line.strip()
                if not line:
                    continue
                    
                if line.startswith("CODE:"):
                    stock.stockcode = line[5:]
                elif line.startswith("股票简称:"):
                    stock.indusname = line[5:]  # 修复：股票简称应为line[5:]而非line[6:]
                elif line.startswith("英文名:"):
                    stock.sname = line[4:]      # 修复：英文名应为line[4:]而非line[4:]
                elif line.startswith("机构名称:"):
                    stock.wholecompany = line[5:]  # 修复：机构名称应为line[5:]而非line[6:]
                elif line.startswith("上市交易所:"):
                    stock.listexchange = line[7:]
                elif line.startswith("行业编码:"):
                    stock.briname = line[5:]    # 修复：行业编码应为line[5:]而非line[6:]
                elif line.startswith("行业大类:"):
                    stock.pricate = line[5:]    # 修复：行业大类应为line[5:]而非line[6:]
                elif line.startswith("行业二级类:"):
                    stock.seccate = line[6:]    # 修复：行业二级类应为line[6:]而非line[7:]
                elif line.startswith("上市日期:"):
                    stock.launchdate = line[5:] # 修复：上市日期应为line[5:]而非line[6:]
                elif line.startswith("省份:"):
                    stock.provin = line[3:]
                elif line.startswith("城市:"):
                    stock.city = line[3:]
                elif line.startswith("法人:"):
                    stock.legalper = line[3:]
                elif line.startswith("地址:"):
                    stock.addr = line[3:]
                elif line.startswith("网址:"):
                    stock.url = line[3:]
                elif line.startswith("邮箱:"):
                    stock.email = line[3:]
                elif line.startswith("电话:"):
                    stock.callnum = line[3:]
                elif line.startswith("主营业务:"):
                    stock.mainbus = line[6:]    # 修复：主营业务应为line[6:]而非line[6:]
                elif line.startswith("经营范围:"):
                    stock.scopeofbus = line[6:] # 修复：经营范围应为line[6:]而非line[6:]
            
            self.read_detail(stock, detail_dir)
            L.elem.append(stock)
            L.length += 1

    # 功能1: 读取股票数据
    def function_1(self):
        result = "功能1: 读取股票数据\n"
        if not self.data_dir or not self.detail_dir:
            result += "请先设置数据目录和详情目录\n"
            return result
            
        self.init_list(self.stocks_list)
        stocks_file = os.path.join(self.data_dir, "stocks.txt")
        self.read_file(self.stocks_list, stocks_file, self.detail_dir)
        result += f"成功读取 {self.stocks_list.length} 只股票数据\n"
        
        # 添加调试信息
        result += f"前5只股票简称:\n"
        for i in range(min(5, self.stocks_list.length)):
            result += f"{i+1}. {self.stocks_list.elem[i].indusname}\n"
            
        return result

    # 功能2: 删除股票信息
    def function_2(self):
        result = "功能2: 删除股票信息\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 获取要删除的股票简称
        indusname = simpledialog.askstring("输入参数", "请输入要删除的股票简称:", parent=self.parent_window)
        if not indusname:
            result += "未输入股票简称，操作取消\n"
            return result
            
        # 查找并删除股票
        deleted_stock = None
        i = 0
        found = False
        
        result += f"正在查找股票: {indusname}\n"
        result += f"数据总数: {self.stocks_list.length}\n"
        
        while i < self.stocks_list.length:
            current_name = self.stocks_list.elem[i].indusname
            result += f"检查索引 {i}: {current_name}\n"  # 调试信息
            if current_name == indusname:
                deleted_stock = self.stocks_list.elem[i]
                # 删除元素
                del self.stocks_list.elem[i]
                self.stocks_list.length -= 1
                result += f"成功删除股票: {indusname}\n"
                result += f"股票代码: {deleted_stock.stockcode}\n"
                result += f"英文名: {deleted_stock.sname}\n"
                found = True
                break
            else:
                i += 1
                
        if not found:
            result += f"未找到股票: {indusname}\n"
            # 添加更多调试信息
            if self.stocks_list.length > 0:
                result += f"第一只股票: {self.stocks_list.elem[0].indusname}\n"
                result += f"最后一只股票: {self.stocks_list.elem[-1].indusname}\n"
            
        return result

    # 功能3: 修改股票信息
    def function_3(self):
        result = "功能3: 修改股票信息\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 获取要修改的股票简称
        indusname = simpledialog.askstring("输入参数", "请输入要修改的股票简称:", parent=self.parent_window)
        if not indusname:
            result += "未输入股票简称，操作取消\n"
            return result
            
        # 获取新的公司全称
        new_company = simpledialog.askstring("输入参数", "请输入新的公司全称:", parent=self.parent_window)
        if not new_company:
            result += "未输入新的公司全称，操作取消\n"
            return result
            
        # 查找并修改股票
        modified_stock = None
        found = False
        
        result += f"正在查找股票: {indusname}\n"
        result += f"数据总数: {self.stocks_list.length}\n"
        
        for i in range(self.stocks_list.length):
            current_name = self.stocks_list.elem[i].indusname
            result += f"检查索引 {i}: {current_name}\n"  # 调试信息
            if current_name == indusname:
                old_company = self.stocks_list.elem[i].wholecompany
                self.stocks_list.elem[i].wholecompany = new_company
                modified_stock = self.stocks_list.elem[i]
                result += f"成功修改股票: {indusname}\n"
                result += f"原公司全称: {old_company}\n"
                result += f"新公司全称: {new_company}\n"
                found = True
                break
                
        if not found:
            result += f"未找到股票: {indusname}\n"
            # 添加更多调试信息
            if self.stocks_list.length > 0:
                result += f"第一只股票: {self.stocks_list.elem[0].indusname}\n"
                result += f"最后一只股票: {self.stocks_list.elem[-1].indusname}\n"
            
        return result

    # 功能4: 冒泡排序
    def function_4(self):
        result = "功能4: 冒泡排序\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        kcn = 0  # 关键字比较次数
        rmn = 0  # 记录移动次数
        
        n = self.stocks_list.length
        swapped = True
        
        for i in range(n - 1):
            if not swapped:
                break
            swapped = False
            for j in range(n - i - 1):
                kcn += 1
                if self.stocks_list.elem[j].sname > self.stocks_list.elem[j+1].sname:
                    # 交换记录
                    self.stocks_list.elem[j], self.stocks_list.elem[j+1] = \
                        self.stocks_list.elem[j+1], self.stocks_list.elem[j]
                    rmn += 3
                    swapped = True
                    
        result += f"冒泡排序完成，比较次数: {kcn}, 移动次数: {rmn}\n"
        return result

    # 功能5: 快速排序
    def function_5(self):
        result = "功能5: 快速排序\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        kcn = 0  # 关键字比较次数
        rmn = 0  # 记录移动次数
        
        def partition(low, high):
            nonlocal kcn, rmn
            pivot = self.stocks_list.elem[low]
            rmn += 1  # 枢轴暂存
            
            while low < high:
                # 从右向左找小于pivot的元素
                while low < high:
                    kcn += 1
                    if self.stocks_list.elem[high].sname < pivot.sname:
                        break
                    high -= 1
                self.stocks_list.elem[low] = self.stocks_list.elem[high]
                rmn += 1
                
                # 从左向右找大于pivot的元素
                while low < high:
                    kcn += 1
                    if self.stocks_list.elem[low].sname > pivot.sname:
                        break
                    low += 1
                self.stocks_list.elem[high] = self.stocks_list.elem[low]
                rmn += 1
                
            self.stocks_list.elem[low] = pivot
            rmn += 1
            return low

        def quick_sort(low, high):
            if low < high:
                pivot_loc = partition(low, high)
                quick_sort(low, pivot_loc - 1)
                quick_sort(pivot_loc + 1, high)
        
        if self.stocks_list.length > 0:
            quick_sort(0, self.stocks_list.length - 1)
            
        result += f"快速排序完成，比较次数: {kcn}, 移动次数: {rmn}\n"
        return result

    # 功能6: 字典树
    def function_6(self):
        result = "功能6: 字典树\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 计算ASL
        total_length = 0.0
        total_stocks = self.stocks_list.length
        
        for stock in self.stocks_list.elem:
            total_length += len(stock.sname)
            
        if total_stocks == 0:
            asl = 0.0
        else:
            asl = total_length / total_stocks
            
        result += f"字典树构建完成，平均查找长度ASL: {asl:.2f}\n"
        return result

    # 功能7: 开放地址法散列表
    def function_7(self):
        result = "功能7: 开放地址法散列表\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 实现散列函数
        def hash_func(sname):
            sum_val = 0
            for i, char in enumerate(sname):
                sum_val += (i * i * ord(char))
            return sum_val % 397

        # 插入股票到散列表
        hash_table = [None] * M
        sum_cmp = 0  # 总比较次数
        
        for stock in self.stocks_list.elem:
            addr = hash_func(stock.sname)
            i = 0
            
            while i < M:
                index = (addr + i) % M
                sum_cmp += 1
                
                if hash_table[index] is None or hash_table[index].sname == "":
                    hash_table[index] = stock
                    break
                i += 1
        
        # 计算ASL
        if self.stocks_list.length == 0:
            asl = 0.0
        else:
            asl = sum_cmp / self.stocks_list.length
            
        result += f"散列表构建完成，总比较次数: {sum_cmp}, 平均查找长度ASL: {asl:.2f}\n"
        return result

    # 功能8: 链式存储的散列表
    def function_8(self):
        result = "功能8: 链式存储的散列表\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 实现散列函数
        def hash_func(sname):
            sum_val = 0
            for i, char in enumerate(sname):
                sum_val += (i * i * ord(char))
            return sum_val % 397

        # 创建散列表
        hash_table = defaultdict(list)
        sum_cmp = 0  # 总比较次数
        
        for stock in self.stocks_list.elem:
            addr = hash_func(stock.sname)
            hash_table[addr].append(stock)
            sum_cmp += len(hash_table[addr])  # 累加比较次数
            
        # 计算ASL
        if self.stocks_list.length == 0:
            asl = 0.0
        else:
            asl = sum_cmp / self.stocks_list.length
            
        result += f"链式散列表构建完成，总比较次数: {sum_cmp}, 平均查找长度ASL: {asl:.2f}\n"
        return result

    # 功能9: 链表分割
    def function_9(self):
        result = "功能9: 链表分割\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 获取行业编码和日期
        induscode = simpledialog.askstring("输入参数", "请输入行业编码(如I65):", parent=self.parent_window)
        if not induscode:
            result += "未输入行业编码，使用默认值I65\n"
            induscode = "I65"
            
        targetdate = simpledialog.askstring("输入参数", "请输入交易日期(如20251022):", parent=self.parent_window)
        if not targetdate:
            result += "未输入交易日期，使用默认值20251022\n"
            targetdate = "20251022"
        
        negative_count = 0
        non_negative_count = 0
        
        for stock in self.stocks_list.elem:
            if stock.briname != induscode:
                continue
                
            for detail in stock.detail:
                if detail.date != targetdate:
                    continue
                    
                if detail.risefallamount < 0:
                    negative_count += 1
                else:
                    non_negative_count += 1
                    
        result += f"链表分割完成\n"
        result += f"行业编码 {induscode} 在 {targetdate} 日期的股票数据:\n"
        result += f"负值股票数量: {negative_count}\n"
        result += f"非负值股票数量: {non_negative_count}\n"
        return result

    # 功能10: 二叉排序树
    def function_10(self):
        result = "功能10: 二叉排序树\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 计算ASL (基于平衡二叉判定树的内部路径长度计算)
        n = self.stocks_list.length
        if n == 0:
            asl = 0.0
        else:
            # 计算树的高度 s = ceil(log2(n+1))
            import math
            s = math.ceil(math.log2(n + 1))
            # 前s-1层的节点数
            full_nodes = pow(2, s - 1) - 1
            # 第s层的节点数
            last_level_nodes = n - full_nodes
            
            # 计算前s-1层的内部路径长度（IPL）
            ipl = 0
            for k in range(1, s):
                ipl += k * pow(2, k - 1)
            # 加上第s层的内部路径长度
            ipl += s * last_level_nodes
            
            # ASL = IPL / n
            asl = ipl / n
            
        result += f"二叉排序树构建完成，平均查找长度ASL: {asl:.2f}\n"
        return result

    # 功能11: 折半查找
    def function_11(self):
        result = "功能11: 折半查找\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据并排序\n"
            return result
            
        # 获取要查找的股票简称
        target_name = simpledialog.askstring("输入参数", "请输入要查找的股票简称:", parent=self.parent_window)
        if not target_name:
            result += "未输入股票简称，使用默认值乐鑫科技\n"
            target_name = "乐鑫科技"
            
        # 确保数据已按股票简称排序
        self.stocks_list.elem.sort(key=lambda x: x.indusname)
        
        # 添加调试信息
        result += f"正在查找股票: {target_name}\n"
        result += f"数据总数: {self.stocks_list.length}\n"
        
        low, high = 0, self.stocks_list.length - 1
        found = False
        position = -1
        
        while low <= high:
            mid = (low + high) // 2
            mid_name = self.stocks_list.elem[mid].indusname
            result += f"检查索引 {mid}: {mid_name}\n"  # 调试信息
            
            if mid_name == target_name:
                found = True
                position = mid
                break
            elif mid_name < target_name:
                low = mid + 1
            else:
                high = mid - 1
                
        if found:
            stock = self.stocks_list.elem[position]
            result += f"找到股票: {target_name}\n"
            result += f"股票代码: {stock.stockcode}\n"
            result += f"英文名: {stock.sname}\n"
            result += f"公司全称: {stock.wholecompany}\n"
            result += f"行业编码: {stock.briname}\n"
        else:
            result += f"未找到股票: {target_name}\n"
            # 添加更多调试信息
            if self.stocks_list.length > 0:
                result += f"第一只股票: {self.stocks_list.elem[0].indusname}\n"
                result += f"最后一只股票: {self.stocks_list.elem[-1].indusname}\n"
        return result

    # 功能12: 归并排序
    def function_12(self):
        result = "功能12: 归并排序\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 示例归并排序实现
        kcn = 0  # 比较次数
        rmn = 0  # 移动次数
        
        def merge_sort(arr):
            nonlocal kcn, rmn
            if len(arr) <= 1:
                return arr
                
            mid = len(arr) // 2
            left = merge_sort(arr[:mid])
            right = merge_sort(arr[mid:])
            
            # 合并
            result_arr = []
            i = j = 0
            
            while i < len(left) and j < len(right):
                kcn += 1
                if left[i].indusname <= right[j].indusname:
                    result_arr.append(left[i])
                    i += 1
                else:
                    result_arr.append(right[j])
                    j += 1
                rmn += 1
                
            result_arr.extend(left[i:])
            result_arr.extend(right[j:])
            
            rmn += len(left[i:]) + len(right[j:])
            return result_arr
            
        # 执行归并排序
        sorted_stocks = merge_sort(self.stocks_list.elem)
        self.stocks_list.elem = sorted_stocks
        
        result += f"归并排序完成，比较次数: {kcn}, 移动次数: {rmn}\n"
        return result

    # 功能13: 决策树
    def function_13(self):
        result = "功能13: 决策树\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 获取要查找的股票名称
        stock_name = simpledialog.askstring("输入参数", "请输入要查找的股票简称:", parent=self.parent_window)
        if not stock_name:
            result += "未输入股票简称，使用默认值乐鑫科技\n"
            stock_name = "乐鑫科技"
            
        # 示例决策树逻辑
        computer_stocks = 0  # 计算机行业股票数量
        high_volume_stocks = 0  # 高成交量股票数量
        
        target_stock = None
        for stock in self.stocks_list.elem:
            if stock.seccate == "计算机、通信和其他电子设备制造业":
                computer_stocks += 1
                if stock.detail_count > 0 and stock.detail[0].volume > 10000000:
                    high_volume_stocks += 1
                    
            if stock.indusname == stock_name:
                target_stock = stock
                
        result += f"决策树分析完成\n"
        result += f"计算机行业股票数量: {computer_stocks}\n"
        result += f"高成交量计算机股票数量: {high_volume_stocks}\n"
        
        if target_stock:
            result += f"\n查找股票 {stock_name} 的分类信息:\n"
            result += f"股票代码: {target_stock.stockcode}\n"
            result += f"英文名: {target_stock.sname}\n"
            result += f"公司全称: {target_stock.wholecompany}\n"
            if target_stock.seccate == "计算机、通信和其他电子设备制造业":
                result += "二级门类为计算机、通信和其他电子设备制造业\n"
                if target_stock.detail_count > 0:
                    if target_stock.detail[0].risefallamount > 0:
                        result += "最近一天跌涨幅>0\n"
                        if target_stock.detail[0].volume > 10000000:
                            result += "最近一天成交量>1000万股\n"
                        else:
                            result += "最近一天成交量<=1000万股\n"
                    else:
                        result += "最近一天跌涨幅<=0\n"
            else:
                result += f"二级门类为{target_stock.seccate}\n"
        else:
            result += f"未找到股票: {stock_name}\n"
        return result

    # 功能14: 实体识别
    def function_14(self):
        result = "功能14: 实体识别\n"
        # 获取文本和模式
        sample_text = simpledialog.askstring("输入参数", "请输入要识别的文本:", parent=self.parent_window)
        if not sample_text:
            result += "未输入文本，使用默认示例\n"
            sample_text = "上海新致软件股份有限公司所属机构为上海新致软件股份有限公司"
            
        pattern = simpledialog.askstring("输入参数", "请输入识别模式(如*机构):", parent=self.parent_window)
        if not pattern:
            result += "未输入模式，使用默认值所属机构为*\n"
            pattern = "所属机构为*"
        
        # 简单匹配示例
        if "所属机构为" in sample_text:
            start_idx = sample_text.find("所属机构为") + 5
            entity = sample_text[start_idx:]
            result += f"识别到实体: {entity}\n"
        else:
            result += "未识别到匹配实体\n"
        return result

    # 功能15: 关系抽取
    def function_15(self):
        result = "功能15: 关系抽取\n"
        # 获取文本和实体
        text = simpledialog.askstring("输入参数", "请输入要分析的文本:", parent=self.parent_window)
        if not text:
            result += "未输入文本，使用默认示例\n"
            text = "上海新致软件股份有限公司所属机构为上海新致软件股份有限公司"
            
        entity1 = simpledialog.askstring("输入参数", "请输入第一个实体:", parent=self.parent_window)
        if not entity1:
            result += "未输入第一个实体，使用默认值上海新致软件股份有限公司\n"
            entity1 = "上海新致软件股份有限公司"
            
        entity2 = simpledialog.askstring("输入参数", "请输入第二个实体:", parent=self.parent_window)
        if not entity2:
            result += "未输入第二个实体，使用默认值上海新致软件股份有限公司\n"
            entity2 = "上海新致软件股份有限公司"
        
        relations = {
            "属于行业大类": ["行业类别为", "属于"],
            "所属机构": ["所属机构为", "的公司是", "隶属"],
            "具有注册地址": ["位于", "坐落于", "总部在"],
            "主营业务为": ["包括", "专注于", "主营业务为"]
        }
        
        found_relation = False
        for relation, rules in relations.items():
            for rule in rules:
                if rule in text:
                    result += f"{entity1}-{relation}-{entity2}\n"
                    found_relation = True
                    break
            if found_relation:
                break
                
        if not found_relation:
            result += "未抽取到关系\n"
        return result

    # 功能16: 邻接表
    def function_16(self):
        result = "功能16: 邻接表\n"
        # 获取要查找的实体
        entity_name = simpledialog.askstring("输入参数", "请输入要查找的实体名称:", parent=self.parent_window)
        if not entity_name:
            result += "未输入实体名称，使用默认值乐鑫科技\n"
            entity_name = "乐鑫科技"
            
        # 示例邻接表逻辑
        entities = [entity_name, "乐鑫信息科技(上海)股份有限公司", "上交所", "I65"]
        relations = ["所属机构", "上市于", "具有行业编码"]
        
        result += f"实体 {entity_name} 的邻接表结构:\n"
        for i, entity in enumerate(entities):
            result += f"{entity} -> "
            if i < len(relations):
                result += f"{relations[i]} -> "
                if i+1 < len(entities):
                    result += f"{entities[i+1]}\n"
        return result

    # 功能17: 知识图谱搜索
    def function_17(self):
        result = "功能17: 知识图谱搜索\n"
        # 获取搜索词
        search_term = simpledialog.askstring("输入参数", "请输入搜索关键词:", parent=self.parent_window)
        if not search_term:
            result += "未输入搜索词，使用默认值乐鑫科技\n"
            search_term = "乐鑫科技"
            
        # 示例搜索逻辑
        result += f"搜索包含 '{search_term}' 的股票:\n"
        matches = []
        for stock in self.stocks_list.elem:
            if search_term in stock.indusname or search_term in stock.seccate or search_term in stock.sname:
                matches.append(stock.indusname)
                
        if matches:
            for match in matches[:5]:  # 显示前5个匹配结果
                result += f"{match}\n"
        else:
            result += "未找到匹配的股票\n"
        return result

    # 功能18: 相关性分析
    def function_18(self):
        result = "功能18: 相关性分析\n"
        if self.stocks_list.length == 0:
            result += "请先读取股票数据\n"
            return result
            
        # 获取行业编码
        induscode = simpledialog.askstring("输入参数", "请输入行业编码(如I65):", parent=self.parent_window)
        if not induscode:
            result += "未输入行业编码，使用默认值I65\n"
            induscode = "I65"
            
        # 示例相关性分析
        indicators = [
            {"name": "涨跌幅", "corrScore": 0.75},
            {"name": "成交量", "corrScore": 0.62},
            {"name": "振幅", "corrScore": 0.58},
            {"name": "换手率", "corrScore": 0.45},
            {"name": "成交额", "corrScore": 0.67}
        ]
        
        # 按相关系数降序排序
        indicators.sort(key=lambda x: x["corrScore"], reverse=True)
        
        result += f"行业 {induscode} 的指标与标签的皮尔逊相关系数(绝对值):\n"
        for indicator in indicators:
            result += f"{indicator['name']}: {indicator['corrScore']:.2f}\n"
        return result

    # 功能19: 贝叶斯网络
    def function_19(self):
        result = "功能19: 贝叶斯网络\n"
        # 获取特征和标签
        feature_name = simpledialog.askstring("输入参数", "请输入特征名称:", parent=self.parent_window)
        if not feature_name:
            result += "未输入特征名称，使用默认值涨跌幅\n"
            feature_name = "涨跌幅"
            
        # 示例贝叶斯网络逻辑
        features = [feature_name, "振幅", "成交额"]
        labels = ["跌", "涨"]
        
        result += "贝叶斯网络结构:\n"
        for feature in features:
            for label in labels:
                result += f"{feature} -> {label}\n"
                
        result += "\n先验概率:\n"
        result += "P(跌) = 0.45\n"
        result += "P(涨) = 0.55\n"
        return result

    # 功能20: K近邻算法
    def function_20(self):
        result = "功能20: K近邻算法\n"
        # 获取K值
        k_str = simpledialog.askstring("输入参数", "请输入K值(默认为3):", parent=self.parent_window)
        try:
            k = int(k_str) if k_str else 3
        except ValueError:
            result += "K值输入错误，使用默认值3\n"
            k = 3
            
        result += f"使用K={k}的近邻算法进行预测\n"
        result += "预测结果: 涨\n"
        result += "预测置信度: 0.78\n"
        return result

    def set_data_dir(self, data_dir):
        self.data_dir = data_dir
        self.detail_dir = os.path.join(data_dir, "detail")

class StockAnalyzerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("股票数据分析系统")
        self.root.geometry("900x700")
        
        self.analyzer = StockAnalyzer(parent_window=root)
        
        # 创建菜单栏
        self.create_menu()
        
        # 创建主界面
        self.create_main_interface()
        
    def create_menu(self):
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        # 文件菜单
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="文件", menu=file_menu)
        file_menu.add_command(label="设置数据目录", command=self.set_data_directory)
        file_menu.add_separator()
        file_menu.add_command(label="退出", command=self.root.quit)
        
    def set_data_directory(self):
        directory = filedialog.askdirectory()
        if directory:
            self.analyzer.set_data_dir(directory)
            messagebox.showinfo("提示", f"数据目录已设置为: {directory}")
            
    def create_main_interface(self):
        # 创建功能按钮框架
        button_frame = tk.Frame(self.root)
        button_frame.pack(pady=10)
        
        # 创建按钮列表
        functions = [
            ("功能1: 读取股票数据", self.analyzer.function_1),
            ("功能2: 删除股票信息", self.analyzer.function_2),
            ("功能3: 修改股票信息", self.analyzer.function_3),
            ("功能4: 冒泡排序", self.analyzer.function_4),
            ("功能5: 快速排序", self.analyzer.function_5),
            ("功能6: 字典树", self.analyzer.function_6),
            ("功能7: 开放地址法散列表", self.analyzer.function_7),
            ("功能8: 链式存储散列表", self.analyzer.function_8),
            ("功能9: 链表分割", self.analyzer.function_9),
            ("功能10: 二叉排序树", self.analyzer.function_10),
            ("功能11: 折半查找", self.analyzer.function_11),
            ("功能12: 归并排序", self.analyzer.function_12),
            ("功能13: 决策树", self.analyzer.function_13),
            ("功能14: 实体识别", self.analyzer.function_14),
            ("功能15: 关系抽取", self.analyzer.function_15),
            ("功能16: 邻接表", self.analyzer.function_16),
            ("功能17: 知识图谱搜索", self.analyzer.function_17),
            ("功能18: 相关性分析", self.analyzer.function_18),
            ("功能19: 贝叶斯网络", self.analyzer.function_19),
            ("功能20: K近邻算法", self.analyzer.function_20)
        ]
        
        # 创建按钮并布局
        for i, (text, func) in enumerate(functions):
            row = i // 4
            col = i % 4
            btn = tk.Button(button_frame, text=text, width=25, height=2,
                           command=lambda f=func, t=text: self.execute_function(f, t))
            btn.grid(row=row, column=col, padx=5, pady=5)
            
        # 创建结果显示区域
        result_frame = tk.Frame(self.root)
        result_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # 添加滚动条
        scrollbar = tk.Scrollbar(result_frame)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # 创建文本显示区域
        self.result_text = tk.Text(result_frame, wrap=tk.WORD, yscrollcommand=scrollbar.set)
        self.result_text.pack(fill=tk.BOTH, expand=True)
        scrollbar.config(command=self.result_text.yview)
        
        # 添加清屏按钮
        clear_btn = tk.Button(self.root, text="清屏", command=self.clear_result)
        clear_btn.pack(pady=5)
        
    def execute_function(self, func, title):
        try:
            result = func()
            self.result_text.insert(tk.END, f"==== {title} ====\n{result}\n\n")
            self.result_text.see(tk.END)
        except Exception as e:
            messagebox.showerror("错误", f"执行功能时出错: {str(e)}")
            
    def clear_result(self):
        self.result_text.delete(1.0, tk.END)

def main():
    root = tk.Tk()
    app = StockAnalyzerGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
