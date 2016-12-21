/* CAN Bus ECU Simulator
 *  
 * www.skpang.co.uk
 *
 * 
 */
 
#include "ecu_sim.h"
#include <FlexCAN.h>

/* C++ wrapper */
ecu_simClass::ecu_simClass() {
 
}

uint8_t ecu_simClass::init(uint32_t baud) {

  Can0.begin(baud);
  ecu.dtc = 0;
  return 0;
}

uint8_t ecu_simClass::update(void) 
{
  CAN_message_t can_MsgRx,can_MsgTx;
   
  if(Can0.read(can_MsgRx)) 
  {
     if (can_MsgRx.id == PID_REQUEST) 
     {
        can_MsgTx.ext = 0; 
        if(can_MsgRx.buf[1] == MODE3) // Request trouble codes
        {
            if(ecu.dtc == false){
                can_MsgTx.buf[0] = 0x02; 
                can_MsgTx.buf[1] = MODE3_RESPONSE;    
                can_MsgTx.buf[2] = 0x00;  
             }else{
                can_MsgTx.buf[0] = 0x06; 
                can_MsgTx.buf[1] = MODE3_RESPONSE;    
                can_MsgTx.buf[2] = 0x02;  
                can_MsgTx.buf[3] = 0x01;  
                can_MsgTx.buf[4] = 0x00;                
                can_MsgTx.buf[5] = 0x02;
                can_MsgTx.buf[6] = 0x00;                
             }
             can_MsgTx.id = PID_REPLY;
             can_MsgTx.len = 8; 
             Can0.write(can_MsgTx);
        }
      
        if(can_MsgRx.buf[1] == MODE4) // Clear trouble codes, clear Check engine light
        {
            ecu.dtc = false;  
      
            can_MsgTx.buf[0] = 0x00; 
            can_MsgTx.buf[1] = MODE4_RESPONSE; 
            can_MsgTx.id = PID_REPLY;
            can_MsgTx.len = 8; 
            Can0.write(can_MsgTx);  
        }
        
        if(can_MsgRx.buf[1] == MODE1)
        {
            can_MsgTx.id = PID_REPLY;
            can_MsgTx.len = 8; 
            can_MsgTx.buf[1] = MODE1_RESPONSE;
            
            switch(can_MsgRx.buf[2])
            {   /* Details from http://en.wikipedia.org/wiki/OBD-II_PIDs */
                case PID_SUPPORTED:
                    can_MsgTx.buf[0] = 0x06;  
                    can_MsgTx.buf[2] = PID_SUPPORTED; 
                    can_MsgTx.buf[3] = 0xE8;
                    can_MsgTx.buf[4] = 0x19;
                    can_MsgTx.buf[5] = 0x30;
                    can_MsgTx.buf[6] = 0x12;
                    can_MsgTx.buf[5] = 0x00;
                    Can0.write(can_MsgTx);  
                    break;
                
                case MONITOR_STATUS:
                    can_MsgTx.buf[0] = 0x05;  
                    can_MsgTx.buf[2] = MONITOR_STATUS; 
                    if(ecu.dtc == true) can_MsgTx.buf[3] = 0x82;
                        else can_MsgTx.buf[3] = 0x00;
                    
                    can_MsgTx.buf[4] = 0x07;
                    can_MsgTx.buf[5] = 0xFF;
                    Can0.write(can_MsgTx);      
                    break;
                        
                case ENGINE_RPM:              //   ((A*256)+B)/4    [RPM]
                    can_MsgTx.buf[0] = 0x04;  
                    can_MsgTx.buf[2] = ENGINE_RPM; 
                    can_MsgTx.buf[3] = (ecu.engine_rpm & 0xff00) >> 8;
                    can_MsgTx.buf[4] = ecu.engine_rpm & 0x00ff;
                    Can0.write(can_MsgTx);
                    break;
                               
                case ENGINE_COOLANT_TEMP:     //     A-40              [degree C]
                    can_MsgTx.buf[0] = 0x03;  
                    can_MsgTx.buf[2] = ENGINE_COOLANT_TEMP; 
                    can_MsgTx.buf[3] = ecu.coolant_temp;
                    Can0.write(can_MsgTx);
                    break;
                               
                case VEHICLE_SPEED:         // A                  [km]
                    can_MsgTx.buf[0] = 0x03;  
                    can_MsgTx.buf[2] = VEHICLE_SPEED; 
                    can_MsgTx.buf[3] = ecu.vehicle_speed;
                    Can0.write(can_MsgTx);
                    break;
    
                case MAF_SENSOR:               // ((256*A)+B) / 100  [g/s]
                    can_MsgTx.buf[0] = 0x04;  
                    can_MsgTx.buf[2] = MAF_SENSOR; 
                    can_MsgTx.buf[3] = (ecu.maf_airflow & 0xff00) >> 8;
                    can_MsgTx.buf[4] =  ecu.maf_airflow & 0x00ff;
                    Can0.write(can_MsgTx);
                    break;
    
                case O2_VOLTAGE:            // A * 0.005   (B-128) * 100/128 (if B==0xFF, sensor is not used in trim calc)
                    can_MsgTx.buf[0] = 0x04;  
                    can_MsgTx.buf[2] = O2_VOLTAGE; 
                    can_MsgTx.buf[3] = ecu.o2_voltage & 0x00ff;
                    can_MsgTx.buf[4] = (ecu.o2_voltage & 0xff00) >> 8;
                    Can0.write(can_MsgTx);
                    break;;
                   
                case THROTTLE:            //
                    can_MsgTx.buf[0] = 0x03;  
                    can_MsgTx.buf[2] = THROTTLE; 
                    can_MsgTx.buf[3] = ecu.throttle_position;
                    Can0.write(can_MsgTx);
                    break;
              }//switch
          }
       }
    }
   return 0;
}
     
ecu_simClass ecu_sim;
