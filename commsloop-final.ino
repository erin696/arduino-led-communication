// program demonstrating communication between a transmitter and reciever while utalizing an ultrasonic sensor to measeure distance and displayed on lcd screen
#include <LiquidCrystal.h> // library for controlling led display


#define STX 0x70 // defines start and end of text for communication
#define ETX 0x71

const int trigPin = 4; // sets pins for ultrasonic sensor
const int echoPin = 5;
const int ledPin = 3;
long duration; // variable to store time by the ultrasonic signal


LiquidCrystal lcd(7, 8, 9, 10, 11,12); //object installation 

char txButton, txTilt, txPot, txA, txB, txC, txD;
char txBuffer[8] = {0, 0, 0, 0, 0, 0, 0, ETX}; // transmit buffer- temporary storage area to hold data during communication- prepared here by getTxChar()
char rxBuffer[7]; // recieve buffer- holds incoming data before its processed- read into rxBuffer[] by rxChar()
char rxButton, rxTilt, rxPot, rxA, rxB, rxC, rxD;
int rx_index;

void readInputs() {  // input handling
  digitalWrite(trigPin, LOW); 
  digitalWrite(trigPin, HIGH);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  txA = duration * 0.034 / 2;

}

void writeOutputs() { // output handling 
  unsigned int distance = rxA;
  lcd.setCursor(0, 0);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print(" cm");
  lcd.setCursor(0, 1);

}

char encrypt(char in_char) { // placeholder functions to encrypt data, currently no mofification
  char out_char;
  out_char = in_char;
  return out_char;
}

char decrypt(char in_char) {
  char out_char;
  out_char = in_char;
  return out_char;
}

void setup() { // initialises pins, serial communication and lcd
  pinMode(3, OUTPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT); 
  lcd.begin(16, 2);
}

const long txInterval = 200;
int tx_state = 0;
char tx_char = 'H';
char chr;
unsigned long previousTxMillis = 0;
char tx_string[] = "Hello World";
#define TX_START_OF_TEXT -1
int tx_string_state = TX_START_OF_TEXT;

char getTxChar() { // prepares data for transmission- sends elements from txbuffer
  char chr;
  switch (tx_string_state) {
    case TX_START_OF_TEXT:
      tx_string_state = 0;
      txBuffer[0] = txButton;
      txBuffer[1] = txTilt;
      txBuffer[2] = txPot;
      txBuffer[3] = txA;
      txBuffer[4] = txB;
      txBuffer[5] = txC;
      txBuffer[6] = txD;
      return STX;
      break;

    default:
      chr = txBuffer[tx_string_state];
      tx_string_state++;
      if (chr == ETX) {
        tx_string_state = TX_START_OF_TEXT;
        return ETX;
      } else {
        return chr;
      }
      break;
  }
}

void txChar() { // bit by bit transmission of a character- waits for txInterval between
  unsigned long currentTxMillis = millis();
  if (currentTxMillis - previousTxMillis >= txInterval) {
    previousTxMillis = previousTxMillis + txInterval;
    switch (tx_state) {
      case 0:
        chr = encrypt(getTxChar());
        digitalWrite(3, HIGH);
        tx_state++;
        break;

      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
        if ((chr & 0x40) != 0) {
          digitalWrite(3, HIGH);
        } else {
          digitalWrite(3, LOW);
        }
        chr = chr << 1;
        tx_state++;
        break;

      case 8:
        digitalWrite(3, HIGH);
        tx_state++;
        break;

      default:
        digitalWrite(3, LOW);
        tx_state++;
        if (tx_state > 20) tx_state = 0;
        break;
    }
  }
}

const long rxInterval = 20;
int rx_state = 0;
char rx_char;
unsigned long previousRxMillis = 0;
int rx_bits[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void rxChar() { // reception handler from a sensor- reads from a0 and stores bit by bit- stx resets buffer index then etx completes message and updates variables
  unsigned long currentRxMillis = millis();
  int sensorValue;
  int i;

  if (currentRxMillis - previousRxMillis >= rxInterval) {
    previousRxMillis = previousRxMillis + rxInterval;
    sensorValue = analogRead(A0);

    switch (rx_state) {
      case 0:
        if (sensorValue >= 600) {
          rx_bits[0]++;
          rx_state++;
        }
        break;

      case 100:
        if ((rx_bits[0] >= 6) && (rx_bits[8] >= 6)) {
          rx_char = 0;

          for (i = 1; i < 8; i++) {
            rx_char = rx_char << 1;
            if (rx_bits[i] >= 6) rx_char = rx_char | 0x01;
          }
          rx_char = decrypt(rx_char);
          Serial.println(rx_char + 0);

          switch (rx_char) {
            case STX:
              rx_index = 0;
              break;
             
            case ETX:
              rxButton = rxBuffer[0];
              rxTilt = rxBuffer[1];
              rxPot = rxBuffer[2];
              rxA = rxBuffer[3];
              rxB = rxBuffer[4];
              rxC = rxBuffer[5];
              rxD = rxBuffer[6];
              rx_index = 0;
              break;
             
            default:
              rxBuffer[rx_index] = rx_char;
              rx_index++;
              break;
          }
        } else {
          Serial.println("Rx error");
        }
        for (i = 0; i < 10; i++) {
          rx_bits[i] = 0;
        }
        rx_state = 0;
        break;

      default:
        if (sensorValue >= 600) {
          rx_bits[rx_state / 10]++;
        }
        rx_state++;
        break;
    }
  }
}

void loop() { // runs always- handles main operations- communication loop where data is transmitted, recieved and proccessed 
  readInputs(); //reads input
  txChar(); // transmits data 
  rxChar(); // recieves data
  writeOutputs(); // outputs of the ldc
  delay(1000);
}
