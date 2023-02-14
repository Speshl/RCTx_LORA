#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h>               
#include "HT_SSD1306Wire.h"


#define RF_FREQUENCY                                915000000 // Hz

#define TX_OUTPUT_POWER                             100        // dBm

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        4         // Same for Tx and Rx //default 8
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            100
#define BUFFER_SIZE                                 4 // Define the payload size here

SSD1306Wire  oledDisplay(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

uint8_t steerValue = 0;
uint8_t escValue = 0;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void setup() {
  Serial.begin(115200);
  VextON();
  delay(100);

  // Initialising the UI will init the display too.
  oledDisplay.init();
  oledDisplay.setFont(ArialMT_Plain_10);
  oledDisplay.clear();
  oledDisplay.setTextAlignment(TEXT_ALIGN_LEFT);
  oledDisplay.drawString(0, 0, "Starting Up...");
  oledDisplay.display();
  
  Mcu.begin();

  txNumber=0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                 true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
 oledDisplay.clear();
 oledDisplay.drawString(0, 0, "TX Start Up Complete");
 oledDisplay.display();
}



void loop()
{
	if(lora_idle == true && Serial.available() >= BUFFER_SIZE+2)
	{
    uint8_t buffer[BUFFER_SIZE];
    //Serial.readBytes(buffer, BUFFER_SIZE);    
    if(Serial.read() != 255){ //expect first byte of command to be 255
      return;
    }

    if(Serial.read() != 127){//expect second byte of command to be 127
      return;
    }
    Serial.readBytes(buffer, BUFFER_SIZE); 
    oledDisplay.clear();
    char str[30];
    sprintf(str,"TX: %d %d %d %d",buffer[0],buffer[1],buffer[2],buffer[3]);
    oledDisplay.drawString(0, 0, str);
    oledDisplay.display();

		Radio.Send(buffer, BUFFER_SIZE); //send the package out	
    lora_idle = false;
	}
  Radio.IrqProcess( );
}

void OnTxDone( void )
{
  lora_idle = true;
	//Serial.println("TX done......");
}

void OnTxTimeout( void )
{
  Radio.Sleep( );
  lora_idle = true;
  Serial.println("TX Timeout......");
  oledDisplay.drawString(0, 10, "TX Timeout......");
  oledDisplay.display();
}
