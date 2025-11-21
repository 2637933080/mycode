import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Arrow
import matplotlib as mpl
# Set the font that supports Chinese characters
plt.rcParams['font.sans-serif'] = ['SimHei']  # For Windows
# Alternative fonts: ['Microsoft YaHei', 'WenQuanYi Micro Hei']
plt.rcParams['axes.unicode_minus'] = False  # To correctly display the minus sign

# If you're using a different OS, you might need these alternatives:
# Mac: ['Arial Unicode MS', 'Heiti TC']
# Linux: ['WenQuanYi Zen Hei', 'WenQuanYi Micro Hei']

# For more consistent rendering across platforms:
mpl.rcParams['font.family'] = 'sans-serif'
def draw_box(ax, xy, text, width=2.5, height=0.8, color='#e0ecf7'):
    box = FancyBboxPatch(
        (xy[0], xy[1]), width, height,
        boxstyle="round,pad=0.1",
        linewidth=1.5, edgecolor='navy', facecolor=color, zorder=2
    )
    ax.add_patch(box)
    ax.text(xy[0] + width/2, xy[1] + height/2, text, ha='center', va='center', fontsize=12, zorder=3)

def draw_arrow(ax, start, end):
    ax.annotate(
        '', xy=end, xytext=start,
        arrowprops=dict(arrowstyle="->", lw=2, color='gray'),
        annotation_clip=False
    )

fig, ax = plt.subplots(figsize=(10, 7))
ax.set_xlim(0, 8)
ax.set_ylim(0, 8)
ax.axis('off')

# 绘制各模块
draw_box(ax, (2.7, 7), "数据输入\n(历史路径/强度/气象)", color='#b3cde3')
draw_box(ax, (2.7, 5.7), "数据预处理\n(清洗/标准化/特征工程)", color='#fbb4ae')
draw_box(ax, (2.7, 4.4), "多特征融合", color='#ccebc5')
draw_box(ax, (2.7, 3.1), "LSTM/双向LSTM\n时序建模", color='#decbe4')
draw_box(ax, (2.7, 1.8), "注意力机制", color='#fed9a6')
draw_box(ax, (2.7, 0.5), "多目标输出\n(路径/强度/降水等)", color='#ffffcc')

# 箭头
draw_arrow(ax, (4, 7), (4, 6.5))
draw_arrow(ax, (4, 5.7), (4, 5.2))
draw_arrow(ax, (4, 4.4), (4, 3.9))
draw_arrow(ax, (4, 3.1), (4, 2.6))
draw_arrow(ax, (4, 1.8), (4, 1.3))

# 侧边说明
ax.text(6.2, 4.4, "特征示例：\n- 经度/纬度\n- 风速/气压\n- 月份/降水量", fontsize=11, color='dimgray', va='center')

plt.tight_layout()
plt.show()