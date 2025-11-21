import os
import sys
from qgis.core import (
    QgsApplication, QgsProject, QgsRasterLayer, QgsVectorLayer,
    QgsField, QgsWkbTypes, QgsCoordinateReferenceSystem,
    QgsCoordinateTransform, QgsVectorFileWriter, QgsMapLayer,
    QgsFeatureRequest,QgsGeometry,QgsPointXY,QgsSymbolLayer, QgsSimpleMarkerSymbolLayer,
                       QgsSimpleLineSymbolLayer, QgsSimpleFillSymbolLayer,
                       QgsColorRampShader, QgsRasterShader, QgsSingleBandPseudoColorRenderer,
    QgsFeatureRequest, QgsGeometry, QgsPalLayerSettings, QgsVectorLayerSimpleLabeling,
    QgsSimpleMarkerSymbolLayer, QgsSimpleLineSymbolLayer, QgsSimpleFillSymbolLayer,QgsTextFormat,
    QgsGradientColorRamp, QgsColorRamp,QgsGradientColorRamp, QgsColorRamp,
    QgsMarkerSymbol, QgsLineSymbol, QgsFillSymbol, QgsSingleSymbolRenderer,
    QgsCategorizedSymbolRenderer, QgsGraduatedSymbolRenderer, QgsRendererRange, QgsRendererCategory
)
from qgis.gui import QgsMapCanvas, QgsProjectionSelectionDialog
from qgis.PyQt.QtCore import QVariant

from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QFileDialog, QLabel, QMessageBox, QInputDialog, # 补充：导入 QInputDialog
QDialog, QFormLayout, QLineEdit, QSpinBox,
                             QDoubleSpinBox, QComboBox, QPushButton, QDialogButtonBox,
                             QColorDialog, QTabWidget, QTextEdit, QLabel, QTableWidget,
                             QTableWidgetItem, QHeaderView
)

from qgis.PyQt.QtGui import QColor
from PyQt5.QtCore import Qt

QGIS_INSTALL_PATH = "D:/OSGeo4W"
PYTHON_VERSION = "Python312"

# 添加依赖路径
sys.path.extend([
    os.path.join(QGIS_INSTALL_PATH, "apps", "qgis-ltr", "python"),
    os.path.join(QGIS_INSTALL_PATH, "apps", PYTHON_VERSION),
    os.path.join(QGIS_INSTALL_PATH, "apps", PYTHON_VERSION, "Lib", "site-packages"),
    os.path.join(QGIS_INSTALL_PATH, "apps", "Qt5", "Lib"),
    os.path.join(QGIS_INSTALL_PATH, "apps", "Qt5", "plugins"),
    os.path.join(QGIS_INSTALL_PATH, "bin"),
])

# ---------------------- GUI主窗口类 ----------------------
class QGISFileLoader(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("GIS数据编辑与投影工具")
        self.setGeometry(100, 100, 1200, 800)  # 窗口位置和大小

        # 初始化QGIS核心
        self.init_qgis_core()

        # 初始化UI组件
        self.init_ui()

    def init_qgis_core(self):
        """初始化QGIS核心环境"""
        os.environ["QGIS_PREFIX_PATH"] = QGIS_INSTALL_PATH
        self.qgs_app = QgsApplication([], False)
        self.qgs_app.setPrefixPath(QGIS_INSTALL_PATH, True)
        self.qgs_app.initQgis()

    def init_ui(self):
        """构建GUI界面"""
        # 中心窗口和主布局
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)

        # 1. 按钮区域（水平布局）
        btn_layout = QHBoxLayout()
        btn_layout.setSpacing(10)

        # 加载GeoTiff按钮
        self.tiff_btn = QPushButton("加载GeoTiff影像/DEM")
        self.tiff_btn.clicked.connect(self.load_geotiff)
        btn_layout.addWidget(self.tiff_btn)

        # 加载shp按钮
        self.shp_btn = QPushButton("加载shp矢量文件")
        self.shp_btn.clicked.connect(self.load_shp)
        btn_layout.addWidget(self.shp_btn)

        # 加载qgs工程按钮
        self.qgs_btn = QPushButton("加载qgs工程文件")
        self.qgs_btn.clicked.connect(self.load_qgs)
        btn_layout.addWidget(self.qgs_btn)

        # 矢量编辑按钮
        self.edit_btn = QPushButton("编辑选中矢量图层")
        self.edit_btn.clicked.connect(self.edit_vector)
        btn_layout.addWidget(self.edit_btn)

        # 坐标系转换按钮
        self.crs_btn = QPushButton("矢量图层坐标系转换")
        self.crs_btn.clicked.connect(self.convert_crs)
        btn_layout.addWidget(self.crs_btn)

        # 添加按钮区域到主布局
        main_layout.addLayout(btn_layout)

        # 2. 地图画布（占满剩余空间）
        self.canvas = QgsMapCanvas()
        self.canvas.setMinimumSize(800, 600)
        main_layout.addWidget(self.canvas)

        # 3. 状态提示标签
        self.status_label = QLabel("就绪：请点击按钮加载文件或编辑数据")
        main_layout.addWidget(self.status_label)

    def load_geotiff(self):
        """通过文件对话框选择并加载GeoTiff"""
        file_path, _ = QFileDialog.getOpenFileName(
            self, "选择GeoTiff文件", "", "GeoTiff Files (*.tif *.tiff)"
        )
        if not file_path:
            return  # 用户取消选择

        # 加载栅格图层
        raster_layer = QgsRasterLayer(file_path, os.path.basename(file_path), "gdal")
        if raster_layer.isValid():
            QgsProject.instance().addMapLayer(raster_layer)
            self.update_canvas()
            self.status_label.setText(f"成功加载GeoTiff：{os.path.basename(file_path)}")
            QMessageBox.information(self, "成功", f"GeoTiff文件加载完成！\n文件名：{os.path.basename(file_path)}")
        else:
            self.status_label.setText(f"失败：GeoTiff加载失败 - {raster_layer.error().message()}")
            QMessageBox.warning(self, "失败", f"GeoTiff加载失败！\n错误：{raster_layer.error().message()}")

    def load_shp(self):
        """通过文件对话框选择并加载shp"""
        file_path, _ = QFileDialog.getOpenFileName(
            self, "选择shp矢量文件", "", "Shapefile Files (*.shp)"
        )
        if not file_path:
            return

        # 加载矢量图层
        vector_layer = QgsVectorLayer(file_path, os.path.basename(file_path), "ogr")
        if vector_layer.isValid():
            QgsProject.instance().addMapLayer(vector_layer)
            self.update_canvas()
            self.status_label.setText(f"成功加载shp：{os.path.basename(file_path)}")
            QMessageBox.information(self, "成功", f"shp文件加载完成！\n文件名：{os.path.basename(file_path)}")
        else:
            self.status_label.setText(f"失败：shp加载失败 - {vector_layer.error().message()}")
            QMessageBox.warning(self, "失败", f"shp加载失败！\n错误：{vector_layer.error().message()}")

    def load_qgs(self):
        """通过文件对话框选择并加载qgs工程"""
        file_path, _ = QFileDialog.getOpenFileName(
            self, "选择qgs工程文件", "", "QGIS Project Files (*.qgs *.qgz)"
        )
        if not file_path:
            return

        # 清空当前项目并加载新工程
        QgsProject.instance().clear()
        if QgsProject.instance().read(file_path):
            self.update_canvas()
            self.status_label.setText(f"成功加载qgs工程：{os.path.basename(file_path)}")
            QMessageBox.information(self, "成功", f"qgs工程加载完成！\n文件名：{os.path.basename(file_path)}")
        else:
            self.status_label.setText(f"失败：qgs工程加载失败")
            QMessageBox.warning(self, "失败", "qgs工程加载失败！请检查文件完整性")

    def edit_vector(self):
        """编辑当前选中的矢量图层（打开交互式编辑窗口）"""
        # 获取当前矢量图层
        vector_layers = [l for l in QgsProject.instance().mapLayers().values()
                         if l.type() == QgsMapLayer.VectorLayer and l.isValid()]
        if not vector_layers:
            QMessageBox.warning(self, "警告", "没有可用的矢量图层！请先加载shp文件")
            return

        # 选择要编辑的图层
        if len(vector_layers) > 1:
            layer_names = [l.name() for l in vector_layers]
            selected_name, ok = QInputDialog.getItem(self, "选择编辑图层", "请选择要编辑的矢量图层：", layer_names)
            if not ok:
                return
            target_layer = next(l for l in vector_layers if l.name() == selected_name)
        else:
            target_layer = vector_layers[0]

        # 打开编辑子窗口
        edit_dialog = VectorEditDialog(target_layer, self)
        if edit_dialog.exec_():
            self.update_canvas()
            self.status_label.setText(f"成功编辑矢量图层：{target_layer.name()}")


    def convert_crs(self):
        """矢量图层坐标系转换：显示源坐标系，并在GUI选择目标坐标系"""
        # 获取可用矢量图层
        vector_layers = [l for l in QgsProject.instance().mapLayers().values()
                         if l.type() == QgsMapLayer.VectorLayer and l.isValid()]
        if not vector_layers:
            QMessageBox.warning(self, "警告", "没有可用的矢量图层！请先加载shp文件")
            return

        layer_names = [l.name() for l in vector_layers]
        selected_name, ok = QInputDialog.getItem(self, "选择转换图层", "请选择要转换的矢量图层：", layer_names, 0, False)
        if not ok:
            return
        source_layer = next(l for l in vector_layers if l.name() == selected_name)

        # 显示源坐标系
        src_crs = source_layer.crs()
        QMessageBox.information(
            self, "当前图层坐标系",
            f"图层：{source_layer.name()}\n"
            f"源坐标系：{src_crs.authid()} - {src_crs.description()}"
        )

        # 选择目标坐标系
        dlg = QgsProjectionSelectionDialog(self)
        dlg.setWindowTitle("选择目标投影坐标系")
        try:
            dlg.setCrs(src_crs)
        except Exception:
            pass
        if dlg.exec_() != QDialog.Accepted:
            return
        try:
            target_crs = dlg.crs()
        except Exception:
            target_crs = dlg.selectedCrs()

        # 选择输出文件
        default_name = f"{source_layer.name()}_{target_crs.authid().replace(':', '')}.shp"
        output_path, _ = QFileDialog.getSaveFileName(
            self, "保存转换后的shp", default_name, "Shapefile Files (*.shp)"
        )
        if not output_path:
            return
        if not output_path.lower().endswith(".shp"):
            output_path += ".shp"

        # 写出并重投影
        options = QgsVectorFileWriter.SaveVectorOptions()
        options.driverName = "ESRI Shapefile"
        options.fileEncoding = "UTF-8"
        options.destinationCrs = target_crs

        err = QgsVectorFileWriter.writeAsVectorFormatV3(
            source_layer,
            output_path,
            QgsProject.instance().transformContext(),
            options
        )

        if err[0] == QgsVectorFileWriter.NoError:
            QMessageBox.information(
                self, "成功",
                f"坐标系转换完成！\n源：{src_crs.authid()} → 目标：{target_crs.authid()}\n保存路径：{output_path}"
            )
            self.status_label.setText(
                f"成功转换：{source_layer.name()}  {src_crs.authid()} → {target_crs.authid()}"
            )
            if QMessageBox.question(self, "加载", "是否加载转换后的图层？",
                                    QMessageBox.Yes | QMessageBox.No) == QMessageBox.Yes:
                self.load_shp_path(output_path)
        else:
            QMessageBox.warning(self, "失败", f"坐标系转换失败！\n错误：{err[1]}")
            self.status_label.setText(f"失败：坐标系转换出错 - {err[1]}")

    def load_shp_path(self, file_path):
        """通过路径直接加载shp（用于转换后自动加载）"""
        vector_layer = QgsVectorLayer(file_path, os.path.basename(file_path), "ogr")
        if vector_layer.isValid():
            QgsProject.instance().addMapLayer(vector_layer)
            self.update_canvas()

    def update_canvas(self):
        """更新地图画布（加载所有图层并自适应范围）"""
        layers = list(QgsProject.instance().mapLayers().values())
        if layers:
            self.canvas.setLayers(layers)
            # 自适应所有图层的联合范围
            combined_extent = layers[0].extent()
            for layer in layers[1:]:
                combined_extent = combined_extent.union(layer.extent())
            self.canvas.setExtent(combined_extent)
            self.canvas.refresh()

    def closeEvent(self, event):
        """窗口关闭时释放QGIS资源"""
        self.qgs_app.exitQgis()
        event.accept()
# 矢量编辑子窗口
class VectorEditDialog(QDialog):
    def __init__(self, vector_layer, parent=None):
        super().__init__(parent)
        self.setWindowTitle(f"矢量图层编辑 - {vector_layer.name()}")
        self.setMinimumSize(600, 500)
        self.layer = vector_layer
        self.init_ui()

    def init_ui(self):
        # 标签页：属性编辑、几何编辑、图形参数、符号库
        self.tabs = QTabWidget()
        self.attr_tab = self._create_attribute_tab()
        self.geom_tab = self._create_geometry_tab()
        self.graphic_tab = self._create_graphic_tab()
        self.symbol_tab = self._create_symbol_tab()

        self.tabs.addTab(self.attr_tab, "属性编辑")
        self.tabs.addTab(self.geom_tab, "几何编辑")
        self.tabs.addTab(self.graphic_tab, "图形参数")
        self.tabs.addTab(self.symbol_tab, "符号库编辑")

        # 确认/取消按钮
        self.buttons = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel,
            Qt.Horizontal, self
        )
        self.buttons.accepted.connect(self.apply_edit)
        self.buttons.rejected.connect(self.reject)

        # 布局
        layout = QVBoxLayout()
        layout.addWidget(self.tabs)
        layout.addWidget(self.buttons)
        self.setLayout(layout)

    def _create_attribute_tab(self):
        """属性结构+属性值编辑面板"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # 1. 属性结构编辑（添加字段）
        form_layout = QFormLayout()
        self.field_name = QLineEdit()
        self.field_type = QComboBox()
        self.field_type.addItems(["字符串", "整数", "双精度", "布尔值"])
        self.add_field_btn = QPushButton("添加字段")
        self.add_field_btn.clicked.connect(self._add_field)

        form_layout.addRow("字段名称：", self.field_name)
        form_layout.addRow("字段类型：", self.field_type)
        form_layout.addRow(self.add_field_btn)
        layout.addLayout(form_layout)

        # 2. 属性值编辑（表格显示）
        layout.addWidget(QLabel("属性值编辑（双击单元格修改）："))
        self.attr_table = QTableWidget()
        self._load_attr_table()
        layout.addWidget(self.attr_table)

        return widget

    def _load_attr_table(self):
        """加载属性表格"""
        fields = self.layer.fields()
        features = list(self.layer.getFeatures())
        self.attr_table.setRowCount(len(features))
        self.attr_table.setColumnCount(len(fields) + 1)  # +1 显示要素ID
        self.attr_table.setHorizontalHeaderLabels(["要素ID"] + [f.name() for f in fields])

        for row, feat in enumerate(features):
            # 要素ID
            self.attr_table.setItem(row, 0, QTableWidgetItem(str(feat.id())))
            # 属性值
            for col, field in enumerate(fields):
                val = feat[field.name()]
                self.attr_table.setItem(row, col+1, QTableWidgetItem(str(val) if val else ""))

        self.attr_table.horizontalHeader().setStretchLastSection(True)

    def _add_field(self):
        """添加属性字段（属性结构编辑）"""
        field_name = self.field_name.text().strip()
        if not field_name:
            QMessageBox.warning(self, "警告", "字段名称不能为空！")
            return

        # 字段类型映射
        type_map = {
            "字符串": QVariant.String,
            "整数": QVariant.Int,
            "双精度": QVariant.Double,
            "布尔值": QVariant.Bool
        }
        field_type = type_map[self.field_type.currentText()]
        new_field = QgsField(field_name, field_type)

        if self.layer.dataProvider().addAttributes([new_field]):
            self.layer.updateFields()
            self._load_attr_table()
            QMessageBox.information(self, "成功", f"字段「{field_name}」添加成功！")
            self.field_name.clear()


    def _create_geometry_tab(self):
        """点、线、面几何编辑面板"""
        widget = QWidget()
        layout = QVBoxLayout(widget)

        # 选择要素
        self.feat_id = QSpinBox()
        self.feat_id.setRange(0, max(0, self.layer.featureCount() - 1))
        self.load_geom_btn = QPushButton("加载当前要素几何")
        self.load_geom_btn.clicked.connect(self._load_geometry)

        form = QFormLayout()
        form.addRow("要素ID：", self.feat_id)
        layout.addLayout(form)
        layout.addWidget(self.load_geom_btn)

        # 几何坐标编辑（根据几何类型动态生成）
        self.geom_layout = QFormLayout()
        self.coord_inputs = []  # 存储坐标输入框
        layout.addLayout(self.geom_layout)

        return widget


    def _load_geometry(self):
        """加载选中要素几何（支持 Multi* 类型，仅显示第一个子几何）"""
        feat_id = self.feat_id.value()
        feat = next(self.layer.getFeatures(QgsFeatureRequest(feat_id)), None)
        if not feat:
            return

        while self.geom_layout.rowCount() > 0:
            self.geom_layout.removeRow(0)
        self.coord_inputs.clear()

        geom = feat.geometry()
        if not geom or geom.isEmpty():
            return

        wkb_type = geom.wkbType()
        is_multi = QgsWkbTypes.isMultiType(wkb_type)

        if QgsWkbTypes.geometryType(wkb_type) == QgsWkbTypes.PointGeometry:
            if is_multi:
                pts = geom.asMultiPoint()
                if not pts:
                    return
                pt = pts[0]
            else:
                pt = geom.asPoint()
            self._add_coord_input("X坐标：", pt.x())
            self._add_coord_input("Y坐标：", pt.y())

        elif QgsWkbTypes.geometryType(wkb_type) == QgsWkbTypes.LineGeometry:
            if is_multi:
                lines = geom.asMultiPolyline()
                if not lines:
                    return
                pts = lines[0]
            else:
                pts = geom.asPolyline()
            for i, p in enumerate(pts):
                self._add_coord_input(f"点{i + 1} X：", p.x())
                self._add_coord_input(f"点{i + 1} Y：", p.y())

        elif QgsWkbTypes.geometryType(wkb_type) == QgsWkbTypes.PolygonGeometry:
            if is_multi:
                polys = geom.asMultiPolygon()
                if not polys:
                    return
                ring = polys[0][0] if polys[0] else []
            else:
                poly = geom.asPolygon()
                ring = poly[0] if poly else []
            if not ring:
                return
            if len(ring) > 1 and ring[0] == ring[-1]:
                ring = ring[:-1]
            for i, p in enumerate(ring):
                self._add_coord_input(f"顶点{i + 1} X：", p.x())
                self._add_coord_input(f"顶点{i + 1} Y：", p.y())


    def _add_coord_input(self, label, value):
        """添加坐标输入框"""
        edit = QDoubleSpinBox()
        edit.setRange(-180, 180)
        edit.setDecimals(6)
        edit.setValue(value)
        self.coord_inputs.append(edit)
        self.geom_layout.addRow(label, edit)

    def _create_graphic_tab(self):
        """图形参数编辑（透明度、标注等）"""
        widget = QWidget()
        layout = QFormLayout(widget)

        # 透明度
        self.opacity = QDoubleSpinBox()
        self.opacity.setRange(0.1, 1.0)
        self.opacity.setSingleStep(0.1)
        self.opacity.setValue(self.layer.opacity())
        layout.addRow("图层透明度：", self.opacity)

        # 标注字段选择（修复：避免空字段列表）
        self.label_field = QComboBox()
        field_names = [f.name() for f in self.layer.fields()]
        if field_names:  # 只有存在字段时才添加
            self.label_field.addItems(field_names)
        else:
            self.label_field.addItem("无字段")
            self.label_field.setEnabled(False)  # 无字段时禁用
        layout.addRow("标注字段：", self.label_field)

        # 标注字体大小
        self.label_size = QSpinBox()
        self.label_size.setRange(6, 20)
        self.label_size.setValue(10)
        layout.addRow("标注字体大小：", self.label_size)

        return widget

    def _create_symbol_tab(self):
        """符号库编辑（点/线/面符号+颜色表）"""
        widget = QWidget()
        vlayout = QVBoxLayout(widget)

        form = QFormLayout()

        # 几何对应的基础参数
        self.symbol_type = QComboBox()
        geom_type = self.layer.geometryType()
        if geom_type == QgsWkbTypes.PointGeometry:
            self.symbol_type.addItems(["圆形", "方形", "三角形", "图片标记"])
            self.symbol_size = QDoubleSpinBox()
            self.symbol_size.setRange(1, 20)
            self.symbol_size.setValue(8)
            form.addRow("符号大小：", self.symbol_size)
        elif geom_type == QgsWkbTypes.LineGeometry:
            self.symbol_type.addItems(["实线", "虚线", "点划线"])
            self.line_width = QDoubleSpinBox()
            self.line_width.setRange(0.5, 5.0)
            self.line_width.setValue(2.0)
            form.addRow("线宽：", self.line_width)
        else:
            self.symbol_type.addItems(["实心", "渐变", "图案填充"])
            self.outline_width = QDoubleSpinBox()
            self.outline_width.setRange(0.1, 3.0)
            self.outline_width.setValue(0.5)
            form.addRow("轮廓宽度：", self.outline_width)

        form.addRow("符号类型：", self.symbol_type)

        # 颜色选择
        self.color_btn = QPushButton("选择颜色")
        self.color_btn.setStyleSheet("background-color: red")
        self.color_btn.clicked.connect(self._choose_color)
        self.current_color = QColor("red")
        form.addRow("主色：", self.color_btn)

        # 颜色表模式
        self.color_ramp = QComboBox()
        self.color_ramp.addItems(["单色系", "渐变色", "分类色"])
        form.addRow("颜色表：", self.color_ramp)

        # 渐变/分类所需字段
        field_names = [f.name() for f in self.layer.fields()]
        num_field_names = [f.name() for f in self.layer.fields()
                           if f.type() in (QVariant.Int, QVariant.Double)]

        self.grad_field = QComboBox()
        self.grad_field.addItems(num_field_names if num_field_names else ["无可用数值字段"])
        self.grad_field.setEnabled(bool(num_field_names))
        form.addRow("渐变字段：", self.grad_field)

        self.grad_classes = QSpinBox()
        self.grad_classes.setRange(2, 9)
        self.grad_classes.setValue(5)
        form.addRow("分级数：", self.grad_classes)

        self.cat_field = QComboBox()
        self.cat_field.addItems(field_names if field_names else ["无字段"])
        self.cat_field.setEnabled(bool(field_names))
        form.addRow("分类字段：", self.cat_field)

        # 根据模式控制可编辑项
        self.color_ramp.currentTextChanged.connect(self._update_symbol_tab_controls)
        self._update_symbol_tab_controls(self.color_ramp.currentText())

        vlayout.addLayout(form)
        return widget

    def _update_symbol_tab_controls(self, mode: str):
        """根据颜色表模式启用/禁用控件"""
        if mode == "单色系":
            self.grad_field.setEnabled(False)
            self.grad_classes.setEnabled(False)
            self.cat_field.setEnabled(False)
        elif mode == "渐变色":
            self.grad_field.setEnabled(
                True and self.grad_field.count() > 0 and self.grad_field.currentText() != "无可用数值字段")
            self.grad_classes.setEnabled(True)
            self.cat_field.setEnabled(False)
        else:  # 分类色
            self.grad_field.setEnabled(False)
            self.grad_classes.setEnabled(False)
            self.cat_field.setEnabled(True and self.cat_field.count() > 0 and self.cat_field.currentText() != "无字段")

    def _choose_color(self):
        """选择符号颜色"""
        color = QColorDialog.getColor(self.current_color, self, "选择符号颜色")
        if color.isValid():
            self.current_color = color
            self.color_btn.setStyleSheet(f"background-color: {color.name()}")


    def apply_edit(self):
            """应用所有编辑"""
            if not self.layer.startEditing():
                QMessageBox.warning(self, "失败", "无法开启编辑模式！")
                return
            try:
                self._apply_attribute_edit()  # 属性
                self._apply_geometry_edit()  # 几何
                self._apply_graphic_edit()  # 图形参数
                self._apply_symbol_edit()  # 符号库
                if self.layer.commitChanges():
                    QMessageBox.information(self, "成功", "所有编辑已应用！")
                    self.accept()
                else:
                    raise Exception(f"提交失败：{self.layer.commitErrors()}")
            except Exception as e:
                self.layer.rollBack()
                QMessageBox.warning(self, "失败", f"编辑失败：{str(e)}")

    def _apply_attribute_edit(self):
            """应用属性编辑（字段+值）"""
            fields = self.layer.fields()
            for row in range(self.attr_table.rowCount()):
                feat_id = int(self.attr_table.item(row, 0).text())
                for col in range(1, self.attr_table.columnCount()):
                    cell = self.attr_table.item(row, col)
                    new_val = cell.text() if cell else ""
                    field_type = fields[col - 1].type()
                    if field_type == QVariant.Int:
                        new_val = int(new_val) if new_val else 0
                    elif field_type == QVariant.Double:
                        new_val = float(new_val) if new_val else 0.0
                    elif field_type == QVariant.Bool:
                        new_val = str(new_val).lower() == "true"
                    self.layer.changeAttributeValue(feat_id, col - 1, new_val)

    def _apply_geometry_edit(self):
        """应用几何编辑（支持 Multi*，只替换第一个子几何）"""
        if not getattr(self, "coord_inputs", None) or len(self.coord_inputs) == 0:
            return
        feat_id = self.feat_id.value()
        feat = next(self.layer.getFeatures(QgsFeatureRequest(feat_id)), None)
        if not feat:
            return

        geom = feat.geometry()
        wkb_type = geom.wkbType()
        is_multi = QgsWkbTypes.isMultiType(wkb_type)
        gtype = QgsWkbTypes.geometryType(wkb_type)
        nums = [e.value() for e in self.coord_inputs]

        if gtype == QgsWkbTypes.PointGeometry:
            if len(nums) < 2:
                return
            new_pt = QgsPointXY(nums[0], nums[1])
            if is_multi:
                multi = geom.asMultiPoint()
                if not multi:
                    multi = []
                if multi:
                    multi[0] = new_pt
                else:
                    multi.append(new_pt)
                new_geom = QgsGeometry.fromMultiPointXY(multi)
            else:
                new_geom = QgsGeometry.fromPointXY(new_pt)

        elif gtype == QgsWkbTypes.LineGeometry:
            if len(nums) < 4 or len(nums) % 2 != 0:
                return
            pts = [QgsPointXY(nums[i], nums[i + 1]) for i in range(0, len(nums), 2)]
            if is_multi:
                multi = geom.asMultiPolyline()
                if not multi:
                    multi = []
                if multi:
                    multi[0] = pts
                else:
                    multi.append(pts)
                new_geom = QgsGeometry.fromMultiPolylineXY(multi)
            else:
                new_geom = QgsGeometry.fromPolylineXY(pts)

        elif gtype == QgsWkbTypes.PolygonGeometry:
            if len(nums) < 6 or len(nums) % 2 != 0:
                return
            ring = [QgsPointXY(nums[i], nums[i + 1]) for i in range(0, len(nums), 2)]
            if ring[0] != ring[-1]:
                ring.append(ring[0])
            if is_multi:
                multi = geom.asMultiPolygon()
                if not multi:
                    multi = []
                shell = [ring]
                if multi:
                    multi[0] = shell
                else:
                    multi.append(shell)
                new_geom = QgsGeometry.fromMultiPolygonXY(multi)
            else:
                new_geom = QgsGeometry.fromPolygonXY([ring])
        else:
            return

        self.layer.changeGeometry(feat_id, new_geom)

    def _apply_symbol_edit(self):
        """应用符号库样式（单色 / 分类 / 渐变）"""
        try:
            geom_type = self.layer.geometryType()
            mode = self.color_ramp.currentText()
            base_color = self.current_color

            # 构造基础符号
            def make_symbol(color: QColor):
                hex_color = color.name()
                if geom_type == QgsWkbTypes.PointGeometry:
                    return QgsMarkerSymbol.createSimple({
                        "name": self._get_marker_shape(),
                        "color": hex_color,
                        "size": str(self.symbol_size.value())
                    })
                elif geom_type == QgsWkbTypes.LineGeometry:
                    return QgsLineSymbol.createSimple({
                        "color": hex_color,
                        "line_style": self._get_line_style(),
                        "width": str(self.line_width.value())
                    })
                else:
                    return QgsFillSymbol.createSimple({
                        "color": hex_color,
                        "style": self._get_fill_style(),
                        "outline_color": "#000000",
                        "outline_width": str(self.outline_width.value())
                    })

            # 单色
            if mode == "单色系":
                renderer = QgsSingleSymbolRenderer(make_symbol(base_color))

            # 分类
            elif mode == "分类色":
                field = self.cat_field.currentText()
                if not field or field == "无字段":
                    return
                idx = self.layer.fields().indexFromName(field)
                if idx < 0:
                    return
                uniques = list(self.layer.uniqueValues(idx))
                categories = []
                for i, val in enumerate(uniques):
                    c = QColor.fromHsv((i * 37) % 360, 200, 235)
                    categories.append(QgsRendererCategory(val, make_symbol(c), str(val)))
                renderer = QgsCategorizedSymbolRenderer(field, categories)

            # 渐变
            else:
                field = self.grad_field.currentText()
                if not field or field == "无可用数值字段":
                    return
                idx = self.layer.fields().indexFromName(field)
                if idx < 0:
                    return
                ftype = self.layer.fields()[idx].type()
                if ftype not in (QVariant.Int, QVariant.Double):
                    return
                min_v = self.layer.minimumValue(idx)
                max_v = self.layer.maximumValue(idx)
                if min_v is None or max_v is None or min_v == max_v:
                    return
                classes = self.grad_classes.value()
                ramp = QgsGradientColorRamp(QColor("#ffffff"), base_color)
                step = (max_v - min_v) / classes
                ranges = []
                for i in range(classes):
                    lower = float(min_v + i * step)
                    upper = float(max_v if i == classes - 1 else min_v + (i + 1) * step)
                    t = (i + 0.5) / classes
                    c = ramp.color(t)
                    ranges.append(QgsRendererRange(lower, upper, make_symbol(c), f"{lower:.2f}-{upper:.2f}"))
                renderer = QgsGraduatedSymbolRenderer(field, ranges)
                renderer.setSourceColorRamp(ramp)
                renderer.setMode(QgsGraduatedSymbolRenderer.EqualInterval)

            self.layer.setRenderer(renderer)
            self.layer.triggerRepaint()
        except Exception as e:
            QMessageBox.warning(self, "错误", f"符号库编辑失败：{e}")


    def _apply_graphic_edit(self):
        """应用图形参数（透明度、标注）"""
        try:
            self.layer.setOpacity(self.opacity.value())

            if self.label_field.count() == 0 or self.label_field.currentText() == "无字段":
                self.layer.setLabelsEnabled(False)
                return

            label_field = self.label_field.currentText()
            fields = self.layer.fields()
            if fields.isEmpty() or fields.indexFromName(label_field) == -1:
                self.layer.setLabelsEnabled(False)
                return

            label_settings = QgsPalLayerSettings()
            label_settings.fieldName = label_field
            label_settings.scaleVisibility = False  # 可保留或删除

            text_format = QgsTextFormat()
            text_format.setSize(self.label_size.value())
            # 修改: 使用 setFormat 代替 setTextFormat
            label_settings.setFormat(text_format)

            self.layer.setLabeling(QgsVectorLayerSimpleLabeling(label_settings))
            self.layer.setLabelsEnabled(True)
        except Exception as e:
            QMessageBox.warning(self, "错误", f"图形参数编辑失败：{str(e)}")


    def _get_marker_shape(self):
            """映射点符号形状（QgsMarkerSymbol.createSimple 的 name 值）"""
            shape_map = {
                "圆形": "circle",
                "方形": "square",
                "三角形": "triangle",
                "图片标记": "circle"  # 占位：未实现图片标记，退化为圆形
            }
            return shape_map.get(self.symbol_type.currentText(), "circle")

    def _get_line_style(self):
            """映射线样式（createSimple 的 line_style 值）"""
            style_map = {
                "实线": "solid",
                "虚线": "dash",
                "点划线": "dash dot"
            }
            return style_map.get(self.symbol_type.currentText(), "solid")

    def _get_fill_style(self):
            """映射面填充样式（createSimple 的 style 值）"""
            style_map = {
                "实心": "solid",
                "渐变": "solid",  # 基础符号仍用实心；渐变由“渐变色”渲染器控制
                "图案填充": "dense4"
            }
            return style_map.get(self.symbol_type.currentText(), "solid")


# ---------------------- 程序入口 ----------------------
if __name__ == "__main__":
    # 初始化PyQt应用
    app = QApplication(sys.argv)

    # 创建并显示主窗口
    window = QGISFileLoader()
    window.show()

    # 运行应用
    exit_code = app.exec_()
    sys.exit(exit_code)