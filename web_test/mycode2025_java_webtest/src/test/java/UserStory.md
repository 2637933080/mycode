# User Story: 查找计算机科学本科课程

## 故事描述
作为一个 prospective student（潜在学生），我希望能够通过多种路径找到曼彻斯特大学的计算机科学本科课程信息，以便我可以了解更多关于该课程的详细信息。

## 场景

### 场景1: 通过主页搜索功能查找课程
Given 我在曼彻斯特大学的主页上
When 我点击搜索图标并在搜索框中输入"Computer Science"
And 点击搜索按钮
And 在搜索结果中点击"BSc Computer Science"链接
Then 我应该被带到BSc Computer Science课程页面
And 页面标题应该是"BSc Computer Science"

### 场景2: 通过导航菜单查找课程
Given 我在曼彻斯特大学的主页上
When 我将鼠标悬停在"Study"菜单上
And 点击"Undergraduate"子菜单项
And 在课程页面上点击搜索框并输入"Computer Science"
And 点击搜索按钮
And 在搜索结果中点击相关的课程链接
Then 我应该被带到BSc Computer Science课程页面
And 页面标题应该是"BSc Computer Science"

### 场景3: 通过直接访问课程列表页面查找课程
Given 我直接访问本科课程列表页面
When 我找到搜索框并输入"Computer Science"
And 点击搜索按钮
And 在搜索结果中点击"BSc Computer Science"链接
Then 我应该被带到BSc Computer Science课程页面
And 页面标题应该是"BSc Computer Science"

## 超链接图模型
```
[主页] --(点击搜索图标)--> [主页搜索结果] --(点击课程链接)--> [BSc计算机科学课程页面]
   |                                                              ^
   |-(悬停在Study上)                                              |
   |-(点击Undergraduate)                                          |
   |-(搜索Computer Science)---------------------------------------|
   |
   |-(直接访问/study/undergraduate/courses/)                      |
   |-(搜索Computer Science)---------------------------------------|

```

这个测试计划实现了超链接图模型的节点覆盖率和超链接覆盖率：
- 节点: 主页、主页搜索结果、本科课程页面、BSc计算机科学课程页面
- 边: 不同路径之间的跳转关系