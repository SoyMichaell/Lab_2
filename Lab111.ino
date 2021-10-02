#include "DHT.h";
#include <ESP8266WiFi.h>

DHT dht(0,DHT11);

//Datos para conexion a red de wifi (Nombre red, Password)
const char* ssid = "RED O";
const char* password = "267519XYZ";

//Declarmos el puerto
WiFiServer server(80);

//Variable para almacenar la solicitud HTTP
String header;

//Variable que controla el estado del led
String outputRedState = "off";


//Asignamos pin a LED (En este caso esta conectado al IO2)
const int redLED = 2;
const int cooler = 0;

//Tiempo actual
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
//Definimos tiempo de espera en milisegundos
const long timeoutTime = 2000;

        
void setup() {
  Serial.begin(9600);
  dht.begin();
  //Inicializamos las salidas
  pinMode(redLED, OUTPUT);
  pinMode(cooler, OUTPUT);
  //Salida del led en bajo
  digitalWrite(redLED, LOW);
  digitalWrite(cooler, LOW);

  //Establecemos conexion a la red wifi
  Serial.print("Conectando");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  //Este while compara el estado de la conexion, si no establece conexión se da un delay se repite el proceso hasta que haya conexión
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //Nos muestra la Ip de conexión
  Serial.println("");
  Serial.print("Conectado a led wifi.");
  Serial.println(ssid);
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
}


void loop(){

  //Escucha los cliente o peticiones recibidas
  WiFiClient client = server.available();
  //Variables de humedad y temperatura
  float h = dht.readHumidity();
  float t = dht.readTemperature();


  //Evalua si hay una nueva peticion
  if (client) {
    Serial.println("Nueva Petición."); //Muestra un mensaje de nueva petición
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { //Forma un bucle mientras el cliente esta conectado a la red
      currentTime = millis(); 
      if (client.available()) { //Si hay bytes para lectura
        char c = client.read(); //Leer byte
        Serial.write(c); //Imprime en el monitor
        header += c;
        if (c == '\n') { // Evalua si el byte es un caracter de nueva linea
          //Si la linea actual esta en blanco, tiene dos caracteres de nueva linea seguidos, indica que es el final de la solicitud del cliente
          if (currentLine.length() == 0) {
          //Encabezado y tipo de contenido
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          Serial.println(t);
          //Enciende y apaga el IO2
          //Si en el header o uri se envia por get /2/on se enciende el led
          if (header.indexOf("GET /2/on") >= 0) {
            Serial.println("ENCENDIDO");
            //Con este controlamos el estado led, empieza en off y cambiamos a on
            outputRedState = "on";
            digitalWrite(redLED, HIGH);
          } else if (header.indexOf("GET /2/off") >= 0) {
            Serial.println("APAGADO");
            outputRedState = "off";
            digitalWrite(redLED, LOW);
          }

          //Pagina HTML
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          client.println("<link href='https://fonts.googleapis.com/css2?family=Work+Sans:wght@300&display=swap' rel='stylesheet'>");
          client.println("<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@4.6.0/dist/css/bootstrap.min.css' integrity='sha384-B0vP5xmATw1+K9KRQjQERJvTumQW0nPEzvF6L/Z6nronJ3oUOFUFpCjEUQouq2+l' crossorigin='anonymous'>");
          //Estilos css
          client.println("<style>html { font-family: 'Work Sans', sans-serif; margin: 0px auto; text-align: center;}</style></head>");
          // Titulo
          client.print("<body>");
          client.print("<div class='container mt-4'>");
            client.print("<div class='jumbotron'><h1 class='display-4'>Sistema de control</h1><p class='lead'>Modulo esp 01 - ARDUINO</p><hr class='my-4'>");
            if(outputRedState=="off"){
              client.println("<a class='btn btn-success btn-lg shadow' href=\'/2/on\' role='button'>Encender sistema</a></div>");
            client.print("</div>");
            }else{
              digitalWrite(cooler,HIGH);
              client.println("<a class='btn btn-danger btn-lg shadow' href=\'/2/off\' role='button'>Apagar sistema</a></div>");
              client.print("<div class='row'>");
              client.print("<div class='col-md-6'><div class='card'><div class='card-header'>Sensor de Temperatura</div><div class='card-body'>"); client.print(t); client.print("</div></div></div>");
              client.print("<div class='col-md-6'><div class='card'><div class='card-header'>Sensor de Humedad</div><div class='card-body'>"); client.print(h);
              client.print("</div></div></div>");
          client.print("</div>"); 
            }
          client.println("</div>");  
          //Input para el envio de la temperatura
          client.println("</body></html>");

          client.println();
          // Rompe el bucle
          break;
          } else { 
          currentLine = "";
          }
          } else if (c != '\r') {
          currentLine += c; 
          }
      }
    }
          //Limpia el header
          header = "";
  }
}
