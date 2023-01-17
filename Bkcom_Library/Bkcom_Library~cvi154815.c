//==============================================================================
//
// Title:       Bkcom_Library
// Purpose:     A short description of the application.
//
// Created on:  28/10/2022 at 16:07:12 by .
// Copyright:   . All Rights Reserved.
//
//==============================================================================
//==============================================================================
// Include files
#include <formatio.h>
#include <rs232.h>
#include <ansi_c.h>
#include <cvirte.h>     
#include <userint.h>
#include "Bkcom_Library.h"
#include "toolbox.h"

#include <stdio.h>

//==============================================================================
// Constants
//==============================================================================
// Types
//==============================================================================
// Static global variables
static int panelHandle;
//==============================================================================
// Static functions
static int threadFunction_id_read_parameters;
static int gExit_read_parameters = 1;
static int CVICALLBACK read_parameters (void *functionData); // Measure thread
//==============================================================================
// Global variables
//==============================================================================
// Global functions
/// HIFN The main entry-point function.
int main (int argc, char *argv[])
{
    int error = 0;
   
	
    /* initialize and load resources */
    nullChk (InitCVIRTE (0, argv, 0));
    errChk (panelHandle = LoadPanel (0, "Bkcom_Library.uir", PANEL));
    
    /* display the panel and run the user interface */
    errChk (DisplayPanel (panelHandle));
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, read_parameters, NULL, &threadFunction_id_read_parameters);
    errChk (RunUserInterface ());
Error:
    /* clean up */
    DiscardPanel (panelHandle);
    return 0;
}

//==============================================================================
// UI callback function prototypes
/// HIFN Exit when the user dismisses the panel.
int CVICALLBACK panelCB (int panel, int event, void *callbackData,
        int eventData1, int eventData2)
{
    if (event == EVENT_CLOSE)
        QuitUserInterface (0);
    return 0;
}
//============================================================================== 

int CVICALLBACK read_parameters (void *functionData)
{

//Port COM
static int PortCOM = 4;

//Error
int error = -5;

//Buffer send & read
unsigned char send_buffer[32]; //
unsigned char read_buffer[32]; //  
unsigned char Checksumm[1]; //  

double Pressure_01 = 0;
double Pressure_02 = 0;
double Temperature = 0;


			
			//Open COM port
			error = OpenComConfig (PortCOM, "", 38400, 2, 8, 1, 256, 256);
			printf("COM %d port open = \t%d \n",PortCOM, error);
			
			//Flush COM Port
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			printf("COM %d port flush = \t%d \n",PortCOM, error);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (send_buffer, 0, 32, 0x0);
			FillBytes (read_buffer, 0, 32, 0x0);
			FillBytes (Checksumm, 0, 1, 0x0);
			
			
			//Concatenate command bytes
			send_buffer[0] = 0x50; //"P"   	Start identifier
			send_buffer[1] = 0x00; //		Number of process data output words (0 for only ask the input image)
			send_buffer[2] = 0x56; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)
			
			
			// Sum the bytes and put it at the last
			for(int i=0; i < 4; i++)
			{
				Checksumm[0]= Checksumm[0] + send_buffer[i];
			}
			
			printf("Checksum = %X\n", Checksumm[0]);
			
			send_buffer[4] = Checksumm[0]; //		
			
		
			
			//Request command from master to slave
			for(int i=0; i < 5; i++)
			{
				printf("send_buffer[%d] = \t%X\n",i,send_buffer[i]);
			}
				
			
			//Write command to COM port
			int write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			printf("Written bytes = \t%d \n", write_bytes_counter);
			printf("OK\n");
			
			//READ: the first READ is empty because the input channels are off		
			for(int i=0; i < 6; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			
			while (1)
			{
				
			
			Delay (0.1);
			// Second round
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			FillBytes (read_buffer, 0, 32, 0x0);
			
			write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			//printf("Written bytes = \t%d \n", write_bytes_counter);
			//printf("OK\n");
			
			//READ			
			for(int i=0; i < 18; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			/*
			//PRINT50
			for(int i=0; i < 18; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  read_buffer[i]);
			}
			*/
			Pressure_01 = ( read_buffer[6] << 8 ) | read_buffer[5];   
			Pressure_02 = ( read_buffer[8] << 8 ) | read_buffer[7];   
			Temperature = ( read_buffer[14] << 8 ) | read_buffer[13];   
			
			Pressure_01 = Pressure_01*10/32767; //Volts
			Pressure_01 = Pressure_01*100; //Pressure in mTorr
			
			Pressure_02 = Pressure_02*10/32767; //Volts
			Pressure_02 = pow(10.0,1.667*Pressure_02-8.458); // Pressure in mTorr (See manual for parameters to convert in other units)
			
			Temperature = Temperature/10.0; // The output value from the module is almost converted to ºC
			
			//printf("Pressure 01 \t%lf\n",Pressure_01);
			//printf("Pressure 02 \t%lf\n",Pressure_02);
			//printf("Temperature \t%lf\n",Temperature);
							
			SetCtrlVal(panelHandle, PANEL_pressure_ccr_n,Pressure_01);
			SetCtrlVal(panelHandle, PANEL_pressure_pkr_n,Pressure_02);
			SetCtrlVal(panelHandle, PANEL_temperature_n,Temperature);
			}
			
			//Delay(1);
			
			CloseCom (PortCOM);	
			
			
				
			
	
	return 0;

}
//============================================================================== 

int CVICALLBACK READ_FUNTION (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{

//Port COM
static int PortCOM = 4;

//Error
int error = -5;

//Buffer send & read
unsigned char send_buffer[32]; //
unsigned char read_buffer[32]; //  
unsigned char Checksumm[1]; //  

double Pressure_01 = 0;
double Pressure_02 = 0;
double Temperature = 0;

//EVEN_COMMIT
switch (event)
	{
		case EVENT_COMMIT:
			
			//Open COM port
			error = OpenComConfig (PortCOM, "", 38400, 2, 8, 1, 256, 256);
			printf("COM %d port open = \t%d \n",PortCOM, error);
			
			//Flush COM Port
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			printf("COM %d port flush = \t%d \n",PortCOM, error);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (send_buffer, 0, 32, 0x0);
			FillBytes (read_buffer, 0, 32, 0x0);
			FillBytes (Checksumm, 0, 1, 0x0);
			
			
			//Concatenate command bytes
			send_buffer[0] = 0x50; //"P"   	Start identifier
			send_buffer[1] = 0x00; //		Number of process data output words (0 for only ask the input image)
			send_buffer[2] = 0x56; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)
			
			
			// Sum the bytes and put it at the last
			for(int i=0; i < 4; i++)
			{
				Checksumm[0]= Checksumm[0] + send_buffer[i];
			}
			
			printf("Checksum = %X\n", Checksumm[0]);
			
			send_buffer[4] = Checksumm[0]; //		
			
		
			
			//Request command from master to slave
			for(int i=0; i < 5; i++)
			{
				printf("send_buffer[%d] = \t%X\n",i,send_buffer[i]);
			}
				
			
			//Write command to COM port
			int write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			printf("Written bytes = \t%d \n", write_bytes_counter);
			printf("OK\n");
			
			//READ: the first READ is empty because the input channels are off		
			for(int i=0; i < 6; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			
			while (1)
			{
				
			
			Delay (0.1);
			// Second round
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			FillBytes (read_buffer, 0, 32, 0x0);
			
			write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			//printf("Written bytes = \t%d \n", write_bytes_counter);
			//printf("OK\n");
			
			//READ			
			for(int i=0; i < 18; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			/*
			//PRINT50
			for(int i=0; i < 18; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  read_buffer[i]);
			}
			*/
			Pressure_01 = ( read_buffer[6] << 8 ) | read_buffer[5];   
			Pressure_02 = ( read_buffer[8] << 8 ) | read_buffer[7];   
			Temperature = ( read_buffer[14] << 8 ) | read_buffer[13];   
			
			Pressure_01 = Pressure_01*10/32767; //Volts
			Pressure_01 = Pressure_01*100; //Pressure in mTorr
			
			Pressure_02 = Pressure_02*10/32767; //Volts
			Pressure_02 = pow(10.0,1.667*Pressure_02-8.458); // Pressure in mTorr (See manual for parameters to convert in other units)
			
			Temperature = Temperature/10.0; // The output value from the module is almost converted to ºC
			
			//printf("Pressure 01 \t%lf\n",Pressure_01);
			//printf("Pressure 02 \t%lf\n",Pressure_02);
			//printf("Temperature \t%lf\n",Temperature);
							
			SetCtrlVal(panelHandle, PANEL_pressure_ccr_n,Pressure_01);
			SetCtrlVal(panelHandle, PANEL_pressure_pkr_n,Pressure_02);
			SetCtrlVal(panelHandle, PANEL_temperature_n,Temperature);
			}
			
			//Delay(1);
			
			CloseCom (PortCOM);	
			
			
				
			break;
	}
	return 0;
}




//============================================================================== 
int CVICALLBACK WRITE_FUNTION (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
//Port COM
static int PortCOM = 4;

//Error
int error = -5;

//Buffer send & read
unsigned char send_buffer[32]; //
unsigned char read_buffer[32]; //  
unsigned char Checksumm[1]; //  


switch (event)
	{
		case EVENT_COMMIT:
			//Open COM port
			error = OpenComConfig (PortCOM, "", 38400, 2, 8, 1, 256, 256);
			printf("COM %d port open = \t%d \n",PortCOM, error);
			
			//Flush COM Port
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			printf("COM %d port flush = \t%d \n",PortCOM, error);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (send_buffer, 0, 32, 0x0);
			FillBytes (read_buffer, 0, 32, 0x0);
			FillBytes (Checksumm, 0, 1, 0x0);
			
			/*	
			//Concatenate command bytes
			send_buffer[0] = 0x50; //"P"   	Start identifier
			send_buffer[1] = 0x04; //		Number of process data output words
			send_buffer[2] = 0x12; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)
			send_buffer[4] = 0x00; //		Byte[0]		
			send_buffer[5] = 0x00; //		Byte[1]
			send_buffer[6] = 0x00; //		Byte[2]
			send_buffer[7] = 0x00; //		Byte[3]	
			send_buffer[8] = 0x00; //		Byte[4]		
			send_buffer[9] = 0x00; //		Byte[5]
			send_buffer[10] = 0x00; //		Byte[6]
			send_buffer[11] = 0x00; //		Byte[7]
			//send_buffer[12] = 0x00:	//		Byte[8] Dummy
			send_buffer[12] = 0x0F; //		Checksum
			*/
			//Concatenate command bytes
			send_buffer[0] = 0x50; //"P"   	Start identifier
			send_buffer[1] = 0x03; //		Number of process data output words
			send_buffer[2] = 0x56; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)
					
			send_buffer[4] = 0x00; //		Data byte 0
			send_buffer[5] = 0x00; //		Data byte 1
			send_buffer[6] = 0x00; //		Data byte 2
			send_buffer[7] = 0x00; //		Data byte 3
			send_buffer[8] = 0x01; //		Data byte 4
			send_buffer[9] = 0x00; //		Data byte 5
		/*	send_buffer[10] = 0x00; //		Data byte 6
			send_buffer[11] = 0x00; //		Data byte 7
			send_buffer[12] = 0x00; //		Data byte 8
			send_buffer[13] = 0x00; //		Data byte 9
			send_buffer[14] = 0x00; //		Data byte 10
			send_buffer[15] = 0x00; //		Data byte 11
		*/
			
			
			for(int i=0; i < 10; i++)
			{
				Checksumm[0]= Checksumm[0] + send_buffer[i];
			}
			
			printf("Checksum = %X\n", Checksumm[0]);
			
			send_buffer[10] = Checksumm[0]; //		
			
		
			
			//Request command from master to slave
			for(int i=0; i < 11; i++)
			{
				printf("send_buffer[%d] = \t%X\n",i,send_buffer[i]);
			}
				
			
			//Write command to COM port
			int write_bytes_counter = 0;
			for(int i=0; i < 11; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			printf("Written bytes = \t%d \n", write_bytes_counter);
			printf("OK\n");
			
			//Delay (0.1);
			//READ			
			for(int i=0; i < 6; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			
			//PRINT50
			/*
			for(int i=0; i < 6; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  read_buffer[i]);
			}
			
			Delay (0.05);
			// Second round
			
			write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			printf("Written bytes = \t%d \n", write_bytes_counter);
			printf("OK\n");
			
			Delay (1);
			//READ			
			for(int i=0; i < 18; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			
			//PRINT50
			for(int i=0; i < 18; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  read_buffer[i]);
			}
			*/
			
			/*
			
			for( int j=0; j<20 ;j++)
			{
			//Write command to COM port
			int write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			printf("Written bytes = \t%d \n", write_bytes_counter);
			printf("OK\n");
			
			//Delay (1);
			//READ			
			for(int i=0; i < 6; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			
			//PRINT50
			for(int i=0; i < 6; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  read_buffer[i]);
			}
			}
			
			*/
			
			//send_buffer[5] = 0x50; //"P"
			//send_buffer[6] = 0x50; //"P"
			//send_buffer[7] = 0x50; //"P"
			
			/*
			
			for (int i = 0; i < 8; i++) 
				{
	    		// Mask each bit in the byte and store it
	    		//bits[i] = (byte & (mask << i)) != 0;
				bits[i] = (send_buffer[1] & (mask << i)) != 0;
				}
	 		
			// For debug purposes, lets print the received data
			
			for (int i = 0; i < 8; i++) 
				{
				printf("Bit %d: %d\n",i, bits[i]);
				}
			
			*/
			
			
			//Delay(1);
			
			CloseCom (PortCOM);	
			break;
	}
	return 0;
}




//============================================================================== 







//============================================================================== 


//============================================================================== 
