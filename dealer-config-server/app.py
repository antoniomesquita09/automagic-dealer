from flask import Flask, jsonify, request
from flask_cors import CORS
import uuid
from games.models import CardGame

app = Flask(__name__)
CORS(app, resources={r"/*": {"origins": "*"}})

@app.route('/games', methods=['GET'])
def get_games():
    games = CardGame.get_all()
    return jsonify(games)

@app.route('/games', methods=['POST'])
def create_game():
    game_data = request.json
    game_data['id'] = str(uuid.uuid4())
    CardGame.create(game_data)
    return jsonify({"message": "Game created", "id": game_data['id']}), 201

@app.route('/games/<string:game_id>', methods=['PUT'])
def update_game(game_id):
    game_data = request.json
    CardGame.update(game_id, game_data)
    return jsonify({"message": "Game updated"})

@app.route('/games/<string:game_id>', methods=['DELETE'])
def delete_game(game_id):
    CardGame.delete(game_id)
    return jsonify({"message": "Game deleted"})

if __name__ == '__main__':
    app.run(debug=True)