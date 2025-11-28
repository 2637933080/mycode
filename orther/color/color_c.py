import cv2
import numpy as np
import tkinter as tk
from tkinter import filedialog, ttk
from PIL import Image, ImageTk

class ColorConverter:
    def __init__(self, root):
        """
        初始化色彩转换器
        :param root: tkinter根窗口
        """
        self.root = root
        self.root.title("图像色彩转换工具")
        self.root.geometry("800x600")
        
        # 当前图像相关变量
        self.original_image = None  # 原始图像
        self.converted_image = None  # 转换后的图像
        self.display_image = None  # 显示用图像
        
        # 创建界面
        self.create_widgets()
        
    def create_widgets(self):
        """
        创建GUI界面组件
        """
        # 创建顶部控制面板
        control_frame = tk.Frame(self.root)
        control_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=10)
        
        # 文件选择按钮
        self.load_button = tk.Button(control_frame, text="选择图片", command=self.load_image)
        self.load_button.pack(side=tk.LEFT, padx=5)
        
        # 色彩空间选择下拉框
        tk.Label(control_frame, text="转换到:").pack(side=tk.LEFT, padx=(20, 5))
        self.color_options = [
            "原始图像",
            "灰度图 (GRAY)",
            "HSV色彩空间",
            "LAB色彩空间",
            "YUV色彩空间",
            "RGB色彩空间"
        ]
        self.selected_color = tk.StringVar(value=self.color_options[0])
        self.color_menu = ttk.Combobox(control_frame, textvariable=self.selected_color, 
                                      values=self.color_options, state="readonly", width=15)
        self.color_menu.pack(side=tk.LEFT, padx=5)
        self.color_menu.bind("<<ComboboxSelected>>", self.convert_color)
        
        # 保存图片按钮
        self.save_button = tk.Button(control_frame, text="保存图片", command=self.save_image, state=tk.DISABLED)
        self.save_button.pack(side=tk.LEFT, padx=5)
        
        # 创建图像显示区域
        self.image_frame = tk.Frame(self.root)
        self.image_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=(0, 10))
        
        # 创建画布用于显示图像
        self.canvas = tk.Canvas(self.image_frame, bg="lightgray")
        self.canvas.pack(fill=tk.BOTH, expand=True)
        
        # 状态栏
        self.status_bar = tk.Label(self.root, text="请选择一张图片开始", relief=tk.SUNKEN, anchor=tk.W)
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)
    
    def load_image(self):
        """
        从文件系统加载图像
        """
        file_path = filedialog.askopenfilename(
            title="选择图片文件",
            filetypes=[
                ("Image Files", "*.jpg *.jpeg *.png *.bmp *.tiff"),
                ("JPEG Files", "*.jpg *.jpeg"),
                ("PNG Files", "*.png"),
                ("BMP Files", "*.bmp"),
                ("TIFF Files", "*.tiff"),
                ("All Files", "*.*")
            ]
        )
        
        if file_path:
            try:
                # 使用OpenCV读取图像
                self.original_image = cv2.imread(file_path)
                if self.original_image is None:
                    self.status_bar.config(text="错误：无法加载图像文件")
                    return
                
                # 更新状态栏
                h, w = self.original_image.shape[:2]
                self.status_bar.config(text=f"已加载图片: {file_path} 尺寸: {w}x{h}")
                
                # 启用保存按钮
                self.save_button.config(state=tk.NORMAL)
                
                # 显示原始图像
                self.display_image = self.original_image.copy()
                self.show_image()
                
            except Exception as e:
                self.status_bar.config(text=f"加载图像时出错: {str(e)}")
    
    def convert_color(self, event=None):
        """
        根据用户选择进行色彩空间转换
        """
        if self.original_image is None:
            return
            
        # 获取选中的色彩空间
        selected = self.selected_color.get()
        
        try:
            # 根据选择执行不同的色彩转换
            if selected == "原始图像（RGB）":
                self.converted_image = self.original_image.copy()
            elif selected == "灰度图 (GRAY)":
                # 转换为灰度图
                self.converted_image = cv2.cvtColor(self.original_image, cv2.COLOR_BGR2GRAY)
            elif selected == "HSV色彩空间":
                # 转换为HSV色彩空间
                self.converted_image = cv2.cvtColor(self.original_image, cv2.COLOR_BGR2HSV)
            elif selected == "LAB色彩空间":
                # 转换为LAB色彩空间
                self.converted_image = cv2.cvtColor(self.original_image, cv2.COLOR_BGR2LAB)
            elif selected == "YUV色彩空间":
                # 转换为YUV色彩空间
                self.converted_image = cv2.cvtColor(self.original_image, cv2.COLOR_BGR2YUV)
            elif selected == "RGB色彩空间":
                # 转换为RGB色彩空间 (OpenCV默认是BGR)
                self.converted_image = cv2.cvtColor(self.original_image, cv2.COLOR_BGR2RGB)
            
            # 设置要显示的图像
            self.display_image = self.converted_image.copy()
            self.show_image()
            
            self.status_bar.config(text=f"已完成色彩转换: {selected}")
            
        except Exception as e:
            self.status_bar.config(text=f"色彩转换时出错: {str(e)}")
    
    def show_image(self):
        """
        在画布上显示图像
        """
        if self.display_image is None:
            return
            
        # 获取画布尺寸
        canvas_width = self.canvas.winfo_width()
        canvas_height = self.canvas.winfo_height()
        
        # 如果获取不到画布尺寸，则使用默认值
        if canvas_width <= 1 or canvas_height <= 1:
            canvas_width = 600
            canvas_height = 400
        
        # 调整图像大小以适应画布
        h, w = self.display_image.shape[:2]
        scale_w = canvas_width / w
        scale_h = canvas_height / h
        scale = min(scale_w, scale_h)
        
        # 计算新尺寸
        new_w = int(w * scale)
        new_h = int(h * scale)
        
        # 调整图像大小
        if len(self.display_image.shape) == 3:
            # 彩色图像
            resized_img = cv2.resize(self.display_image, (new_w, new_h), interpolation=cv2.INTER_AREA)
        else:
            # 灰度图像
            resized_img = cv2.resize(self.display_image, (new_w, new_h), interpolation=cv2.INTER_AREA)
        
        # 将OpenCV图像转换为PIL图像
        if len(resized_img.shape) == 3:
            # 彩色图像
            pil_image = Image.fromarray(cv2.cvtColor(resized_img, cv2.COLOR_BGR2RGB))
        else:
            # 灰度图像
            pil_image = Image.fromarray(resized_img)
        
        # 转换为tkinter可用的格式
        self.tk_image = ImageTk.PhotoImage(pil_image)
        
        # 清除画布并显示新图像
        self.canvas.delete("all")
        x = (canvas_width - new_w) // 2
        y = (canvas_height - new_h) // 2
        self.canvas.create_image(x, y, anchor=tk.NW, image=self.tk_image)
        self.canvas.config(scrollregion=self.canvas.bbox(tk.ALL))
    
    def save_image(self):
        """
        保存当前转换后的图像
        """
        if self.converted_image is None:
            return
            
        file_path = filedialog.asksaveasfilename(
            title="保存图片",
            defaultextension=".png",
            filetypes=[
                ("PNG Files", "*.png"),
                ("JPEG Files", "*.jpg"),
                ("BMP Files", "*.bmp"),
                ("TIFF Files", "*.tiff"),
                ("All Files", "*.*")
            ]
        )
        
        if file_path:
            try:
                # 对于灰度图像需要特殊处理
                if len(self.converted_image.shape) == 2:
                    # 灰度图像
                    success = cv2.imwrite(file_path, self.converted_image)
                else:
                    # 彩色图像
                    success = cv2.imwrite(file_path, self.converted_image)
                    
                if success:
                    self.status_bar.config(text=f"图像已保存到: {file_path}")
                else:
                    self.status_bar.config(text="保存失败")
                    
            except Exception as e:
                self.status_bar.config(text=f"保存图像时出错: {str(e)}")
    
    def on_resize(self, event):
        """
        窗口大小改变事件处理
        """
        if self.display_image is not None:
            self.show_image()

def main():
    """
    主函数，启动应用程序
    """
    root = tk.Tk()
    
    # 创建色彩转换器实例
    app = ColorConverter(root)
    
    # 绑定窗口大小改变事件
    root.bind("<Configure>", lambda e: app.on_resize(e))
    
    # 运行主循环
    root.mainloop()

if __name__ == "__main__":
    main()