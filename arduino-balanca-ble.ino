/*Bibliotecas*/
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <string>

BLEService weightService("4298cd96-8280-11ee-b962-0242ac120002");                              // UUID do serviço
BLECharacteristic requestCharacteristic("4298d03e-8280-11ee-b962-0242ac120002", BLEWrite, 4);  // UUID da característica de request
BLECharacteristic responseCharacteristic("4298d67e-8280-11ee-b962-0242ac120002", BLENotify, 4);

char pesoData[10];  // Array de caracteres para armazenar dados de peso

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int buttonPin = 4;
const int ledPin = 2;

const int DOUT = 27;
const int CLK = 26;
HX711 balanca;
float calibracao = 211830;
float peso = 0.0;
float pesoArray[10];
int tamanho = 0;
const int amostras = 5;

int buttonState = 0;

void setupBLE() {
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1)
      ;
  }

  // Adiciona as características ao serviço
  weightService.addCharacteristic(requestCharacteristic);
  weightService.addCharacteristic(responseCharacteristic);
  // Adiciona o serviço ao BLE
  BLE.addService(weightService);
  // Define o nome do dispositivo
  BLE.setLocalName("sensorBalanca");
  // Inicia a transmissão do BLE
  BLE.advertise();
  Serial.println("- Iniciando BLE");
};
void setupBotao() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.println("- Iniciando push button");
};
void setupBalanca() {
  balanca.begin(DOUT, CLK);
  balanca.set_scale(calibracao);
  balanca.tare();
  Serial.println("- Iniciando Botao");
};

void setupDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  Serial.println("- Iniciando o display");
};

void setup() {
  Serial.begin(9600);
  // setupBLE();
  setupBotao();
  setupBalanca();
  setupDisplay();
  Serial.println("- Aguardando conexao...");
};

void loop() {
  controlarBLE();
  lerBotao();
  controlarBalanca();
  mostrarDisplay();
  debugSerial();
};

void debugSerial(void) {
  Serial.println("");
  Serial.println("PESO: ");
  Serial.print(peso);
  Serial.print(" KG");
  if (buttonState != HIGH) {
    Serial.println("TARA");
  }
};
void controlarBLE() {
  // Verifica se há uma conexão BLE
  BLEDevice central = BLE.central();

  // Se há uma conexão BLE
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    // Aguarda os dados da característica de request
    while (central.connected()) {

      // float peso = random(1, 100) / 10.0;;
      dtostrf(peso, 5, 2, pesoData);
      responseCharacteristic.writeValue(pesoData);

      //
      if (requestCharacteristic.written()) {

        char requestValue = requestCharacteristic.value()[0];
        if (requestValue == 'G') {
          balanca.tare();  // aqui seria o valor do peso recibido - TEM QUE MANDAR FLOAT
          Serial.println("Received 'G' from central. Performing action...");

        } else {
          Serial.println(requestValue);
        }
        Serial.println("Sent weight data to central: " + String(pesoData) + " kg");

        delay(100);
      }
      if (!central.connected()) {
        Serial.println("- dispositivo desconectado");
      }
    }
  }
};
void controlarDisplay(uint8_t textSize = 1, int16_t posX = 0, int16_t posY = 0) {
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(posX, posY);
};
void lerBotao(void) {
  buttonState = digitalRead(buttonPin);
};
void mostrarDisplay(void) {
  display.clearDisplay();
  controlarDisplay(3, 10, 45);
  display.println(peso);
  controlarDisplay(2, 100, 50);
  display.println(F("KG"));

  if (buttonState != HIGH) {
    digitalWrite(ledPin, LOW);
    controlarDisplay(1, 5, 0);
    display.println(F("TARA"));
  }
  controlarDisplay(1, 100, 0);
  display.println(F("BLU"));
  display.display();
};
void controlarBalanca(void) {
  balanca.power_up();
  peso = balanca.get_units(3);

  if (peso < 0.00 && peso > -0.1) {
    peso = 0.00;
  }
  if (buttonState != HIGH) {
    digitalWrite(ledPin, LOW);
    balanca.tare();
  }
  balanca.power_down();
  delay(20);
};
