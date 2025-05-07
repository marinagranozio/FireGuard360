#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

char ssid[] = "";  // Nome della rete Wi-Fi
char pass[] = "";  // Password Wi-Fi

char server[] = "";  // IP del PC con Flask
int port = 5000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, server, port);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Connessione Wi-Fi
  WiFi.begin(ssid, pass);
  Serial.println("Connessione WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connesso al WiFi!");
}

void loop() {
  // Genera valori casuali per temperatura e umidità
  float temperatura = random(150, 300) / 10.0;  // Temperatura tra 15.0 e 30.0°C
  float umidita = random(300, 800) / 10.0;      // Umidità tra 30.0% e 80.0%

  // Crea il payload JSON
  String payload = "{\"temperatura\":";
  payload += temperatura;
  payload += ", \"umidita\":";
  payload += umidita;
  payload += "}";

  // Invia i dati tramite POST al server Flask
  client.beginRequest();
  client.post("/dati");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", payload.length());
  client.beginBody();
  client.print(payload);
  client.endRequest();

  // Leggi la risposta del server
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("HTTP Status: ");
  Serial.println(statusCode);
  Serial.println("Risposta: " + response);

  delay(5000);  // Attendi 5 secondi prima di inviare i dati successivi
}
