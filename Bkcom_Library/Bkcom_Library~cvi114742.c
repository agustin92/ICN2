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
static int threadFunctionId_read_parameters;
static int threadFunctionId_update_interface;
static int gExit_read_parameters = 1;
static int CVICALLBACK read_parameters (void *functionData); // Measure thread
static int CVICALLBACK update_interface (void *functionData); //Update interface
//==============================================================================
// Global variables
DefineThreadSafeArrayVar(unsigned char,Read_Value,18,0)
//==============================================================================
// Global functions
/// HIFN The main entry-point function.
int main (int argc, char *argv[])
{
    int error = 0;
    unsigned char (*Read_Value_Ptr)[18];
	
	// Initialize read values thread safe
	
	
    /* initialize and load resources */
    nullChk (InitCVIRTE (0, argv, 0));
    errChk (panelHandle = LoadPanel (0, "Bkcom_Library.uir", PANEL));
    
    /* display the panel and run the user interface */
    errChk (DisplayPanel (panelHandle));
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
void Start_reading (void)
{
	unsigned char (*Read_Value_Ptr)[18];
	
	InitializeRead_Value();
	Read_Value_Ptr = GetPointerToRead_Value();
	FillBytes (*Read_Value_Ptr, 0, 32, 0x0);
	ReleasePointerToRead_Value();
	/*
	Read_Value_Ptr = GetPointerToRead_Value();
	for(int i=0; i < 18; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  (*Read_Value_Ptr)[i]);
			}
	ReleasePointerToRead_Value();
	*/
	
	gExit_read_parameters = 0; 
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE,read_parameters, NULL,&threadFunctionId_read_parameters);
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE,update_interface, NULL,&threadFunctionId_update_interface);
	return;
}
//==============================================================================     
void Stop_reading (void)
{
	gExit_read_parameters = 1;  
	CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE,threadFunctionId_read_parameters,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE,threadFunctionId_update_interface,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionId_read_parameters); 
	CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionId_update_interface); 
	UninitializeRead_Value();
	return;
}
//==============================================================================
int CVICALLBACK start_reading (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	int status = -1;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle , PANEL_TOGGLEBUTTON, &status);
			if (status ==1)
			{
				Start_reading();
			}
			else
			{
				Stop_reading();
			}
			break;
	}
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
unsigned char read_buffer[18]; //  
unsigned char Checksumm[1]; //  

double Pressure_01 = 0;
double Pressure_02 = 0;
double Temperature = 0;

// Thread safe variables
unsigned char (*Read_Value_Ptr)[18];

			
			//Open COM port
			error = OpenComConfig (PortCOM, "", 38400, 2, 8, 1, 256, 256);
			//printf("COM %d port open = \t%d \n",PortCOM, error);
			
			//Flush COM Port
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			//printf("COM %d port flush = \t%d \n",PortCOM, error);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (send_buffer, 0, 32, 0x0);
			FillBytes (read_buffer, 0, 18, 0x0);
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
			
			//printf("Checksum = %X\n", Checksumm[0]);
			
			send_buffer[4] = Checksumm[0]; //		
			
		
			
			//Request command from master to slave
			/*
			for(int i=0; i < 5; i++)
			{
				printf("send_buffer[%d] = \t%X\n",i,send_buffer[i]);
			}
			*/
			
			//Write command to COM port
			int write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			//printf("Written bytes = \t%d \n", write_bytes_counter);
			//printf("OK\n");
			
			//READ: the first READ is empty because the input channels are off		
			for(int i=0; i < 6; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			
			while (!gExit_read_parameters)
			{
				
			
			Delay (0.1);
			// Second round
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			FillBytes (read_buffer, 0, 18, 0x0);
			
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
			
			
			
			Read_Value_Ptr = GetPointerToRead_Value();
			for(int i=0; i < 18; i++)
			{
			(*Read_Value_Ptr)[i] = read_buffer[i];
			}
			ReleasePointerToRead_Value();
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
							
			//SetCtrlVal(panelHandle, PANEL_pressure_ccr_n,Pressure_01);
			//SetCtrlVal(panelHandle, PANEL_pressure_pkr_n,Pressure_02);
			//SetCtrlVal(panelHandle, PANEL_temperature_n,Temperature);
			}
			
			//Delay(1);
			
			CloseCom (PortCOM);	
			
			
				
			
	
	return 0;

}

//==============================================================================

int CVICALLBACK update_interface (void *functionData);
{
	unsigned char read_values[18];
	unsigned char (*Read_Value_Ptr)[18];
	double Pressure_01 = 0;
	double Pressure_02 = 0;
	double Temperature = 0;
	
	
	while(!gExit_read_parameters);
	
			Read_Value_Ptr = GetPointerToRead_Value();
			for(int i=0; i < 18; i++)
			{
			read_values[i] = (*Read_Value_Ptr)[i];
			}
			ReleasePointerToRead_Value();
	
			Pressure_01 = ( read_values[6] << 8 ) | read_values[5];   
			Pressure_02 = ( read_values[8] << 8 ) | read_values[7];   
			Temperature = ( read_values[14] << 8 ) | read_values[13];   
			
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
			
			//while (1)
			//{
				
			
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
			
			//PRINT50
			for(int i=0; i < 18; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  read_buffer[i]);
			}
			
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
			//}
			
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
			
			
			
			CloseCom (PortCOM);	
			break;
	}
	return 0;
}




//============================================================================== 


//============================================================================== 


//============================================================================== 


