from flask import Flask, request, jsonify
from flask_cors import CORS
from flask_restful import Api, Resource
from flask_sqlalchemy import SQLAlchemy
from werkzeug.security import generate_password_hash, check_password_hash
import os
import datetime
import jwt
import pandas as pd
import json

app = Flask(__name__)
CORS(app)

# 数据库配置
basedir = os.path.abspath(os.path.dirname(__file__))
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + os.path.join(basedir, 'app.db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config['SECRET_KEY'] = 'your-secret-key'  # 用于JWT令牌签名

# 初始化数据库
db = SQLAlchemy(app)
api = Api(app)

# 定义用户模型
class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    username = db.Column(db.String(80), unique=True, nullable=False)
    password = db.Column(db.String(120), nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    created_at = db.Column(db.DateTime, default=datetime.datetime.utcnow)

    def __repr__(self):
        return f'<User {self.username}>'

    def to_dict(self):
        return {
            'id': self.id,
            'username': self.username,
            'email': self.email,
            'created_at': self.created_at.strftime('%Y-%m-%d %H:%M:%S')
        }

# 定义台风数据模型
class TyphoonData(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    sample_id = db.Column(db.Integer, nullable=False)
    track_type = db.Column(db.String(20), nullable=False)  # 轨迹类型：历史、预测、实际
    step = db.Column(db.Integer, nullable=False)
    longitude = db.Column(db.Float, nullable=False)
    latitude = db.Column(db.Float, nullable=False)
    level = db.Column(db.Float, nullable=False)
    speed = db.Column(db.Float, nullable=False)
    pressure = db.Column(db.Float, nullable=False)
    month = db.Column(db.Float)
    date = db.Column(db.Float)
    rainfall = db.Column(db.Float)
    
    def to_dict(self):
        return {
            'id': self.id,
            'sample_id': self.sample_id,
            'track_type': self.track_type,
            'step': self.step,
            'longitude': self.longitude,
            'latitude': self.latitude,
            'level': self.level,
            'speed': self.speed,
            'pressure': self.pressure,
            'month': self.month,
            'date': self.date,
            'rainfall': self.rainfall
        }

@app.route('/', methods=["GET"])
def index():
    return "Welcome to API v1, try /hello or /auth/register."


class Hello(Resource):
    @staticmethod
    def get():
        return "[get] hello flask"

    @staticmethod
    def post():
        return "[post] hello flask"


# 注册API
@app.route('/auth/register', methods=['POST'])
def register():
    data = request.get_json()
    
    # 验证输入数据
    if not data or not data.get('username') or not data.get('password') or not data.get('email'):
        return jsonify({'message': '请提供用户名、密码和邮箱'}), 400
    
    # 检查用户名是否已存在
    if User.query.filter_by(username=data['username']).first():
        return jsonify({'message': '用户名已存在'}), 409
    
    # 检查邮箱是否已存在
    if User.query.filter_by(email=data['email']).first():
        return jsonify({'message': '邮箱已被注册'}), 409
    
    # 创建新用户
    hashed_password = generate_password_hash(data['password'], method='pbkdf2:sha256')
    new_user = User(
        username=data['username'],
        password=hashed_password,
        email=data['email']
    )
    
    db.session.add(new_user)
    db.session.commit()
    
    return jsonify({'message': '注册成功'}), 201

# 登录API
@app.route('/auth/login', methods=['POST'])
def login():
    data = request.get_json()
    
    # 验证输入数据
    if not data or not data.get('username') or not data.get('password'):
        return jsonify({'message': '请提供用户名和密码'}), 400
    
    # 查找用户
    user = User.query.filter_by(username=data['username']).first()
    
    # 验证密码
    if not user or not check_password_hash(user.password, data['password']):
        return jsonify({'message': '用户名或密码不正确'}), 401
    
    # 生成JWT令牌
    token = jwt.encode({
        'user_id': user.id,
        'username': user.username,
        'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=24)
    }, app.config['SECRET_KEY'])
    
    return jsonify({
        'message': '登录成功',
        'token': token,
        'user': user.to_dict()
    }), 200

# 获取用户信息API（需要验证令牌）
@app.route('/auth/me', methods=['GET'])
def get_user():
    auth_header = request.headers.get('Authorization')
    
    if not auth_header or not auth_header.startswith('Bearer '):
        return jsonify({'message': '缺少授权令牌'}), 401
    
    try:
        token = auth_header.split(' ')[1]
        payload = jwt.decode(token, app.config['SECRET_KEY'], algorithms=['HS256'])
        user = User.query.filter_by(id=payload['user_id']).first()
        
        if not user:
            return jsonify({'message': '用户不存在'}), 404
        
        return jsonify(user.to_dict()), 200
    except jwt.ExpiredSignatureError:
        return jsonify({'message': '令牌已过期'}), 401
    except jwt.InvalidTokenError:
        return jsonify({'message': '无效的令牌'}), 401

# 读取CSV文件数据并导入数据库
def import_typhoon_data():
    # 检查数据是否已经导入
    if TyphoonData.query.first() is not None:
        print("台风数据已存在，跳过导入")
        return
    
    # 获取数据文件夹路径 - 首先尝试根目录下的data文件夹
    root_data_dir = os.path.join(os.path.dirname(basedir), 'data')
    # 也尝试后端目录内的data文件夹
    back_data_dir = os.path.join(basedir, 'data')
    
    # 首先尝试从根目录下的data文件夹读取数据
    data_imported = False
    if os.path.exists(root_data_dir):
        data_imported = try_import_from_dir(root_data_dir)
        
    # 如果根目录读取失败，尝试从后端目录内的data文件夹读取
    if not data_imported and os.path.exists(back_data_dir):
        data_imported = try_import_from_dir(back_data_dir)
        
    if data_imported:
        print("台风数据导入完成！")
    else:
        print("警告: 未找到台风数据文件或导入失败！")

# 从指定目录导入台风数据
def try_import_from_dir(data_dir):
    print(f"尝试从 {data_dir} 导入台风数据...")
    imported_files = 0
    
    # 读取所有CSV文件并导入数据库
    for i in range(1, 11):
        csv_file = os.path.join(data_dir, f'typhoon_prediction_sample_{i}.csv')
        if os.path.exists(csv_file):
            try:
                df = pd.read_csv(csv_file)
                for _, row in df.iterrows():
                    # 将中文轨迹类型转换为英文，便于前端处理
                    track_type = "history"
                    if row['轨迹类型'] == "预测轨迹":
                        track_type = "prediction"
                    elif row['轨迹类型'] == "实际轨迹":
                        track_type = "actual"
                    elif row['轨迹类型'] == "历史轨迹":
                        track_type = "history"
                    
                    # 创建台风数据记录
                    typhoon_data = TyphoonData(
                        sample_id=int(row['样本编号']),
                        track_type=track_type,
                        step=int(row['步骤编号']),
                        longitude=float(row['经度']),
                        latitude=float(row['纬度']),
                        level=float(row['台风等级']),
                        speed=float(row['风速']),
                        pressure=float(row['气压']),
                        month=float(row['月份']) if not pd.isna(row['月份']) else 0,
                        date=float(row['日期']) if not pd.isna(row['日期']) else 0,
                        rainfall=float(row['降水量']) if not pd.isna(row['降水量']) else 0
                    )
                    db.session.add(typhoon_data)
                
                # 每个文件提交一次，减少数据库压力
                db.session.commit()
                print(f"已导入文件: {csv_file}")
                imported_files += 1
            except Exception as e:
                print(f"导入文件 {csv_file} 时出错: {str(e)}")
                continue
    
    return imported_files > 0

# 获取台风样本列表
@app.route('/api/typhoon/samples', methods=['GET'])
def get_typhoon_samples():
    # 获取所有不同的样本ID
    sample_ids = db.session.query(TyphoonData.sample_id).distinct().all()
    samples = [{"id": id[0], "name": f"台风样本 {id[0]}"} for id in sample_ids]
    return jsonify(samples)

# 获取特定台风样本的数据
@app.route('/api/typhoon/data/<int:sample_id>', methods=['GET'])
def get_typhoon_data(sample_id):
    # 获取指定样本ID的所有数据
    typhoon_data = TyphoonData.query.filter_by(sample_id=sample_id).all()
    if not typhoon_data:
        return jsonify({"message": "未找到该样本数据"}), 404
    
    # 将数据转换为不同轨迹类型的路径
    paths = {
        "history": [],
        "prediction": [],
        "actual": []
    }
    
    # 确保按步骤排序
    typhoon_data.sort(key=lambda x: x.step)
    
    for data in typhoon_data:
        path = {
            "step": data.step,
            "longitude": data.longitude,
            "latitude": data.latitude,
            "level": data.level,
            "speed": data.speed,
            "pressure": data.pressure,
            "month": data.month,
            "date": data.date,
            "rainfall": data.rainfall
        }
        paths[data.track_type].append(path)
    
    return jsonify({
        "sample_id": sample_id,
        "paths": paths
    })

# 添加调试API端点
@app.route('/api/debug/typhoon/count', methods=['GET'])
def get_typhoon_count():
    """获取数据库中台风记录的数量，用于调试"""
    try:
        total_records = TyphoonData.query.count()
        distinct_samples = db.session.query(TyphoonData.sample_id).distinct().count()
        record_by_sample = {}
        
        sample_ids = db.session.query(TyphoonData.sample_id).distinct().all()
        for id_tuple in sample_ids:
            sample_id = id_tuple[0]
            count = TyphoonData.query.filter_by(sample_id=sample_id).count()
            record_by_sample[f"sample_{sample_id}"] = count
            
        return jsonify({
            "status": "success",
            "total_records": total_records,
            "distinct_samples": distinct_samples,
            "records_by_sample": record_by_sample
        })
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": str(e)
        }), 500

@app.route('/api/debug/typhoon/data_paths', methods=['GET'])
def get_data_paths():
    """获取数据文件路径信息，用于调试"""
    root_data_dir = os.path.join(os.path.dirname(basedir), 'data')
    back_data_dir = os.path.join(basedir, 'data')
    
    paths = {
        "basedir": basedir,
        "root_data_dir": {
            "path": root_data_dir,
            "exists": os.path.exists(root_data_dir),
            "files": []
        },
        "back_data_dir": {
            "path": back_data_dir,
            "exists": os.path.exists(back_data_dir),
            "files": []
        }
    }
    
    # 检查根目录下的数据文件
    if paths["root_data_dir"]["exists"]:
        for i in range(1, 11):
            file_path = os.path.join(root_data_dir, f'typhoon_prediction_sample_{i}.csv')
            paths["root_data_dir"]["files"].append({
                "file": f'typhoon_prediction_sample_{i}.csv',
                "exists": os.path.exists(file_path)
            })
    
    # 检查后端目录下的数据文件
    if paths["back_data_dir"]["exists"]:
        for i in range(1, 11):
            file_path = os.path.join(back_data_dir, f'typhoon_prediction_sample_{i}.csv')
            paths["back_data_dir"]["files"].append({
                "file": f'typhoon_prediction_sample_{i}.csv',
                "exists": os.path.exists(file_path)
            })
    
    return jsonify(paths)

@app.route('/api/debug/typhoon/sample_data/<int:sample_id>', methods=['GET'])
def get_sample_debug_data(sample_id):
    """获取指定样本的原始数据，用于调试"""
    try:
        data = TyphoonData.query.filter_by(sample_id=sample_id).all()
        if not data:
            return jsonify({"status": "error", "message": f"未找到样本ID为 {sample_id} 的数据"}), 404
            
        result = []
        for item in data:
            result.append(item.to_dict())
            
        return jsonify({
            "status": "success",
            "sample_id": sample_id,
            "count": len(result),
            "data": result
        })
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": str(e)
        }), 500

# 重新导入数据的API端点
@app.route('/api/debug/typhoon/reload_data', methods=['POST'])
def reload_typhoon_data():
    """强制重新导入台风数据，用于调试"""
    try:
        # 删除所有现有台风数据
        TyphoonData.query.delete()
        db.session.commit()
        
        # 重新导入数据
        import_typhoon_data()
        
        # 返回新导入的数据统计
        total_records = TyphoonData.query.count()
        distinct_samples = db.session.query(TyphoonData.sample_id).distinct().count()
        
        return jsonify({
            "status": "success",
            "message": "台风数据已重新导入",
            "total_records": total_records,
            "distinct_samples": distinct_samples
        })
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": f"重新导入数据时出错: {str(e)}"
        }), 500

api.add_resource(Hello, '/hello')

# 创建数据库表并导入数据
@app.before_first_request
def setup():
    db.create_all()
    import_typhoon_data()

if __name__ == "__main__":
    app.run(host='127.0.0.1', port=8010)
