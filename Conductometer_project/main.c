/* ========================================
 * Conductometer source code for PSOC 5lp prototyping board

 * Copyright (C) 2020 Federico Vivaldi
 * This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>
#include "stdlib.h"
#include <math.h>
#include "ssd1306.h"


#define DISPLAYADDRESS 0x3C
#define MAX_CONST -32767
#define MIN_CONST  32767
#define TARGET_FREQ 100.0
#define VTOTCH 0
#define VUCH 1
#define VOFFCH 2
#define SAMPLING_SIZE 100
#define REFRESISTANCE1 10000
#define HOLDERSIZE 20


// functions prototypes //
void components_init(void);
void sincheck(uint8 channel);
void min_max_sinanalysis(void);
double filter (double resistance);
void oled_print (double resistance);
int16 * signal_sampling(uint8);
int16 MIN (int16 *voltage_array);
int16 MAX (int16 *voltage_array);

//global variables //
char stg_conductivity_buffer[100];
int16  oled_refresh_counter = 0;

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /*components initialization */
    components_init();
    UART_PutString("Conductometer V15");
    
    //sincheck(Vtotch); //enable if signal check is required
    //sincheck(Vuch);
    
    for(;;)
    {   
        
        min_max_sinanalysis();
    }
}




/*
    Components initializer
*/
void components_init(void){
    
    ADC_Start();
    UART_Start();
    AMux_Start();
    VG_Start();
    I2COLED_Start();
    VG_buffer_Start();
    display_init(DISPLAYADDRESS);
}



/*
    This function samples the selected channel and print to terminal the potential
*/
void sincheck(uint8 channel){
    
    AC_source_Start();
    int16 *buffer = signal_sampling(channel);
    
    for (uint32_t i = 0; i < SAMPLING_SIZE; i++){
        sprintf(stg_conductivity_buffer, "%d\r\n", *(buffer + i));
        UART_PutString(stg_conductivity_buffer);            
    } 
    AC_source_Stop();
}




/*
    This function samples the ac signal voltage drop on the reference resistor and
    cell and determine the cell resistance as Vcell/Vref * Rref. This measurement
    is therefore dependent on the determination of the real value of Rref. In order
    to avoid this problem, high precision reference resistors must be used
    and a calibration of the device is suggested using high precision resistors.

*/

void min_max_sinanalysis(void){
    double cell_resistance;
    float VH, Vu, Voff; //Source, output, offset and buffer voltage
    int16 maxVH, minVH, maxVu, minVu, maxVoff,minVoff;
    maxVH = maxVoff = maxVu = MAX_CONST;
    minVH = minVu = minVoff = MIN_CONST;
    
    //Vh measurements
    int16 *buffer = signal_sampling(VTOTCH);
    maxVH = MAX(buffer);
    minVH = MIN(buffer);
    //Vu measurements
    buffer = signal_sampling(VUCH);
    maxVu = MAX(buffer);
    minVu = MIN(buffer);
    //Voffset
    buffer = signal_sampling(VOFFCH);
    maxVoff = MAX(buffer);
    minVoff = MIN(buffer);        
    
    VH = (maxVH - minVH);
    Vu = (maxVu - minVu);
    Voff = (maxVoff - minVoff);
    cell_resistance = (float)(((Vu - Voff)*REFRESISTANCE1)/(VH - Voff));
    
    // results is filtered by a feed forward ->moving average FIR digital filter
    cell_resistance = filter(cell_resistance);
    
    //print to terminal if connected, baud rate 115200
    sprintf(stg_conductivity_buffer, "%.2f\r\n", cell_resistance);
    UART_PutString(stg_conductivity_buffer);
    
    //print resistance on oled if connected once every two seconds in order to prevent over refresh
    if (oled_refresh_counter == 50)
    {
        oled_print(cell_resistance);
        oled_refresh_counter = 0;
    }
    oled_refresh_counter += 1;
}



/*

    This function samples the signal on the selected channel and store the
    value in una static buffer that is returned. It is used by sincheck and
    mixmax analysis for siwave print or resistance calculation respectively.

*/
int16 * signal_sampling (uint8 channel){
    static int16 buffer[SAMPLING_SIZE];
    AC_source_Start();
    AMux_FastSelect(channel);
    for (uint32_t i = 0; i < SAMPLING_SIZE; i++){      
        buffer[i] = ADC_Read16();    
    }
    AC_source_Stop();
    return buffer;
}

/*
    Used in min max analysis in order to determine the minimum of the signal buffer
*/
int16 MIN (int16 *voltage_array){
    int16 min = MIN_CONST;
    for (uint32_t i = 0; i < SAMPLING_SIZE; i++){
        if (*(voltage_array +i) < min) min = *(voltage_array +i);
    }
    return min;
}

/*
    Used in min max analysis in order to determine the maximum of the signal buffer
*/
int16 MAX (int16 *voltage_array){
    int16 max = MAX_CONST;
    for (uint32_t i = 0; i < SAMPLING_SIZE; i++){
        if (*(voltage_array +i) > max) max = *(voltage_array +i);
    }
    return max;
}


/*
    Digital filter used to remove noise from the resistance analysis. It use a feedforward term in order
    to reduce settling time of the algorithm. If the holder is empy (first measurement) or if a sudden
    resistance step arise (ten time higher or ten time lower), the holder is filled with this new value 
    and a moving average FIR is then applied. 
*/
double filter (double resistance){
    static double holder[HOLDERSIZE];
    double filtered = 0.0;
    
    if (holder[HOLDERSIZE-1] == 0 ||holder[HOLDERSIZE-1]/resistance > 10 || holder[HOLDERSIZE-1]/resistance < 0.1){
        for (uint32_t i = 0; i < HOLDERSIZE; i++){
            holder[i] = resistance;
        }
    } else {
        for (uint32_t i = HOLDERSIZE; i-- >0; ){
            if (i != 0) holder[i] = holder[i-1];
        }
        holder[0] = resistance;
    }
    
    for (uint32_t i = 0; i < HOLDERSIZE; i++){
         filtered = filtered + holder[i]/HOLDERSIZE;
    }
   
    return filtered;
    
}

/*
    Routine necessary in order to print and update on oled screen
*/

void oled_print (double resistance){
    
       display_clear();    
       display_update();    
       gfx_setTextSize(2);
       gfx_setTextColor(WHITE);
       gfx_setCursor(5,25);
       sprintf(stg_conductivity_buffer, "%.2f ohm", resistance); 
       gfx_println(stg_conductivity_buffer);
       display_update();
}
    