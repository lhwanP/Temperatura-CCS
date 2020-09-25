// Programa Pronto da Temperatura do Laboratorio - 21/05/2018

#include <SoftwareSerial.h> // biblioteca para o modulo wifi esp8266

// --------------------------------------------
// Bibliotecas para controle de temperatura
#include <LiquidCrystal.h>
#include <Limits.h>
// --------------------------------------------

// -----------------------------------------------------------------------------------------------
// Variaveis para controle de temperatura

// Declara os pinos analógicos em que o sensor de temperatura está conectado
const int sensorTemp[] = {0, 1, 2, 3, 4};

// Declara a variável das salas
String nomesala[] = {"XPS:", "Implantador:", "Sputtering:", "S_MAQ:", "T_Externa:"};
String idsala[] = {"XPS", "Implantador", "Sputtering", "S_MAQ", "T_Externa"};

// Variável usada para ler o valor de temperatura
int valorSensorTemp[] = {0, 0, 0, 0, 0};

int sensores = 5;
// Variável usada para armazenar o menor valor de temperatura
int valorTemp = INT_MAX;

// Variavel de resposta do modulo ESP8266
String enderecoip = "";

// Biblioteca do LCD e os pinos conectados
LiquidCrystal lcd(9, 8, 5, 4, 3, 2);
// -------------------------------------------------------------------------------------------------

SoftwareSerial esp8266(12, 11); // definição dos pino: RX pino 12, TX pino 11 (RX liga no TX do Modulo e TX liga no RX do Modulo)
//SoftwareSerial esp8266(2, 3); // definição dos pino: RX pino 2, TX pino 3

String server = "intranet.ccs.unicamp.br";
//String server="143.106.5.124";
String data = "MEV=1&FIB=2";
String uri = "/lab_monitor/log.php";

#define DEBUG true // exibe a resposta dos comandos


void setup()
{
  // ----------------------------------------------------------------------------
  // Define o display com 16 colunas e 2 linhas do controle de temperatura
  lcd.begin(16, 2);
  lcd.clear();  // Limpa o display do LCD
  lcd.setCursor(0, 0);
  lcd.print("Inicializando");

  Serial.begin(9600); // velocidade do ...
  Serial.println("CONFIG");
  // Configure na linha abaixo a velocidade inicial do modulo ESP8266:
  esp8266.begin(19200); // velocidade inicial do modulo wifi esp8266
  sendData("AT+RST\r\n", 2000, DEBUG); // restabelecer o módulo
  delay(1000);
  Serial.println("RESET");
  esp8266.begin(115200); // segunda velocidade depois do reset
  sendData("AT+RST\r\n", 2000, DEBUG); // restabelecer o módulo
  delay(1000);
  sendData("ATE1\r\n", 2000, DEBUG); // ativar o eco ( respode o comando digitado )
  delay(1000);
  Serial.println("Versao de firmware"); // imprime na tela
  delay(3000);
  sendData("AT+GMR\r\n", 2000, DEBUG); // impressão da versão de firmware

  Serial.println("CONNECTA");

  // Configure na linha abaixo a velocidade desejada para a comunicacao do modulo ESP8266 (9600, 19200, 38400, etc)
  sendData("AT+CIOBAUD=19200\r\n", 2000, DEBUG); // setar a velocidade de comunicação
  Serial.println("** Final **"); // imprime na tela a finalização
  esp8266.begin(19200); // velocidade do modulo esp8266

  lcd.clear();  // Limpa o display do LCD
  lcd.setCursor(0, 0);
  lcd.print("Conectando");

  // Conecta a rede wireless
  sendData("AT+CWJAP=\"CCSLAB-A\",\"dptrjmn54\"\r\n", 2000, DEBUG); // SSID e senha para conectar o módulo esp8266 ao wifi
  delay(3000);
  sendData("AT+CWMODE=1\r\n", 3000, DEBUG); // seta como cliente wifi
  delay(1000);
  // Mostra o endereco IP

  enderecoip = sendData("AT+CIFSR\r\n", 3000, DEBUG); // Obtenha o endereço IP local do modulo wifi
  int index1 = enderecoip.indexOf(",");
  Serial.println(enderecoip);
  enderecoip = enderecoip.substring(index1 + 2);
  Serial.println(enderecoip);
  index1 = enderecoip.indexOf("\"");
  enderecoip = enderecoip.substring(0, index1);
  Serial.println(enderecoip);




  lcd.clear();  // Limpa o display do LCD
  lcd.setCursor(0, 0);
  lcd.print(enderecoip);
  delay(3000);

}

void loop() {

  leituradesensores();
  montarpayload();
  httppost();
  Serial.println(data);
}

void montarpayload() {

  data = "";
  for (int j = 0; j < 5; j++)
  {
    if (j > 0) data.concat("&");
    data.concat(idsala[j] + "=" + valorSensorTemp[j]);
  }
}


// --------------------------------------------------------------------------------------------
// Controle de temperatura
void leituradesensores()
{

  for (int j = 0; j < 5; j++)
  {
    // Inicializando a variável com o maior valor int possível
    valorTemp  = INT_MAX;

    // Para evitar as grandes variações de leitura do componente LM35 são feitas 6 leitura é o menor valor lido prevalece
    for (int i = 1; i <= 6; i++) // Lendo o valor do sensor de temperatura
    {

      valorSensorTemp[j] = analogRead(sensorTemp[j]);

      // Transforma o valor lido do sensor de temperatura em graus Celsius aproximados
      //valorSensorTemp[j] *= 0.54;

      // Mantendo sempre a menor temperatura lida
      if (valorSensorTemp[j] < valorTemp) {
        valorTemp = valorSensorTemp[j];
      }
      delay(1000); // Aguarda 1 segundo
    }

    valorSensorTemp[j] = valorTemp;  
    valorTemp *= 0.54;

    // Exibindo valor da leitura do sensor de temperatura no display LCD
    lcd.clear();  // Limpa o display do LCD
    lcd.print(nomesala[j]);  // Imprime a string no display do LCD
    lcd.print(valorTemp);  // Imprime o valor da temperatura
    lcd.write(B11011111); // Símbolo de graus Celsius
    lcd.print("C");
    lcd.setCursor(0, 1);
  }
}
// ---------------------------------------------------------------------------------------------

String sendData(String command, const int timeout, boolean debug) // parametros da função
{
  // Envio dos comandos AT para o modulo
  String response = "";
  esp8266.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      // A esp8266 tem dados para exibir sua saída para a janela serial
      char c = esp8266.read(); // leitura do próximo caracter
      response += c;
    }
  }
  if (debug) // se for debug
  {
    Serial.print("RESPOSTA:" + response); // imprime a resposta
  }
  return response;
}

void httppost()
{

  Serial.println("Conecta");

  sendData("AT+CIPSTART=\"TCP\",\"" + server + "\",80\r\n", 1000, DEBUG); // start a TCP connection

  String postRequest =

    "POST " + uri + " HTTP/1.0\r\n" +

    "Host: " + server + "\r\n" +

    "Accept: *" + "/" + "*\r\n" +

    "Content-Length: " + data.length() + "\r\n" +

    "Content-Type: application/x-www-form-urlencoded\r\n" +

    "\r\n" + data;

  int requestLength = postRequest.length();
  String sendCmd = "AT+CIPSEND=";
  //+requestLength; // determine the number of caracters to be sent.
  //sendCmd = sendCmd + "\r\n";
  sendCmd.concat(requestLength);
  Serial.println(sendCmd);

  Serial.println("POST");
  //sendData(sendCmd, 1000, DEBUG);
  esp8266.println(sendCmd);
  delay(1000);

  if (esp8266.find(">")) {

    Serial.println("Sending..");
    //sendData(postRequest, 1000, DEBUG);
    esp8266.print(postRequest);
    delay(1000);

    if (esp8266.find("SEND OK")) {

      Serial.println("Packet sent");
      while (esp8266.available())
      {
        String tmpResp = esp8266.readString();
        Serial.println(tmpResp);

      }
    }
  }
  // close the connection
  esp8266.println("AT+CIPCLOSE");
}

