import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import cv2
from PIL import Image, ImageTk
import numpy as np
import os

class FaceDetectorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("人脸识别工具")
        self.root.geometry("900x700")
        self.root.minsize(800, 600)
        
        # 设置样式
        self.setup_styles()
        
        # 初始化人脸分类器
        self.face_cascade = cv2.CascadeClassifier(
            cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
        )
        
        # 创建变量
        self.image_path = None
        self.original_image = None
        self.processed_image = None
        self.display_image_obj = None  # 用于存储当前显示的图像对象
        
        self.create_widgets()
    
    def setup_styles(self):
        """设置界面样式"""
        style = ttk.Style()
        style.configure("Title.TLabel", font=("微软雅黑", 14, "bold"))
        style.configure("Action.TButton", padding=8, font=("微软雅黑", 10))
        style.map("Action.TButton",
                  foreground=[('pressed', '#ffffff'), ('active', '#000000')],
                  background=[('pressed', '#4d79cc'), ('active', '#b3c6ff')])
        style.configure("Primary.TButton", padding=8, font=("微软雅黑", 10))
        style.map("Primary.TButton",
                  foreground=[('pressed', '#ffffff'), ('active', '#000000')],
                  background=[('pressed', '#4d79cc'), ('active', '#6699ff')])
        style.configure("Secondary.TButton", padding=6, font=("微软雅黑", 9))
        style.map("Secondary.TButton",
                  foreground=[('pressed', '#ffffff'), ('active', '#000000')],
                  background=[('pressed', '#666666'), ('active', '#cccccc')])
        style.configure("Param.TLabelFrame", background="#f0f0f0")
        style.configure("Status.TLabel", background="#e9e9e9", foreground="#333333")
    
    def create_widgets(self):
        # 标题区域
        title_frame = ttk.Frame(self.root)
        title_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=(10, 5))
        
        title_label = ttk.Label(title_frame, text="人脸识别工具", style="Title.TLabel")
        title_label.pack(side=tk.LEFT)
        
        # 创建顶部控制面板
        control_frame = ttk.LabelFrame(self.root, text="控制面板", padding=10)
        control_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=5)
        
        # 第一行控制按钮
        button_frame = ttk.Frame(control_frame)
        button_frame.pack(side=tk.TOP, fill=tk.X, pady=(0, 10))
        
        # 选择图片按钮
        self.select_btn = ttk.Button(button_frame, text="选择图片", command=self.select_image, style="Primary.TButton")
        self.select_btn.pack(side=tk.LEFT, padx=5)
        self.add_button_hover_effect(self.select_btn)
        
        # 检测人脸按钮
        self.detect_btn = ttk.Button(button_frame, text="检测人脸", command=self.detect_faces, state=tk.DISABLED, style="Action.TButton")
        self.detect_btn.pack(side=tk.LEFT, padx=5)
        self.add_button_hover_effect(self.detect_btn)
        
        # 保存结果按钮
        self.save_btn = ttk.Button(button_frame, text="保存结果", command=self.save_image, state=tk.DISABLED, style="Secondary.TButton")
        self.save_btn.pack(side=tk.LEFT, padx=5)
        self.add_button_hover_effect(self.save_btn)
        
        # 图像显示区域
        display_labelframe = ttk.LabelFrame(self.root, text="图像显示区", padding=5)
        display_labelframe.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        self.canvas_frame = ttk.Frame(display_labelframe)
        self.canvas_frame.pack(fill=tk.BOTH, expand=True)
        
        # 创建滚动条
        self.scroll_y = ttk.Scrollbar(self.canvas_frame, orient=tk.VERTICAL)
        self.scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.scroll_x = ttk.Scrollbar(self.canvas_frame, orient=tk.HORIZONTAL)
        self.scroll_x.pack(side=tk.BOTTOM, fill=tk.X)
        
        # 创建画布用于显示图像
        self.canvas = tk.Canvas(self.canvas_frame, 
                               yscrollcommand=self.scroll_y.set,
                               xscrollcommand=self.scroll_x.set,
                               bg='#f5f5f5')
        self.canvas.pack(fill=tk.BOTH, expand=True)
        
        self.scroll_y.config(command=self.canvas.yview)
        self.scroll_x.config(command=self.canvas.xview)
        
        # 状态栏
        self.status_var = tk.StringVar(value="就绪")
        self.status_bar = ttk.Label(self.root, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W, style="Status.TLabel")
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)
        
        # 参数设置框架（移到最底部，在状态栏上方）
        param_frame = ttk.LabelFrame(self.root, text="检测参数", padding=10)
        param_frame.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=5)
        
        # Scale Factor
        scale_frame = ttk.Frame(param_frame)
        scale_frame.pack(side=tk.TOP, fill=tk.X, pady=5)
        ttk.Label(scale_frame, text="Scale Factor:").pack(side=tk.LEFT)
        self.scale_var = tk.DoubleVar(value=1.2)
        self.scale_spin = ttk.Spinbox(scale_frame, from_=1.1, to=2.0, increment=0.1, textvariable=self.scale_var, width=10)
        self.scale_spin.pack(side=tk.RIGHT, padx=(10, 0))
        
        # Min Neighbors
        neighbor_frame = ttk.Frame(param_frame)
        neighbor_frame.pack(side=tk.TOP, fill=tk.X, pady=5)
        ttk.Label(neighbor_frame, text="Min Neighbors:").pack(side=tk.LEFT)
        self.neighbors_var = tk.IntVar(value=5)
        self.neighbors_spin = ttk.Spinbox(neighbor_frame, from_=1, to=10, increment=1, textvariable=self.neighbors_var, width=10)
        self.neighbors_spin.pack(side=tk.RIGHT, padx=(10, 0))
        
        # 绑定窗口大小改变事件
        self.root.bind('<Configure>', self.on_window_configure)
        self.last_width = 0
        self.last_height = 0
    
    def add_button_hover_effect(self, button):
        """添加按钮悬停效果"""
        def on_enter(e):
            button['cursor'] = 'hand2'
            
        def on_leave(e):
            button['cursor'] = ''
            
        button.bind("<Enter>", on_enter)
        button.bind("<Leave>", on_leave)
    
    def select_image(self):
        # 添加按钮点击动画效果
        self.animate_button_click(self.select_btn)
        
        file_path = filedialog.askopenfilename(
            title="选择图片",
            filetypes=[
                ("Image Files", "*.jpg *.jpeg *.png *.bmp *.tiff"),
                ("All Files", "*.*")
            ]
        )
        
        if file_path:
            self.image_path = file_path
            self.load_image()
            self.detect_btn.config(state=tk.NORMAL)
            self.status_var.set(f"已加载图片: {os.path.basename(file_path)}")
    
    def load_image(self):
        # 使用 OpenCV 读取图像
        self.original_image = cv2.imread(self.image_path)
        if self.original_image is None:
            messagebox.showerror("错误", "无法加载图片")
            return
        
        # 显示图像
        self.display_image(self.original_image)
    
    def resize_image_to_fit_canvas(self, image):
        """调整图像大小以适应画布"""
        canvas_width = self.canvas.winfo_width()
        canvas_height = self.canvas.winfo_height()
        
        # 避免除零错误和负数
        if canvas_width <= 1 or canvas_height <= 1:
            canvas_width = 800
            canvas_height = 500
            
        # 获取原始图像尺寸
        height, width = image.shape[:2]
        
        # 计算缩放比例
        scale_width = canvas_width / width
        scale_height = canvas_height / height
        scale = min(scale_width, scale_height)
        
        # 如果图像比画布小，则不放大
        if scale >= 1:
            return image
        
        # 调整图像大小
        new_width = int(width * scale)
        new_height = int(height * scale)
        resized_image = cv2.resize(image, (new_width, new_height), interpolation=cv2.INTER_AREA)
        
        return resized_image
    
    def display_image(self, image):
        # 先调整图像大小以适应画布
        resized_image = self.resize_image_to_fit_canvas(image)
        
        # 转换颜色格式 BGR to RGB
        rgb_image = cv2.cvtColor(resized_image, cv2.COLOR_BGR2RGB)
        
        # 转换为 PIL Image
        pil_image = Image.fromarray(rgb_image)
        
        # 转换为 Tkinter PhotoImage
        self.display_image_obj = ImageTk.PhotoImage(pil_image)
        
        # 在画布上显示图像
        self.canvas.delete("all")
        self.canvas.create_image(0, 0, anchor=tk.NW, image=self.display_image_obj)
        
        # 更新滚动区域
        self.canvas.config(scrollregion=self.canvas.bbox(tk.ALL))
    
    def on_window_configure(self, event=None):
        """窗口大小改变时的处理函数"""
        # 避免重复调用
        if event.widget != self.root:
            return
            
        if self.root.winfo_width() != self.last_width or self.root.winfo_height() != self.last_height:
            self.last_width = self.root.winfo_width()
            self.last_height = self.root.winfo_height()
            
            # 如果有图像则重新调整大小
            if self.processed_image is not None:
                self.display_image(self.processed_image)
            elif self.original_image is not None:
                self.display_image(self.original_image)
    
    def detect_faces(self):
        # 添加按钮点击动画效果
        self.animate_button_click(self.detect_btn)
        
        if self.original_image is None:
            return
        
        # 获取参数
        scale_factor = self.scale_var.get()
        min_neighbors = self.neighbors_var.get()
        
        # 转换为灰度图
        gray = cv2.cvtColor(self.original_image, cv2.COLOR_BGR2GRAY)
        
        # 检测人脸
        faces = self.face_cascade.detectMultiScale(
            gray, 
            scaleFactor=scale_factor, 
            minNeighbors=min_neighbors
        )
        
        # 绘制矩形框
        self.processed_image = self.original_image.copy()
        for (x, y, w, h) in faces:
            cv2.rectangle(self.processed_image, (x, y), (x+w, y+h), (255, 255, 0), 2)
        
        # 显示处理后的图像
        self.display_image(self.processed_image)
        
        # 启用保存按钮
        self.save_btn.config(state=tk.NORMAL)
        
        # 更新状态栏
        self.status_var.set(f"检测到 {len(faces)} 张人脸")
    
    def save_image(self):
        # 添加按钮点击动画效果
        self.animate_button_click(self.save_btn)
        
        if self.processed_image is None:
            return
        
        file_path = filedialog.asksaveasfilename(
            title="保存图片",
            defaultextension=".jpg",
            filetypes=[
                ("JPEG files", "*.jpg"),
                ("PNG files", "*.png"),
                ("BMP files", "*.bmp"),
                ("All files", "*.*")
            ]
        )
        
        if file_path:
            success = cv2.imwrite(file_path, self.processed_image)
            if success:
                self.status_var.set(f"图片已保存到: {file_path}")
                messagebox.showinfo("成功", "图片保存成功!")
            else:
                messagebox.showerror("错误", "保存图片失败")
    
    def animate_button_click(self, button):
        """按钮点击动画效果"""
        original_state = button.cget('state')
        button.config(state='disabled')
        button.update_idletasks()
        self.root.after(100, lambda: button.config(state=original_state))

def main():
    root = tk.Tk()
    app = FaceDetectorApp(root)
    root.mainloop()

if __name__ == "__main__":
    main()