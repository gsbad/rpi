// ===================================================== INCLUDES =========================================================
// --- WIFI ---
#include <ESP8266WiFi.h>
const char* ssid = "fsociety";          
const char* password = "instalarvirus";     
WiFiClientSecure nodemcuClientSeguro;
//WiFiClient nodemcuClient;

// --- MQTT Cliente de publicação do Dashboard Adafruit ====================================================================
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Mapeamento do modulo relay 
#define RELAY D2  

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883                   // use 8883 for SSL --or 1883 default
#define AIO_USERNAME    "gsbad"
#define AIO_KEY         "f0c1cb727a524bdfa502d7bc729b8690"

Adafruit_MQTT_Client mqtt(&nodemcuClientSeguro, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

// Declaraçao das feeds Adafruit
Adafruit_MQTT_Subscribe iluminacaoClient = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/iluminacao");
Adafruit_MQTT_Publish temperaturaClient = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish umidadeClient = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/umidade");



// --- MQTT Cliente de publicação --- ======================================================================================
/*#include <PubSubClient.h>
const char* mqtt_Broker = "iot.eclipse.org"; // Ou o servidor mosquito local 192.168.0.59
const char* mqtt_ClientID = "termometro-nodemcu";
PubSubClient clientPubSub(nodemcuClient);

const char* topicoTemperatura = "lab/temperatura";
const char* topicoUmidade = "lab/umidade";
*/

// --- DHT --- =============================================================================================================
#include <DHT.h>
#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int umidade;
int temperatura;


// --- DISPLAY --- #SANAR PANE DO DISPLAY, DEPOIS DESCOMENTA TUDO REFERENTE À ELE ==========================================
/*#include <Adafruit_SSD1306.h>
#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(OLED_RESET);*/





// =================================================== SETUP =============================================================== 
void setup() {
  Serial.begin(9600);
  Serial.println("DHTxx test!");

  dht.begin();
  //configurarDisplay();

  conectarWifiSerial();
  //clientPubSub.setServer(mqtt_Broker, 1883);

  pinMode(RELAY, OUTPUT); //Seta o pino D2 do relay para adafruit
  digitalWrite(RELAY, HIGH); //MaNtem relay desligado na inicialização

  inscreveTopicoMQTTAdafruit();

}

// ===================================================== LOOP ==============================================================
void loop() {

  // Conecta no clientMQTT
  /*if (!clientPubSub.connected()) {
    reconectarMQTT();
  }
  */

  // Captura os valores da temperatura e umidade atraves do DHT22
  medirTemperaturaUmidade();
  //mostrarTemperaturaUmidade();
  
  // Conecta no MQTT da Adafruit
  MQTT_connect();
    // Iteração de onoff
    verificarToogleRelayDash();
    // Publicação dos dados dos sensores
    publicarMQTTAdafruid();

  // Verificação de conexão com MQTT adfruit io  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  
  // Medir e exibir temperatura e umidade no serial p fins de debug
  medirTemperaturaSerial();
  //publicarTemperaturaUmidadeNoTopico();
}


// ========================================= FUNÇÕES Q RODAM EM SETUP E LOOP===============================================

// CONECTA WIFI E EXIBE NO DISPLAY LED
/*void conectarWifi() {
  delay(10);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("Conectando ");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
    display.display();
  }
}*/

// CONECTA WIFI E EXIBE NA SERIAL A CONFIRMACAO
void conectarWifiSerial() {
  delay(10);

  // Começamos a conectar na rede WiFi

  Serial.println();
  Serial.println();
  Serial.print("Conectando em  ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");  
  Serial.println("Endereco IP: ");
  Serial.println(WiFi.localIP());
}

// FUNÇÃO QUE RECONECTA O MQTT DA LIB PUBSUB
/*void reconectarMQTT() {
  while (!clientPubSub.connected()) {
    clientPubSub.connect(mqtt_ClientID);
  }
}*/

// CONFIGURA DISPLAY LED
/*void configurarDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Compute heat index in Celsius (isFahreheit = false)
  display.setTextColor(WHITE);
  display.clearDisplay();
}*/

// FUNÇÃO QUE PUBLICA NO MQTT DA LIB PUBSUB
/*void publicarTemperaturaUmidadeNoTopico() {
  clientPubSub.publish(topicoTemperatura, String(temperatura).c_str(), true);
  clientPubSub.publish(topicoUmidade, String(umidade).c_str(), true);
}
*/

// SE SUBSCREVE EM UM TÓPICO DO DASH ADAFRUIT IO
void inscreveTopicoMQTTAdafruit(){
  
  mqtt.subscribe(&iluminacaoClient);
  
}

// FUNÇÃO Q RODA EM LOOP PARA VERIFICAR SE OUVE MUDANÇAS NOS TÓPICOS SUBSCRITOS
void verificarToogleRelayDash(){  

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {

    // Feed ILUMINACAO
    if (subscription == &iluminacaoClient) {
      
      Serial.print(F("On-Off button: "));
      Serial.println((char *)iluminacaoClient.lastread);
    //  Serial.print("Interruptor funcionando");
      
      if (strcmp((char *)iluminacaoClient.lastread, "ON") == 0) {
         Serial.print("Interruptor ligado");
        digitalWrite(RELAY, LOW); 
      }
      if (strcmp((char *)iluminacaoClient.lastread, "OFF") == 0) {
        Serial.print("Interruptor desligado");
        digitalWrite(RELAY, HIGH); 
      }
    }

    
  }
}

// FUNÇÃO Q PUBLICA DADOS EM UM TÓPICO
void publicarMQTTAdafruid(){
  
  // Para fins de debug na serial
  Serial.print(F("\nEnviando dado de temperatura "));
  Serial.print(temperatura);
  Serial.print("...");
  
  // Para fins de debug na serial
  Serial.print(F("\nEnviando dado de umidade "));
  Serial.print(umidade);
  Serial.print("...");

  // Publicando e testando
  if (! temperaturaClient.publish(temperatura)) {
    Serial.println(F("Falhou"));
  } else {
    Serial.println(F("OK!"));
  }
  if (! umidadeClient.publish(umidade)) {
    Serial.println(F("Falhou"));
  } else {
    Serial.println(F("OK!"));
  }   
  
}

// FUNÇÃO Q CONECTA NO PROTOCOLO MQTT DA ADAFRUIT
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}


// Mede as temperaturas do DHT22 e atribui as váriaveis
void medirTemperaturaUmidade() {
  umidade = dht.readHumidity() + 40.0;
  temperatura = dht.readTemperature(false);
  delay(5000);
}

/*void mostrarTemperaturaUmidade() {
  mostrarMensagemNoDisplay("Temperatura", (temperatura), " C");
  mostrarMensagemNoDisplay("Umidade", (umidade), " %");
}

void mostrarMensagemNoDisplay(const char* texto1, int medicao, const char* texto2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(texto1);
  display.setTextSize(5);
  display.setCursor(20, 20);
  display.print(medicao);
  display.setTextSize(2);
  display.print(texto2);
  display.display();
  delay(2000);
}*/


// Mede as temperaturas do DHT22 e exibe na serial para fins de debug
void medirTemperaturaSerial() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity() + 40.0;
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Indice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}
