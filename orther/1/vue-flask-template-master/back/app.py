import json
import os
import uuid
from functools import wraps
from flask import Flask, request, jsonify
from flask_cors import CORS
from werkzeug.security import generate_password_hash, check_password_hash

DATA_FILE = os.path.join(os.path.dirname(__file__), 'data.json')

def load_data():
    if not os.path.exists(DATA_FILE):
        return {'users': {}, 'tokens': {}, 'favorites': {}}
    with open(DATA_FILE, 'r', encoding='utf-8') as f:
        return json.load(f)

def save_data(d):
    with open(DATA_FILE, 'w', encoding='utf-8') as f:
        json.dump(d, f, ensure_ascii=False, indent=2)

app = Flask(__name__)
CORS(app)


def require_token(f):
    @wraps(f)
    def wrapper(*args, **kwargs):
        auth = request.headers.get('Authorization', '')
        if auth.startswith('Bearer '):
            token = auth.split(' ', 1)[1]
        else:
            token = request.args.get('token')
        data = load_data()
        username = data['tokens'].get(token)
        if not username:
            return jsonify({'error': 'unauthorized'}), 401
        request.username = username
        return f(*args, **kwargs)
    return wrapper


@app.route('/')
def index():
    return "Welcome to API v1"


@app.route('/api/register', methods=['POST'])
def register():
    req = request.get_json() or {}
    username = req.get('username', '').strip()
    password = req.get('password', '')
    if not username or not password:
        return jsonify({'error': 'username and password required'}), 400
    data = load_data()
    if username in data['users']:
        return jsonify({'error': 'user exists'}), 400
    data['users'][username] = generate_password_hash(password)
    save_data(data)
    return jsonify({'ok': True})


@app.route('/api/login', methods=['POST'])
def login():
    req = request.get_json() or {}
    username = req.get('username', '').strip()
    password = req.get('password', '')
    data = load_data()
    pw_hash = data['users'].get(username)
    if not pw_hash or not check_password_hash(pw_hash, password):
        return jsonify({'error': 'invalid username or password'}), 400
    token = str(uuid.uuid4())
    data['tokens'][token] = username
    save_data(data)
    return jsonify({'token': token, 'username': username})


@app.route('/api/profile')
@require_token
def profile():
    return jsonify({'username': request.username})


@app.route('/api/favorites', methods=['GET', 'POST'])
@require_token
def favorites():
    data = load_data()
    user = request.username
    user_favs = data['favorites'].setdefault(user, [])
    if request.method == 'GET':
        return jsonify({'favorites': user_favs})
    # POST: add favorite
    body = request.get_json() or {}
    name = body.get('name')
    lat = body.get('lat')
    lng = body.get('lng')
    if not name or lat is None or lng is None:
        return jsonify({'error': 'name, lat, lng required'}), 400
    item = {'id': str(uuid.uuid4()), 'name': name, 'lat': lat, 'lng': lng}
    user_favs.append(item)
    data['favorites'][user] = user_favs
    save_data(data)
    return jsonify({'ok': True, 'item': item})


@app.route('/api/favorites/<fav_id>', methods=['DELETE'])
@require_token
def fav_delete(fav_id):
    data = load_data()
    user = request.username
    user_favs = data['favorites'].get(user, [])
    new = [f for f in user_favs if f.get('id') != fav_id]
    data['favorites'][user] = new
    save_data(data)
    return jsonify({'ok': True})


if __name__ == '__main__':
    # ensure data file exists
    d = load_data()
    save_data(d)
    app.run(host='127.0.0.1', port=8010, debug=True)
