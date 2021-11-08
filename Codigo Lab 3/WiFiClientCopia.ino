#include "DHT.h"; //Libreria Sensor de temperatura
#include <ESP32Servo.h> //Libreria servomotor
#include <WiFi.h> // Libreria WiFi
#include <PubSubClient.h> // Libreria Cliente
#include "ESP32_MailClient.h"; // Libreria Correo

//**************************************
//*********** VARIABLES CONSTANTES ***************
//**************************************

const char* mqtt_server = "node02.myqtthub.com";
const int mqtt_port = 1883;
const char* mqtt_user = "cliente2";
const char* mqtt_pass = "cliente2";

const char* root_topic_subscribe = "Topic Raiz";
const char* root_topic_subscribe2 = "Temperatura";
const char* root_topic_subscribe3 = "Humedad";
const char* root_topic_subscribe4 = "Puerta";

const char* root_topic_publish = "Topic Raiz";
const char* root_topic_publish2 = "Temperatura";
const char* root_topic_publish3 = "Humedad";
const char* root_topic_publish4 = "Puerta";

const char* ssid     = "RED O";
const char* password = "267519XYZ";

//Credenciales correo

const String hostEmail = "smtp.gmail.com";
const String Email = "sucorreoelectronico@gmail.com";
const String pass = "sucontraseña";

//**************************************
//*********** GLOBALES   ***************
//**************************************
WiFiClient espClient;
PubSubClient client(espClient);
char msg1[100];
char msg2[100];
char msg3[100];
char msg4[100];

DHT dht(4, DHT11);
const int rele = 5;
SMTPData datosSMTP;
Servo servo;

//************************
//** F U N C I O N E S ***
//************************
void callback(char* topic, byte*  payload, unsigned int length);
void reconnect();
void setup_wifi();

void setup() {
  Serial.begin(115200);
  dht.begin(); //Inicializa sensor DHT11
  pinMode(rele, OUTPUT); //Inicializa rele como salid
  digitalWrite(rele, HIGH); // Coloca en estado APAGADO el rele
  setup_wifi(); //Llama función de configuracion red wifi
  client.setServer(mqtt_server, mqtt_port); //Estable conexion con el servidor
  client.setCallback(callback); //Llama la función y envia los topic
  servo.attach(23);// Inicializa el ServoMotor
}

void loop() {

    int t = dht.readTemperature();
  int h = dht.readHumidity();

      String str1,str2,str3,str4;

      String content = "", title = "";

  if(Serial.available()){
        char data = Serial.read();
        if(data == '4'){
         Serial.println(root_topic_publish4);
        servo.write(0);
        str4 = "Abierta";
        str4.toCharArray(msg4, 100);
        client.publish(root_topic_publish4, msg4);
        Serial.println(msg4);
        }
        if(data == '5'){
         Serial.println(root_topic_publish4);
        servo.write(270);
        str4 = "Cerrada";
        str4.toCharArray(msg4, 100);
        client.publish(root_topic_publish4, msg4);
        Serial.println(msg4);
        }
      }

  if (!client.connected()) {
    reconnect();
  }

  if (client.connected()) {



      if(root_topic_publish == "Topic Raiz"){
        Serial.println("---- Información TOPIC RAIZ ----");
        if (t > 30) {
          //Serial.println("---- Iniciando Correo ----");
          delay(200);
          title = "Temperatura";
          content = "Hola soy YO, te aviso que el ventilador sea encendido";
          correo(title,content); //Llama función de envio de correo
          digitalWrite(rele, LOW); //Enciende el rele
          str1 = "Temperatura: " + String(t) + "°C " + "Humedad: " + String(h) + "% " + "Ventidalor encendido";
        } else {
          //Serial.println("---- Iniciando Correo ----");
          delay(200);
          title = "Temperatura";
          content = "Hola soy tu YO, te aviso que el ventilador sea apagado";
          correo(title,content); //Llama función de envio de correo
          digitalWrite(rele, HIGH); //Apaga el rele
          str1 = "Temperatura: " + String(t) + "°C " + "Humedad: " + String(h) + "% " + "Ventidalor apagado";
        }
        str1.toCharArray(msg1, 100);
        client.publish(root_topic_publish, msg1);
        Serial.println(msg1);
      }
      if(root_topic_publish2 == "Temperatura"){
        Serial.println("---- Información TEMPERATURA ----");
        Serial.println(root_topic_publish2);
        str2 = String(t) + "°C";
        str2.toCharArray(msg2, 100);
        client.publish(root_topic_publish2, msg2);
        Serial.println(msg2);
      }
      if(root_topic_publish3 == "Humedad"){
        Serial.println("---- Información HUMEDAD ----");
        Serial.println(root_topic_publish3);
        str3 = String(h) + "%";
        str3.toCharArray(msg3, 100);
        client.publish(root_topic_publish3, msg3);
        Serial.println(msg3);
      }
            
      
      

    delay(25000);
  }
  client.loop();
}

//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi() {
  delay(1000);
  // Nos conectamos a nuestra red Wifi
  Serial.println();
  Serial.print("Conectando a ssid: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a red WiFi!");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

//*****************************
//***    CONEXION MQTT      ***
//*****************************
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión Broker...");
    // Creamos un cliente ID
    String clientId = "cliente_2";

    // Intentamos conectar
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Conectado al broker!");
      Serial.println("Cliente en uso: " + clientId);
      // Nos suscribimos
      if (client.subscribe(root_topic_subscribe)) {
        client.subscribe(root_topic_subscribe2);
        client.subscribe(root_topic_subscribe3);
        client.subscribe(root_topic_subscribe4);
        Serial.println("Suscripcion a topic " + String(root_topic_subscribe));
        Serial.println("Suscripcion a topic " + String(root_topic_subscribe2));
        Serial.println("Suscripcion a topic " + String(root_topic_subscribe3));
        Serial.println("Suscripcion a topic " + String(root_topic_subscribe4));
        Serial.println("------------------------- TOPICS -------------------------");
      } else {
        Serial.println("fallo Suscripciión a topic " + String(root_topic_subscribe));
        Serial.println("fallo Suscripciión a topic " + String(root_topic_subscribe2));
        Serial.println("fallo Suscripciión a topic " + String(root_topic_subscribe3));
        Serial.println("fallo Suscripciión a topic " + String(root_topic_subscribe4));
      }
    } else {
      Serial.print("falló conexión broker:( con error -> ");
      Serial.print(client.state());
      Serial.println(" Intentamos de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

//*****************************
//***       CALLBACK        ***
//*****************************
void callback(char* topic, byte* payload, unsigned int length) {
  String incoming = "";
  Serial.print("Mensaje recibido desde -> ");
  Serial.print(topic);
  Serial.println("");
  for (int i = 0; i < length; i++) {
    incoming += (char)payload[i];
  }
  incoming.trim();
  
  Serial.println("Mensaje -> " + incoming);
}

void correo(String title, String content){
  datosSMTP.setLogin(hostEmail,465,Email,pass);
  datosSMTP.setSender("ESP32S", Email);
  datosSMTP.setPriority("High");
  datosSMTP.setSubject(title);
  datosSMTP.setMessage(content, false);
  datosSMTP.addRecipient(Email);
  if (!MailClient.sendMail(datosSMTP)){
    Serial.println("Error enviando el correo, " + MailClient.smtpErrorReason());
    datosSMTP.empty();
    delay(10000);
  }
}
