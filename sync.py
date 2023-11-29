import pymongo
import json
import serial
import time
from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi
from pymongo import ssl_support

# MongoDB Atlas Connection
uri = "mongodb+srv://jpnas:iHBXPcSA5a3Rxph9@cluster0.482qyxh.mongodb.net/?retryWrites=true&w=majority"
database_name = "card_games"

# Serial Connection
serial_port = '/dev/cu.usbmodem21301' # Substitua pelo seu dispositivo serial
baud_rate = 9600  # Mesma taxa de baud do código Arduino

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

    data = list(collection.find({}, {'_id': 0}))  # Busca todos os documentos, excluindo o campo '_id'
    return data

def send_data(serial_connection, data):
    for game in data:
        serial_connection.write(f'{game["name"]} {game["minPlayers"]} {game["maxPlayers"]}'.encode('utf-8'))


def main():
    games_data = fetch_data()

    # Converte os dados para um formato adequado, se necessário
    # Por exemplo, uma simplificação ou serialização específica

    with serial.Serial(serial_port, baud_rate) as ser:
        time.sleep(2)  # Aguarda a conexão serial se estabilizar
        send_data(ser, games_data)
        print("Dados enviados com sucesso para o Arduino.")

if __name__ == "__main__":
    main()
