import pymongo
import json
import serial
import time
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi
from pymongo import ssl_support

uri = "mongodb+srv://jpnas:iHBXPcSA5a3Rxph9@cluster0.482qyxh.mongodb.net/?retryWrites=true&w=majority"
database_name = "card_games"

serial_port = '/dev/cu.usbmodem101'
baud_rate = 9600

def fetch_data():
    client = MongoClient(uri,
                     server_api=ServerApi('1'),
                     tls=ssl_support.HAVE_SSL,
                     tlsAllowInvalidCertificates=True)
    try:
        client.admin.command('ping')
        print("Pinged your deployment. You successfully connected to MongoDB!")
    except Exception as e:
        print(e)

    db = client["card_games"]
    db = client[database_name]
    collection = db.games

    data = list(collection.find({}, {'_id': 0}))
    return data

def send_data(serial, game):
    serial.write(f"game {game['name']}\n".encode('utf-8'))

    serial.write(f"minPlayers {game['minPlayers']}\n".encode('utf-8'))

    serial.write(f"maxPlayers {game['maxPlayers']}\n".encode('utf-8'))

    serial.write("instructions\n".encode('utf-8'))

    for instruction in game['instructions']:
        serial.write(f"{instruction['type']} {instruction['cardAmount']}\n".encode('utf-8'))


    serial.write("endinstructions\n".encode('utf-8'))

    if 'excludedCards' in game:
        excluded_cards_str = ' '.join(map(str, game['excludedCards']))
        serial.write(f"excludedCards {excluded_cards_str}\n".encode('utf-8'))


    serial.write("endgame\n".encode('utf-8'))

def main():
    games_data = fetch_data()

    with serial.Serial(serial_port, baud_rate) as ser:
        time.sleep(2)
        for game in games_data:
            send_data(ser, game)
            serial.write("cambiodesligo".encode('utf-8'))
            
        print("Dados enviados com sucesso para o Arduino.")

if __name__ == "__main__":
    main()