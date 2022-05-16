//Incluimos la libreria de la Pantalla
#include <LiquidCrystal.h>
//Incluimos la libreria de conectividad
#include <WiFi.h>
//Incluimos la libreria de Firebase
#include <FirebaseESP32.h>

// Definimos la conectividad de la red

#define ssid "Usco_Ingenieria"
#define pass ""
#define URL "https://base-hc-sr04-default-rtdb.firebaseio.com/"
#define secreto "4Qfla7Z6BQOpP1NOfdcGT0ljf16sCRcmu4RNSI1e"
FirebaseData myFireBaseData;
FirebaseData fbdo;

// Configuro el ADC
#define ADC_VREF_mV    3300.0 // in millivolt
#define ADC_RESOLUTION 4096.0
#define PIN_LM35       32 // ESP32 pin GIOP36 (ADC0) connected to LM35


//Creamos el caracter oscuro
byte no[8]={
  0b00011111,
  0b00000000,
  0b00011111,
  0b00011111,
  0b00011111,
  0b00011111,
  0b00000000,
  0b00011111,  
};

//Creamos el caracter 80
byte B[8]={
  0b00000000,
  0b00001110,
  0b00001110,
  0b00000000,
  0b00011111,
  0b00010101,
  0b00011111,
  0b00000000,
};

//Creanis el caracter 60
byte C[8]={
  0b00000000,
  0b00001110,
  0b00001110,
  0b00000000,
  0b00010111,
  0b00010101,
  0b00011111,
  0b00000000,
};

//Creamos el caracter 40
byte D[8]={
  0b00011110,
  0b00011110,
  0b00000000,
  0b00000100,
  0b00011110,
  0b00010100,
  0b00001100,
  0b00000100,
};

//Creamos el caracter 20
byte E[8]={
  0b00011111,
  0b00010001,
  0b00011111,
  0b00000000,
  0b00011101,
  0b00010101,
  0b00010101,
  0b00010111,
};

//Creamos el caracter 0
byte F[8]={
  0b00000000,
  0b00000000,
  0b00011111,
  0b00010001,
  0b00010001,
  0b00011111,
  0b00000000,
  0b00000000,
};

//Creamos la base
byte G[8]={
  0b00011111,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00000001,
  0b00011111,
};

//Creamos el cuerpo
byte I[8]={
  0b00011111,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00011111,
};

//Declaramos las constanteS de los Pines
const int ECHO = 22; //Pin ECHO
const int TRIG = 23; //Pin TRIG
const int BOMBA1 = 4; // pin BOMBA LLENADO
const int BOMBA2 = 2; // Pin BOMBA DRENADO
bool Modo, sistema, drenado, llenado, fully;
float Nivel_Deseado, Nivel_Actual, Nivel_Total;
float area, altura, porcentaje, miliVolt, temp, guarda;
long tiempo, distancia;
int adcVal;

//Configuramos la pantalla 
LiquidCrystal lcd(13, 12, 14, 27, 26, 25);



void setup() {
  //Configuramos los Pines
  pinMode (ECHO, INPUT); //Declaro que el P. ECHO sera de Entrada
  pinMode (TRIG, OUTPUT); //Declaro que el P. TRIG sera de Salida
  pinMode (BOMBA1, OUTPUT); //Declaro que el p.Bomba sera de Salida
  pinMode (BOMBA2, OUTPUT); 
  lcd.begin(16, 2); //Inicializamos la LCD
  Serial.begin(115200); //Inicializamos el Puerto serie
  lcd.createChar (0, no);  //Caractér de nivel
  lcd.createChar (2,B); //Caractér de 80
  lcd.createChar (3,C); //Caractér de 60
  lcd.createChar (4,D); //Caractér de 40
  lcd.createChar (5,E); //Caractér de 20
  lcd.createChar (6,F); //Caractér de 0
  lcd.createChar (7,G); //Caractér base
  lcd.createChar (9,I); //Caractér cuerpo
  
  //Visualizamos el estado de las conexiones
  WiFi.begin(ssid,pass);
  Serial.print("Conectando a la red WiFi: ");
  Serial.println(ssid);
  while (WiFi.status()!=WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Firebase.begin(URL,secreto);
  Firebase.reconnectWiFi(true);
  Serial.println("Conectado con éxito");
  digitalWrite(BOMBA1, LOW);
  digitalWrite(BOMBA2, LOW);
}

void loop() {
  ControlSistema();
  if (sistema==true){
    //Sistema Activo
    ParametrosTanque();
    lcd.clear();
    CuerpoLCD();
    ModoOperacion();
    Serial.print("Modo:");
    Serial.print(Modo);
    Serial.print(" ");
    ConsultaNivel();
    Serial.print("Niv. Deseado");
    Serial.print(Nivel_Deseado); //OJO porcentual
    Serial.print(" ");
    Serial.print("Niv. Actual");
    NivelActual();
    Serial.print(Nivel_Actual);
    Serial.print(" ");
    NivelEnLCD();
    Serial.print("Niv. Total");
    Serial.print(Nivel_Total);
    Serial.print(" ");
    Serial.print(porcentaje);
    Serial.print("%");
    Temperatura();
    Serial.print(" ");
    Serial.print(temp);
    Serial.print("°C");
    if (Modo==true){
      // Llenado Manual
      if ((porcentaje<Nivel_Deseado) && (porcentaje<=100)){
        digitalWrite(BOMBA1, HIGH);
        digitalWrite(BOMBA2, LOW);
        Serial.print(" ");
        Serial.print("Manual");
        Serial.print(" ");
        Serial.println("Llenando");
        llenado=true;
        drenado=false;
      }
      if (porcentaje>Nivel_Deseado){
        digitalWrite(BOMBA1, LOW);
        digitalWrite(BOMBA2, HIGH);
        Serial.print(" ");
        Serial.print("Manual");
        Serial.print(" ");
        Serial.println("Drenando"); 
        llenado=false;
        drenado=true;  
        fully=false;   
      }
    }
      if (porcentaje>=100){
        fully=true;
      }
    if (Modo==false){
      // Llenado Auto, no importa inicio, llenamos hasta 100%
      if (porcentaje<100){
        Serial.print(" ");
        Serial.print("Auto");
        Serial.print(" ");
        Serial.println("Llenando");
        digitalWrite(BOMBA1, HIGH);
        digitalWrite(BOMBA2,LOW);
        drenado=false;
        llenado=true;
        fully=false;
        
      }
      if (porcentaje>=100){
        Serial.print(" ");
        Serial.print("Auto");
        Serial.print(" ");
        Serial.println("Lleno...");
        digitalWrite(BOMBA1,LOW);
        digitalWrite(BOMBA1,LOW); 
        fully=true;   
    }
    }
  }
  if (sistema==false){
    //Sistema Inactivo
    digitalWrite(BOMBA1, LOW);
    digitalWrite(BOMBA2, LOW);
    Serial.println("Sistema Deshabilitado...");
    ParametrosTanque();
    lcd.clear();
    CuerpoLCD();
    NivelActual();
    NivelEnLCD();
    Temperatura();
    if (porcentaje>=100){
      fully=true;
    }
    if (porcentaje<100){
      fully=false;
    }
  }
 ActualizarBase();   
}


void ModoOperacion(){
  // Consulto el Modo de Operación
  Firebase.get(myFireBaseData,"/Modo/");
  Modo=myFireBaseData.boolData();
}


void ConsultaNivel(){
  // Consulto el Nivel del tanque en el slider
  Firebase.get(myFireBaseData,"/Nivel_Deseado/");
  Nivel_Deseado=myFireBaseData.floatData();
}


void NivelActual(){
  // Consulto el Nivel real
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  tiempo=pulseIn(ECHO, HIGH);
  Nivel_Actual=(area*((0.01715*tiempo)-guarda)); // en CM, y se resta 5 para la diferencia del borde
}

void CuerpoLCD(){
  lcd.setCursor(12,0);
  lcd.write((byte)6); // caracter 0
  lcd.setCursor(10,0);
  lcd.write((byte)5); // caracter 20
  lcd.setCursor(8,0);
  lcd.write((byte)4); // caracter 40
  lcd.setCursor(6,0);
  lcd.write((byte)3); // caracter 60
  lcd.setCursor(4,0);
  lcd.write((byte)2); // caracter 80
  lcd.setCursor(3,1);
  lcd.write((byte)9);
  lcd.setCursor(12,1);
  lcd.write((byte)7);
  lcd.setCursor(4,1);
  lcd.write((byte)9);
  lcd.setCursor(5,1);
  lcd.write((byte)9);
  lcd.setCursor(6,1);
  lcd.write((byte)9);  
  lcd.setCursor(7,1);
  lcd.write((byte)9);
  lcd.setCursor(8,1);
  lcd.write((byte)9);
  lcd.setCursor(9,1);
  lcd.write((byte)9);
  lcd.setCursor(10,1);
  lcd.write((byte)9);
  lcd.setCursor(11,1);
  lcd.write((byte)9);

}

void NivelEnLCD(){
  porcentaje=100-((Nivel_Actual*100)/Nivel_Total);
  if (porcentaje>=0 && porcentaje<9){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=10 && porcentaje<19){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=20 && porcentaje<29){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=30 && porcentaje<39){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=40 && porcentaje<49){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
    lcd.setCursor(8,1);
    lcd.write((byte)0);
  }      
  if (porcentaje>=50 && porcentaje<59){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
    lcd.setCursor(8,1);
    lcd.write((byte)0);
    lcd.setCursor(7,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=60 && porcentaje<69){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
    lcd.setCursor(8,1);
    lcd.write((byte)0);
    lcd.setCursor(7,1);
    lcd.write((byte)0);
    lcd.setCursor(6,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=70 && porcentaje<79){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
    lcd.setCursor(8,1);
    lcd.write((byte)0);
    lcd.setCursor(7,1);
    lcd.write((byte)0);
    lcd.setCursor(6,1);
    lcd.write((byte)0);
    lcd.setCursor(5,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=80 && porcentaje<89){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
    lcd.setCursor(8,1);
    lcd.write((byte)0);
    lcd.setCursor(7,1);
    lcd.write((byte)0);
    lcd.setCursor(6,1);
    lcd.write((byte)0);
    lcd.setCursor(5,1);
    lcd.write((byte)0);
    lcd.setCursor(4,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=90 && porcentaje<99){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
    lcd.setCursor(8,1);
    lcd.write((byte)0);
    lcd.setCursor(7,1);
    lcd.write((byte)0);
    lcd.setCursor(6,1);
    lcd.write((byte)0);
    lcd.setCursor(5,1);
    lcd.write((byte)0);
    lcd.setCursor(4,1);
    lcd.write((byte)0);
    lcd.setCursor(3,1);
    lcd.write((byte)0);
  }
  if (porcentaje>=100){
    lcd.setCursor(12,1);
    lcd.write((byte)0);
    lcd.setCursor(11,1);
    lcd.write((byte)0);
    lcd.setCursor(10,1);
    lcd.write((byte)0);
    lcd.setCursor(9,1);
    lcd.write((byte)0);
    lcd.setCursor(8,1);
    lcd.write((byte)0);
    lcd.setCursor(7,1);
    lcd.write((byte)0);
    lcd.setCursor(6,1);
    lcd.write((byte)0);
    lcd.setCursor(5,1);
    lcd.write((byte)0);
    lcd.setCursor(4,1);
    lcd.write((byte)0);
    lcd.setCursor(3,1);
    lcd.write((byte)0);
    lcd.setCursor(2,1);
    lcd.write((byte)0);
  }
}

void ParametrosTanque(){
  // Ojo, los parámetros deben venir en Cm
  Firebase.get(myFireBaseData,"/Parametros/area");
  area=myFireBaseData.floatData();
  Firebase.get(myFireBaseData,"/Parametros/altura");
  altura=myFireBaseData.floatData();
  Nivel_Total=area*altura; //cm^3
  Firebase.get(myFireBaseData,"/Parametros/guarda");
  guarda=myFireBaseData.floatData();
  
}

void ControlSistema(){
  Firebase.get(myFireBaseData,"/Sistema/");
  sistema=myFireBaseData.boolData();
}

void Temperatura(){
  adcVal= analogRead(PIN_LM35);
  miliVolt = adcVal*(ADC_VREF_mV/ADC_RESOLUTION);
  temp= miliVolt/10;
}

void ActualizarBase(){
  if (llenado==true){
    Firebase.RTDB.setBool(&fbdo, "Llenado/", true);
  }
  if (llenado==false){
    Firebase.RTDB.setBool(&fbdo, "Llenado/", false);
  }
  if (drenado==true){
    Firebase.RTDB.setBool(&fbdo, "Drenado/", true);
  }
  if (drenado==false){
    Firebase.RTDB.setBool(&fbdo, "Drenado/", false);
  }
  if (fully==true){
    Firebase.RTDB.setBool(&fbdo, "Lleno/", true);
  }
  if (fully==false){
    Firebase.RTDB.setBool(&fbdo, "Lleno/", false);
  }
  Firebase.RTDB.setFloat(&fbdo, "Nivel_Actual/", porcentaje);
  Firebase.RTDB.setFloat(&fbdo, "Temperatura/", temp);
  Firebase.RTDB.setFloat(&fbdo, "Volumen/", Nivel_Actual);
  
}
