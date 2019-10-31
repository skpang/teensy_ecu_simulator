/* CAN Bus ECU Simulator
 *  
 * www.skpang.co.uk
 * 
 * V1.0 Dec 2016
 *   
 * Make sure the fonts are installed first
 * https://github.com/PaulStoffregen/ILI9341_fonts
 * 
 * Put the font files in /hardware/teensy/avr/libraries/ILI9341_t3 folder
 * 
 * Also requres new FlexCAN libarary
 * https://github.com/collin80/FlexCAN_Library
 * 
 * 
 */
#include <FlexCAN.h>
#include <Encoder.h>
#include <Metro.h>
#include "ecu_sim.h"

#include "SPI.h"
#include "ILI9341_t3.h"
#include "font_Arial.h"
#include "font_LiberationMono.h"
#include "font_CourierNew.h"

// Menu defines
#define L0_MAIN_MENU   0
#define L0_RPM         1
#define L0_COOLANT     2
#define L0_THROTTLE    3
#define L0_SPEED       4
#define L0_MAF         5
#define L0_O2          6
#define L0_DTC         7
#define JOG_INC        0
#define JOG_DEC        1

#define SCK_PIN   13  //Clock pin
#define MISO_PIN  12  //Mater in Slave output
#define MOSI_PIN  11  //Master out Slave input
#define SD_PIN    10  //pin for SD card control
#define TFT_DC 9
#define TFT_CS 2
#define MENU_OFFSET   50
#define LINE_SPACING  15
#define VALUE_OFFSET  110
#define FONT_SIZE     11
#define CURSOR_OFFSET 46
#define CURSOR_LEN    12

const int JOY_LEFT = 0;
const int JOY_RIGHT= 6;
const int JOY_CLICK = 5;
const int JOY_UP  = 7;
const int JOY_DOWN = 1;
const int JOG_A = 20;
const int JOG_B = 19;

uint8_t menu_state;
uint8_t old_menu_state;
long wheel_position = 0;
long old_wheel_position = 0;

ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);
Encoder jog(JOG_A, JOG_B);
ecu_t ecu;

void setup() {

  pinMode(JOY_LEFT, INPUT_PULLUP);
  pinMode(JOY_RIGHT, INPUT_PULLUP);
  pinMode(JOY_CLICK, INPUT_PULLUP);
  pinMode(JOY_UP, INPUT_PULLUP);
  pinMode(JOY_DOWN, INPUT_PULLUP); 
  
  tft.begin();                        // Setup TFT
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);

  tft.setCursor(0, 0);
  tft.println("ECU Simulator v1.1 skpang.co.uk 10/19");

  tft.drawLine(0,20,340,20,ILI9341_DARKGREY);    //Horizontal line
  tft.setFont(LiberationMono_10);

  tft.setCursor(0, MENU_OFFSET + (LINE_SPACING * 0));
  tft.println("Engine rpm"); 
  tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * 0));
  tft.print("0 rpm");
  
  tft.setCursor(0, MENU_OFFSET + (LINE_SPACING * 1));
  tft.println("Coolant temp ");
  tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * 1));
  tft.print("0 C");
  
  tft.setCursor(0, MENU_OFFSET + (LINE_SPACING * 2));
  tft.println("Throttle ");
  tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * 2));  
  tft.print("0 %");  
  
  tft.setCursor(0, MENU_OFFSET + (LINE_SPACING * 3));
  tft.println("Vehicle speed ");
  tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * 3));  
  tft.print("0 km");  
  
  tft.setCursor(0, MENU_OFFSET + (LINE_SPACING * 4));
  tft.println("MAF airflow ");
  tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * 4));  
  tft.print("0 g/s");  
  
  tft.setCursor(0, MENU_OFFSET + (LINE_SPACING * 5));
  tft.println("O2 voltage ");
  tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * 5));  
  tft.print("0    ");

  tft.setCursor(0, MENU_OFFSET + (LINE_SPACING * 6));
  tft.println("DTC ");
  tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * 6));  
  tft.print("0  ");
   
  ecu_sim.init(500000);
  delay(900);
    
  menu_state = L0_MAIN_MENU;
  Serial.println(F("ECU Simulator v1.0 SK Pang 12/16"));
  for(int i=0;i<8;i++)
  {
    menu_state = i;
    update_param(JOG_INC);
    
  }

}

void loop() {
    
  ecu_sim.update();
  wheel_position = jog.read();

  if(wheel_position != old_wheel_position)
  {
    if(wheel_position < old_wheel_position)
    {
       update_param(JOG_DEC);
    }else  update_param(JOG_INC);
 
    old_wheel_position = wheel_position; 
    
  }
  
  if(digitalRead(JOY_DOWN) == 0)
  {
    delay(10);
    if(menu_state != L0_DTC) menu_state++;
    update_menu(); 
  }
  
  if(digitalRead(JOY_UP) == 0)
  {
    delay(10);
    if(menu_state != L0_MAIN_MENU) menu_state--;
    update_menu(); 
  }

}

void clear_value(void)
{
   tft.fillRect(VALUE_OFFSET,CURSOR_OFFSET + (LINE_SPACING * menu_state) - FONT_SIZE,100,15,ILI9341_BLACK); // Clear area
   tft.setCursor(VALUE_OFFSET, MENU_OFFSET + (LINE_SPACING * (menu_state-1)));
  
}

void update_param(unsigned char dir)
{
  int a;
  int b;
 
    switch(menu_state)
    {
     
        case L0_MAIN_MENU:
            
            break;
            
        case L0_RPM:
            if((dir == JOG_INC) & (ecu.engine_rpm < 16384)){
              ecu.engine_rpm = ecu.engine_rpm +10;
            } else if (ecu.engine_rpm >= 10)
            {
              ecu.engine_rpm = ecu.engine_rpm - 10;
            }
          
            clear_value();
            
            a =   (ecu.engine_rpm & 0xff00) >> 8;
            b =  ecu.engine_rpm & 0x00ff;
            tft.print(((256*a) +b)/4); 
            tft.print(" rpm"); 
            break;
            
        case L0_COOLANT:
           
            if((dir == JOG_INC) & (ecu.coolant_temp < 215)) {
                ecu.coolant_temp++;
            } else if (ecu.coolant_temp > 0) ecu.coolant_temp--;
            
            clear_value();
            tft.print(ecu.coolant_temp -40); 
            tft.print(" C");            
              
            break;
            
        case L0_THROTTLE:
            if((dir == JOG_INC) & (ecu.throttle_position <= 100)){
                ecu.throttle_position++;
            } else if( ecu.throttle_position >0 ) ecu.throttle_position--;
            
            clear_value();
            tft.print(ecu.throttle_position); 
            tft.print(" %");
                                       
            break;
        
        case L0_SPEED:
            if((dir == JOG_INC) & (ecu.vehicle_speed < 255)){
                ecu.vehicle_speed++;
            } else if (ecu.vehicle_speed > 0) ecu.vehicle_speed--;
            clear_value();  
            tft.print(ecu.vehicle_speed); 
            tft.print(" km");

            break;
            
        case L0_MAF:
            if((dir == JOG_INC) & (ecu.maf_airflow < 655)){
                ecu.maf_airflow = ecu.maf_airflow +10;
            } else if (ecu.maf_airflow > 0) ecu.maf_airflow = ecu.maf_airflow - 10;
            
            clear_value();     
                    
            a =   (ecu.maf_airflow & 0xff00) >> 8;
            b =  ecu.maf_airflow & 0x00ff;
                   
            tft.print(float(((256*a) +b)/float (100))); 
            tft.print(" g/s");        
               
            break;
        case L0_O2:
            if((dir == JOG_INC) & (ecu.o2_voltage < 100)){
                ecu.o2_voltage++;
            } else if (ecu.o2_voltage > 0 ) ecu.o2_voltage--;
            
            clear_value();            
            tft.setCursor(VALUE_OFFSET, 125);
            tft.print(ecu.o2_voltage); 
            tft.print("    ");
                        
            break;
            
        case L0_DTC:
            if(dir == JOG_INC){
                ecu.dtc = true;
               
            } else {
                ecu.dtc = false;
               
            }        
            clear_value();
            tft.print(ecu.dtc);             
            break;
            
    }

}

void update_menu(void)
{
 
   switch(menu_state)
    {
        case L0_MAIN_MENU:
            
            break;
            
        case L0_RPM:
            tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (old_menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (old_menu_state * LINE_SPACING),ILI9341_BLACK);   //Erase old underline
            tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (menu_state * LINE_SPACING),ILI9341_YELLOW);   //Draw new underline
            old_menu_state = menu_state;
            break;
            
        case L0_COOLANT:
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (old_menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (old_menu_state * LINE_SPACING),ILI9341_BLACK);   //Erase old underline
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (menu_state * LINE_SPACING),ILI9341_YELLOW);//Draw new underline             
              old_menu_state = menu_state;
            break;
            
        case L0_THROTTLE:
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (old_menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (old_menu_state * LINE_SPACING),ILI9341_BLACK);   //Erase old underline
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (menu_state * LINE_SPACING),ILI9341_YELLOW);//Draw new underline         
              old_menu_state = menu_state;              
            break;
        
        case L0_SPEED:
               tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (old_menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (old_menu_state * LINE_SPACING),ILI9341_BLACK);   //Erase old underline
               tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (menu_state * LINE_SPACING),ILI9341_YELLOW); //Draw new underline          
              old_menu_state = menu_state;               
            break;
            
        case L0_MAF:
               tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (old_menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (old_menu_state * LINE_SPACING),ILI9341_BLACK);   //Erase old underline
               tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (menu_state * LINE_SPACING),ILI9341_YELLOW); //Draw new underline
              old_menu_state = menu_state; 
            break;              
            
        case L0_O2:
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (old_menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (old_menu_state * LINE_SPACING),ILI9341_BLACK);   //Erase old underline
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (menu_state * LINE_SPACING),ILI9341_YELLOW); //Draw new underline         
              old_menu_state = menu_state;                
            break;
            
        case L0_DTC:
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (old_menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (old_menu_state * LINE_SPACING),ILI9341_BLACK);   //Erase old underline
              tft.drawLine(VALUE_OFFSET,CURSOR_OFFSET + (menu_state * LINE_SPACING),(VALUE_OFFSET + CURSOR_LEN),CURSOR_OFFSET + (menu_state * LINE_SPACING),ILI9341_YELLOW); //Draw new underline         
              old_menu_state = menu_state;              
            break;
            
    }

  //Wait until button is released
  while(digitalRead(JOY_UP) ==0);
  while(digitalRead(JOY_DOWN) ==0);
  delay(10);

}
