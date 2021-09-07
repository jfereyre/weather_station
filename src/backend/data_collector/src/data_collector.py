from paho.mqtt import client as mqtt_client
from data_store import DataStore
import json


g_broker = "localhost"
g_broker_port = 1883
g_broker_client_id = "data_collector"
g_username = 'public'
g_password = 'public'

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    l_client = mqtt_client.Client(g_broker_client_id)
    l_client.username_pw_set(g_username, g_password)
    l_client.on_connect = on_connect
    l_client.connect(g_broker, g_broker_port)
    return l_client

def subscribe(a_client: mqtt_client):
    def on_message(a_client, a_userdata, a_msg):
        print(f"Received `{a_msg.payload.decode()}` from `{a_msg.topic}` topic")

        DataStore("log.json").store(json.loads(a_msg.payload.decode()), a_msg.topic.split('/')[1])
        print(json.dumps(DataStore("log.json").getCategoryData("wind_gauge")))

    a_client.subscribe("weather_data/rain_gauge")
    a_client.subscribe("weather_data/wind_gauge")
    a_client.on_message = on_message

def run():
    l_client = connect_mqtt()
    subscribe(l_client)
    l_client.loop_forever()

if __name__ == '__main__':
    run()    