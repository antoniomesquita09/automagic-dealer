from db import get_db

class CardGame:
    # @staticmethod
    def create(game_data):
        db = get_db()
        db.games.insert_one(game_data)

    # @staticmethod
    def get_all():
        db = get_db()
        return list(db.games.find({}, {'_id': 0}))

    # @staticmethod
    def get_by_id(game_id):
        db = get_db()
        return db.games.find_one({"id": game_id}, {'_id': 0})

    # @staticmethod
    def update(game_id, game_data):
        db = get_db()
        db.games.update_one({"id": game_id}, {"$set": game_data})

    # @staticmethod
    def delete(game_id):
        db = get_db()
        db.games.delete_one({"id": game_id})
