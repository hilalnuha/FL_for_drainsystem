#include <Wire.h>
#define CENTER 821
#include <Fuzzy.h>
#include <FuzzyComposition.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzyOutput.h>
#include <FuzzyRule.h>
#include <FuzzyRuleAntecedent.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzySet.h>

#include <DallasTemperature.h>
#include <OneWire.h>

const float batasAtas = 5.0;
const float batasBawah = 11.0;

//inisialisasi objek fuzzy
Fuzzy* fuzzy = new Fuzzy();

//fuzzy PH
FuzzySet* asam = new FuzzySet(0.0, 0.0, 6.0, 7.0);
FuzzySet* netral = new FuzzySet(6.0, 7.0, 8.5, 9.0);
FuzzySet* basa = new FuzzySet(8.5, 9.0, 14.0, 14.0);

//fuzzy suhu
FuzzySet* dingin = new FuzzySet(0.0, 0.0, 23.0, 24.0);
FuzzySet* normal = new FuzzySet(23.0, 24.0, 28.0, 29.0);
FuzzySet* panas = new FuzzySet(28.0, 29.0, 55.0, 55.0);


//untuk PH
#define analogPH A1

//untuk suhu
#define ONE_WIRE_BUS 4

//setting ds18b20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float suhuSekarang;

//setting ph
float ph;
float lalu = 0;
float alpha = 0.04;
int temp;

//untuk ultrasonik
const int trigPin = 9;
const int echoPin = 10;
long duration;
float distance;

//untuk relay
const int relayKeluar = 5;
const int relayMasuk = 6;

float output1 = 0.0;
String jwb;
boolean kuras = false;
boolean isiAir = false;
void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  Wire.onReceive(receiveEvent); /* register receive event */
  Wire.onRequest(requestEvent); /* register request event */
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  digitalWrite(relayKeluar, HIGH);
  digitalWrite(relayMasuk, HIGH);
  pinMode(relayKeluar, OUTPUT);
  pinMode(relayMasuk, OUTPUT);
  Serial.begin(9600);

  // FuzzyInput 1
  FuzzyInput* pHAir = new FuzzyInput(1);
  pHAir->addFuzzySet(asam);
  pHAir->addFuzzySet(netral);
  pHAir->addFuzzySet(basa);

  fuzzy->addFuzzyInput(pHAir);

  // FuzzyInput 2
  FuzzyInput* suhuAir = new FuzzyInput(2);
  suhuAir->addFuzzySet(dingin);
  suhuAir->addFuzzySet(normal);
  suhuAir->addFuzzySet(panas);

  fuzzy->addFuzzyInput(suhuAir);

  // FuzzyOutput
  FuzzyOutput* statusAir = new FuzzyOutput(1);

  FuzzySet* tidakDikuras = new FuzzySet(45, 55, 90, 100);
  statusAir->addFuzzySet(tidakDikuras);
  FuzzySet* dikuras = new FuzzySet(0, 10, 45, 55);
  statusAir->addFuzzySet(dikuras);

  fuzzy->addFuzzyOutput(statusAir);

  //Rule
  FuzzyRuleAntecedent* ifPhAsamDANsuhuDingin = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhAsamDANsuhuNormal = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhAsamDANsuhuPanas = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhNetralDANsuhuDingin= new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhNetralDANsuhuNormal = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhNetralDANsuhuPanas = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhBasaDANsuhuDingin = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhBasaDANsuhuNormal = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* ifPhBasaDANsuhuPanas = new FuzzyRuleAntecedent();

  ifPhAsamDANsuhuDingin->joinWithAND(asam, dingin);
  ifPhAsamDANsuhuNormal->joinWithAND(asam, normal);
  ifPhAsamDANsuhuPanas->joinWithAND(asam, panas);
  ifPhNetralDANsuhuDingin->joinWithAND(netral, dingin);
  ifPhNetralDANsuhuNormal->joinWithAND(netral, normal);
  ifPhNetralDANsuhuPanas->joinWithAND(netral, panas);
  ifPhBasaDANsuhuDingin->joinWithAND(basa, dingin);
  ifPhBasaDANsuhuNormal->joinWithAND(basa, normal);
  ifPhBasaDANsuhuPanas->joinWithAND(basa, panas);

  FuzzyRuleConsequent* thenDikuras = new FuzzyRuleConsequent();
  FuzzyRuleConsequent* thenTidakDikuras = new FuzzyRuleConsequent();
  thenDikuras->addOutput(dikuras);
  thenTidakDikuras->addOutput(tidakDikuras);

  FuzzyRule* fuzzyRule1 = new FuzzyRule(1, ifPhAsamDANsuhuDingin, thenDikuras);
  FuzzyRule* fuzzyRule2 = new FuzzyRule(2, ifPhAsamDANsuhuNormal, thenDikuras);
  FuzzyRule* fuzzyRule3 = new FuzzyRule(3, ifPhAsamDANsuhuPanas, thenDikuras);
  FuzzyRule* fuzzyRule4 = new FuzzyRule(4, ifPhNetralDANsuhuDingin, thenDikuras);
  FuzzyRule* fuzzyRule5 = new FuzzyRule(5, ifPhNetralDANsuhuNormal, thenTidakDikuras);
  FuzzyRule* fuzzyRule6 = new FuzzyRule(6, ifPhNetralDANsuhuPanas, thenDikuras);
  FuzzyRule* fuzzyRule7 = new FuzzyRule(7, ifPhBasaDANsuhuDingin, thenDikuras);
  FuzzyRule* fuzzyRule8 = new FuzzyRule(8, ifPhBasaDANsuhuNormal, thenDikuras);
  FuzzyRule* fuzzyRule9 = new FuzzyRule(9, ifPhBasaDANsuhuPanas, thenDikuras);
  fuzzy->addFuzzyRule(fuzzyRule1);
  fuzzy->addFuzzyRule(fuzzyRule2);
  fuzzy->addFuzzyRule(fuzzyRule3);
  fuzzy->addFuzzyRule(fuzzyRule4);
  fuzzy->addFuzzyRule(fuzzyRule5);
  fuzzy->addFuzzyRule(fuzzyRule6);
  fuzzy->addFuzzyRule(fuzzyRule7);
  fuzzy->addFuzzyRule(fuzzyRule8);
  fuzzy->addFuzzyRule(fuzzyRule9);
  //ph = 7.00;
  //suhuSekarang = 31.78;
}

void loop() {
  distance = ultras();
  ph = phCari();
  suhuSekarang = ambilsuhu();
  Serial.print("jarak : ");
  Serial.println(distance);

  if (kuras == false) {
    if (isiAir == false) {
      Serial.print("pH = ");
      Serial.println(ph);
      Serial.print("Suhu = ");
      Serial.println(suhuSekarang);

      fuzzy->setInput(1, ph);
      fuzzy->setInput(2, suhuSekarang);

      fuzzy->fuzzify();
      Serial.print("Ph : ");
      Serial.print(asam->getPertinence());
      Serial.print(", ");
      Serial.print(netral->getPertinence());
      Serial.print(", ");
      Serial.println(basa->getPertinence());

      Serial.print("Suhu : ");
      Serial.print(dingin->getPertinence());
      Serial.print(", ");
      Serial.print(normal->getPertinence());
      Serial.print(", ");
      Serial.println(panas->getPertinence());

      output1 = fuzzy->defuzzify(1);
      Serial.print("Output = ");
      Serial.println(output1);
      if (output1 <= 50) {
        kuras = true;
        buangAir();
        Serial.println("Air Kuras");
      }
      Serial.println("==============");
      Serial.println();
    }
    else if (isiAir == true) {
      if (distance <= batasAtas) {
        isiAir = false;
        digitalWrite(relayKeluar, HIGH);
        digitalWrite(relayMasuk, HIGH);
        Serial.println("Air Penuh");
        delay(120000);
      }
    }
  }
  else if (kuras == true) {
    if (distance >= batasBawah) {
      kuras = false;
      isiAir = true;
      airMasuk();
      Serial.println("Air Mengisi");
    }
  }
  //-------------------------------------------------------
  jwb = String(ph) + "&" + String(suhuSekarang) + "&" + String(output1);
  delay(500);
}
void buangAir() {
  digitalWrite(relayKeluar, HIGH);
  digitalWrite(relayMasuk, LOW);
}
void airMasuk() {
  digitalWrite(relayKeluar, LOW);
  digitalWrite(relayMasuk, HIGH);
}
float ultras() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  float hasil = duration * 0.034 / 2;
  return hasil;
}
float phCari() {
  int dapat = analogRead(analogPH);
  int buf[10];
  for (int i = 0; i < 10; i++)
  {
    buf[i] = analogRead(analogPH);
    delay(10);
  }
  int avgValue = 0;
  for (int i = 2; i < 8; i++)
    avgValue += (buf[i] - CENTER);
  float pHVol = (float)avgValue * 5.0 / 1024 / 6.0;
  float phValue = pHVol * (-1.33) + 7.0;
  float hsl = 0;
  if (lalu = 0) {
    lalu = phValue;
    hsl = phValue;
  }
  else {
    hsl = alpha * phValue + (1 - alpha) * lalu;
    lalu = hsl;
    hsl = phValue - hsl;
  }
  return hsl;
}
float ambilsuhu() {
  sensorSuhu.requestTemperatures();
  float hasil = sensorSuhu.getTempCByIndex(0);
  return hasil;
}
void receiveEvent(int howMany) {
  while (0 < Wire.available()) {
    char c = Wire.read();      /* receive byte as a character */
    Serial.print(c);           /* print the character */
  }
  Serial.println();             /* to newline */
}
void requestEvent() {
  char buffer[80];
  jwb.toCharArray(buffer, 17);
  Wire.write(buffer);  /*send string on request */
}
