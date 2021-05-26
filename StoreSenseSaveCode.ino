#include <dht.h>

dht DHT;

#define DHT11_PIN    8
#define relay1_pin1  2
#define relay2_pin2  3
#define pinTrig      11
#define pinEcho      10
#define pirPin       4
#define SW           7

int distance = 0;
int Container_Size =0;
byte pirFlag=0 ,auto_manual_flag = 0,fan_flag=0 , light_flag =0 , Switch_Flag=0;

int crnt_match_point = 0;
char browser_LinkID = 0,App_LinkID = 0; 
char AppContDataSending = 0;
int ContSending_timer = 0;
unsigned int Humidity = 0,Temperature = 0;

int base_ms = 0;

void setup() {
  // put your setup code here, to run once:
Serial1.begin(115200);
Serial.begin(9600);
//mySerial.begin (115200);
pinMode(pirPin,INPUT);
pinMode(pinTrig,OUTPUT);
pinMode(pinEcho,INPUT);
pinMode(SW,INPUT_PULLUP);
pinMode(relay1_pin1,OUTPUT);
pinMode(relay2_pin2,OUTPUT);

digitalWrite(relay1_pin1,LOW);
digitalWrite(relay2_pin2,LOW);

if(!AT_test())              Serial.println("WifiModule Error");
if(!Server_setup())         Serial.println("Connection Error");

}

void loop() {
  // put your main code here, to run repeatedly:
int chk;
GET_NEW_DATA(); 

  delay(1);
  base_ms++;
  if(base_ms < 1000)  return;
  base_ms = 0;

  
  if(!digitalRead(SW))      Switch_Flag= 1;
  else                      Switch_Flag= 0;
  

 if(Switch_Flag == 0 )
  {       chk = DHT.read11(DHT11_PIN);
          Temperature = DHT.temperature;
          Humidity = DHT.humidity;
          ultrasonicSensor(); 
         
          if(digitalRead(pirPin))                 pirFlag=1;
          else                                    pirFlag=0;
        
            if((pirFlag==1)&&(Temperature < 35))
            {  if( auto_manual_flag == 0)
            {                                                                                                                                                                                                       
                digitalWrite(relay1_pin1,HIGH);
                digitalWrite(relay2_pin2,LOW);
                delay(2000);
            }
            }
                
            if((pirFlag==1)&&(Temperature > 35))
             { if( auto_manual_flag == 0)
             {
                digitalWrite(relay1_pin1,HIGH);
                digitalWrite(relay2_pin2,HIGH);
                delay(2000);
              }
          }
  
  if(fan_flag == 1)         digitalWrite(relay2_pin2,HIGH);
  else                      digitalWrite(relay2_pin2,LOW);
 
  if(light_flag == 1)       digitalWrite(relay1_pin1,HIGH);
  else                      digitalWrite(relay1_pin1,LOW);
}

  delay(500);

SEND_DATA();

}

void ultrasonicSensor(void)
{
  digitalWrite(pinTrig,LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig,HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig,LOW);

  distance = microsecondsToInches(pulseIn(pinEcho,HIGH));
  if(distance < 10)
   { Container_Size = (10 - distance)*10;
   }
   else  Container_Size = 0;  
}

//-------------wifi code

void GET_NEW_DATA(void)
{ if (Serial1.available() == 0) return;
  String data = "";
  data = readData(5000);
  byte temp = 0;
  char str_to_char_buff[50];   
  data.toCharArray(str_to_char_buff,50);
  Serial.println(data);
  if(compare_string(data, (char *)"+IPD,",5))
  {
      if(compare_string(data, (char *)"APP_Q",5))    auto_manual_flag = 0;
        if(compare_string(data, (char *)"APP_W",5))   auto_manual_flag = 1;
        if(auto_manual_flag == 1)
        {
          if(compare_string(data, (char *)"APP_E",5))   light_flag = 1;
          if(compare_string(data, (char *)"APP_R",5))   light_flag = 0;
          if(compare_string(data, (char *)"APP_T",5))   fan_flag = 1;
          if(compare_string(data, (char *)"APP_Y",5))   fan_flag = 0;
        }

  } 
}


bool Server_setup(void)
{ AT_RESTORE();
  AT_CWMODE(3);
  AT_CIPMUX();
 // if(!AT_CWSAP("Warehouse","1234567890"))  return false;
    if(!AT_CWJAP("Jeet","ganeshaabc123"))    return false;
 // if (!AT_CIPSERVER(1234))                     return false;
  return true;
}

byte SEND_DATA(void)
{ if(!AT_CIPSTART())
  {  return 0;
  }
  if(!AT_CIPSEND())     
  {// lcd.setCursor(0,1);
    Serial.println(" Data  Not Send ");
    return 0;
  }
  Serial.println("  Data Sent     ");
  return 1; 
 
}
bool AT_CIPSTART(void)
{ String data = "";
  bool return_flag = 0;
 Serial1.println("AT+CIPSTART=1,\"TCP\",\"192.168.43.38\",80");
  data = readData(5000);
  return_flag = compare_string(data,(char *)"1,CONNECT",9); 
  return(return_flag);
}


//http://192.168.0.57/1659/warehouse_management/data.php?A=0&B=0&C=0&D=0
bool AT_CIPSEND(void)
{ 
String Senddata="GET/1659/warehouse_management/data.php?A="+String(Container_Size)+"&B="+String( pirFlag)+"&C="+String(Temperature)+"&D="+String(Humidity)+"&E="+String(Switch_Flag)+" HTTP/1.1\r\nHost: 192.168.43.38\r\n\r\n";
  int Str_length = Senddata.length();
  String data = "";
   Serial1.print("AT+CIPSEND=1,");
   Serial1.println(Str_length,DEC);
  data = readData(2000);
  if(compare_string(data,(char *)">",1))
  {
    Serial1.println(Senddata);
    Serial.println(Senddata);
    data = readData(5000);
    Serial.println(data);
    char str_to_char_buff[200]; 
    data.toCharArray(str_to_char_buff,200);
    if(compare_string(data,(char *)"SEND OK",7))
    { 
      if(compare_string(data, (char *)"APP_Q",5))    auto_manual_flag = 0;
        if(compare_string(data, (char *)"APP_W",5))   auto_manual_flag = 1;
        if(auto_manual_flag == 1)
        {
          if(compare_string(data, (char *)"APP_E",5))   light_flag = 1;
          if(compare_string(data, (char *)"APP_R",5))   light_flag = 0;
          if(compare_string(data, (char *)"APP_T",5))   fan_flag = 1;
          if(compare_string(data, (char *)"APP_Y",5))   fan_flag = 0;
        
        
        }
      return true;
    }
  }
  return false;
}



bool AT_test(void)
{ String data = "";
  Serial1.println(F("AT"));
  data = readData(2000);
  return(compare_string(data,(char *)"OK",2));
}

void AT_RESTORE(void)
{ Serial1.println(F("AT+RESTORE"));
  readData(500);
}

void AT_UART(long Uart_value)
{ Serial1.println("AT+UART="+String(Uart_value)+",8,1,0,0");
  readData(2000);
}


void AT_RST(void)
{ Serial1.println(F("AT+RST"));
  readData(500);
}

void AT_CWMODE(char Mode)
{Serial1.print(F("AT+CWMODE="));
 Serial1.println(Mode,DEC);
  readData(500);
}
void AT_CIPMUX(void)
{ String data = "";
 Serial1.println(F("AT+CIPMUX=1"));
  data = readData(500);
}

bool AT_CWSAP(String My_WifiName,String My_Wifipswd)
{ String data = "";
  Serial1.print(F("AT+CWSAP=\""));
  Serial1.print(My_WifiName);
  Serial1.print(F("\",\""));
  Serial1.print(My_Wifipswd);
  Serial1.println(F("\",5,3"));
  data = readData(5000);
  return (compare_string(data, (char *)"OK", 2));
}

bool AT_CWJAP(String ipaddr,String passwrd)
{ String data = "";
bool return_flag = 0;
  Serial1.print("AT+CWJAP=\"");
  Serial1.print(ipaddr);
  Serial1.print("\",\"");
  Serial1.print(passwrd);
  Serial1.println("\"");
  data = readData(10000);
  if(compare_string(data,(char *)"WIFI DISCONNECT",15)) return(0);
  return_flag = compare_string(data,(char *)"WIFI CONNECTED",14); 
  return(return_flag);
}

bool AT_CIPSERVER(int port_num)
{ String data = "";
  Serial1.print(F("AT+CIPSERVER=1,"));
  Serial1.println(port_num,DEC);
  data = readData(500);
  return(compare_string(data,(char *)"OK",2));    
}


bool AT_CIPCLOSE(char linkID)
{ String data = "";
  Serial1.print(F("AT+CIPCLOSE="));
  Serial1.println(linkID);
  data = readData(500);
  return(compare_string(data,(char *)"OK",2));    
}

bool compare_string(String main_str,char *sub_str,char String2_size)
{ int main_str_length = 0,i = 0;
  main_str_length  = main_str.length();
  while(main_str_length != i)
  { if(main_str.substring(i,i + String2_size) == sub_str)
    { crnt_match_point = i+String2_size;
      return true; 
    }                                              
    i++;
  }
  return false;
}

String readData(unsigned long timeout) 
{   String data = "";
    unsigned long t = millis();
    int i = 0;
    while(millis() - t < timeout) 
    { if(Serial1.available() > 0) 
      {   char r = Serial1.read();
          if(i < 50)  data += r;  
          t = millis();
          i++;
      }
    }
 Serial.println(data);
    return data;
}

long microsecondsToInches(long microseconds)
{ // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  return microseconds / 74 / 2;
}
