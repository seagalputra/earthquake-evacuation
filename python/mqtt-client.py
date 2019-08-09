import paho.mqtt.client as mqtt

# Global variable
# Berisi konstanta yang digunakan untuk konfigurasi MQTT
url_broker = "soldier.cloudmqtt.com"
port = 18162
username = "mizrsriv"
password = "vKGsX3BGaJqa"

def skala(x):
    if 0 >= x <= 3:
        kecil = 1
    elif 3 < x < 4:
        kecil = (4-x)/(4-3)
    else:
        kecil = 0

    if 3 < x < 4:
        sedang = (x-3)/(4-3)
    elif 4 <= x <= 6:
        sedang = 1
    elif 6 < x <= 7:
        sedang = (7-x)/(7-6)
    else:
        sedang = 0

    if 6 < x < 7:
        besar = (x-6)/(7-6)
    elif 7 <= x <= 9:
        besar = 1
    else:
        besar = 0

    return (kecil, sedang, besar)

def lantai(y):
    if 1 <= y <= 3:
        rendah = 1
    elif 3 < y <= 4: 
        rendah = (4-y)/(4-3)
    else:
        rendah = 0

    if 3 < y < 4:
        agak_rendah = (y-3)/(4-3)
    elif 4 <= y <= 5:
        agak_rendah = 1
    elif 5 < y <= 6:
        agak_rendah = (6-y)/(6-5)
    else:
        agak_rendah = 0

    if 5 < y < 6:
        agak_tinggi = (y-5)/(6-5)
    elif 6 <= y <= 7:
        agak_tinggi = 1
    elif 7 < y <= 8:
        agak_tinggi = (8-y)/(8-7)
    else:
        agak_tinggi = 0

    if 7 < y < 8:
        tinggi = (y-7)/(8-7)
    elif y >= 8:
        tinggi = 1
    else:
        tinggi = 0

    return (rendah, agak_rendah, agak_tinggi, tinggi)

def rule_base(skala, lantai):
    x_kecil, x_sedang, x_besar = skala
    y_rendah, y_agakrendah, y_agaktinggi, y_tinggi = lantai
    s1 = min(x_kecil, y_rendah)
    s2 = min(x_kecil, y_agakrendah)
    s3 = min(x_kecil, y_agaktinggi)
    s4 = min(x_sedang, y_rendah)
    s5 = min(x_sedang, y_agakrendah)
    w1 = min(x_kecil, y_tinggi)
    w2 = min(x_sedang, y_agaktinggi)
    w3 = min(x_sedang, y_tinggi)
    w4 = min(x_besar, y_rendah)
    a1 = min(x_besar, y_agakrendah)
    a2 = min(x_besar, y_agaktinggi)
    a3 = min(x_besar, y_tinggi)
    return max(s1,s2,s3,s4,s5), max(w1,w2,w3,w4), max(a1,a2,a3)

def defuzzy(rule):
    s,w,a = rule
    alert = (s*30)+(w*50)+(a*70) / (s + w + a)
    return alert

def fuzzy_system(x, y):
    fuzzy_skala = skala(x)
    fuzzy_lantai = lantai(y)
    val = rule_base(fuzzy_skala, fuzzy_lantai)
    return defuzzy(val)

def on_connect(client, userdata, flags, rc):
    print("Connected to publisher")
    client.subscribe("tugas-akhir")

def on_message(client, userdata, msg):
    evacuate = []
    scala = msg.payload
    floor = [1,2,3,4,5,6,7,8,9,10]
    print(scala)
    for i in range(len(floor)):
        evacuate.append(fuzzy_system(float(scala), float(floor[i])))
        print("Skala Ritcher : ", float(scala))
        print("Posisi Lantai : ", float(floor[i]))
        # setelah skor dari output fuzzy didapatkan, input kedalam fungsi is_evacuate
        # untuk mendapatkan jenis evakuasi dalam string
        evacuation_msg = is_evacuate(evacuate[i])
        # topic untuk subcriber dibedakan pada tiap lantai dari 1-10
        evacuation_topic = 'floor_' + str(i+1)
        client.publish(evacuation_topic, evacuation_msg)

def is_evacuate(evacuation_score):
    '''Fungsi untuk menentukan jenis evakuasi dari setiap lantai

    Input   : Skor output fuzzy
    Output  : Pesan string untuk evakuasi
    '''

    if evacuation_score <= 30:
        message = "Siaga"
    elif evacuation_score >= 30 and evacuation_score <= 50:
        message = "Waspada"
    else:
        message = "Awas"
    
    return message

if __name__ == "__main__":
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set(username, password)
    client.connect(url_broker, port)

    client.loop_forever()