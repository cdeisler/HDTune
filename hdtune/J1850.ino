//
// Logging format:
// $DLOBD,prio,single/three,inframe,adressmode,messagetype,data0,data1,data2...,*00
//

#include <avr/pgmspace.h>

#define SIZE_BUFS  (1+2*12)
#define NOOF_BUFS  2
#define NOT_STORING 99

char    j1850buf [NOOF_BUFS][SIZE_BUFS];  // buffers

uint8_t bufIdx = 0;
uint8_t index = NOT_STORING;
char*   j1850message = 0;

boolean J1850_setup(void)
{
  Serial.println(F("J1850 Init..."));
  j1850Serial.begin(38400);
  index = NOT_STORING;
  bufIdx = 0;
  Serial.println(F("J1850 Init done!"));
  return true;
}


void J1850_loop() 
{
  if (j1850Serial.available())
  {
    //Serial.println(F("j1850 avail"));
    char c = j1850Serial.read();
    // sync char
    if (c == '>' && index == NOT_STORING) {
      index = 0;
      return;
    }
    if (index != NOT_STORING) {
      // store the character
      j1850buf[bufIdx][index++] = c;
      if (index >= SIZE_BUFS) {
        // too long message, reset
        index = NOT_STORING;
      } 
      else if (c < ' ') {
        // terminate and set buffer length
        j1850buf[bufIdx][index-1] = 0;
        // update message pointer
        j1850message = j1850buf[bufIdx];
        // change buffer and prepare for next message
        bufIdx = ++bufIdx % NOOF_BUFS;
        index = NOT_STORING;
      }
    } 
  }
}

char* MapFormatToString(uint8_t format) {
    switch (format) {
        case SPD:
            return "SPD";
            //break;
        case RPM:
            return "RPM";
            //sprintf(buffer, "%c\n", RPM);
            //break;
        case GEAR:
            return "GEAR";
            //sprintf(buffer, "%c\n", GEAR);
            //break;
        case FUELGAGE:
            return "FUEL";
        case ENGTEMP:
            return "ENGTMP";
        case TURNSIG:
            return "TURNSIG";
        default:
            return "UNK";
            //sprintf(buffer, "Unknown\n");
    }

}
//
// Get the latest message as a NMEA sentence, message is cleared after this call
//
boolean J1850_getMessage(char* message)
{  
  if (j1850message == 0) return false;

  // NMEA message type  
  strcpy_P(message, (const prog_char*)F("$DLOBD,"));
  // append hex codes from OBD2 decoder
  strcat(message, j1850message);
  strcat_P(message, (const prog_char*)F(",*00"));
  // mark as used
  j1850message = 0;
  return true;
}

boolean J1850_getRawMessage(char* message)
{  
  if (j1850message == 0) return false;

  char *tmp = j1850message;
  strcpy(message, j1850message);
  j1850message = 0;
  return true;
}

unsigned int rpm = 0;
uint8_t speed = 0;
uint8_t gear = 0;
uint8_t engtemp = 0;


j1850mesg_t J1850_getMessageType() {
  if (j1850message == 0) return NOMSG;

  if (strstr_P(j1850message, (const prog_char*)F("2B1B1002"))) {
    rpm = hex2uint16(j1850message+8);
    return RPM;
  }
  if (strstr_P(j1850message, (const prog_char*)F("48291002"))) {
    speed = hex2uint16(j1850message+8);
    return SPD;
  }
  if (strstr_P(j1850message, (const prog_char*)F("A8491010"))) {
    engtemp = hex2uint8(j1850message+8);
    return ENGTEMP;
  }
  if (strstr_P(j1850message, (const prog_char*)F("A83B1003"))) {
    uint8_t tmp = hex2uint8(j1850message+8);
    gear = 0;
    if (tmp != 0) {
      while ((tmp >>= 1) != 0) gear++;
    }
    return GEAR;
  }
  if (strstr_P(j1850message, (const prog_char*)F("A88361"))) {
    return FUELGAGE;
  }
  if (strstr_P(j1850message, (const prog_char*)F("48DA40"))) {
    return TURNSIG;
  }
  return UNKNOWN;
}








