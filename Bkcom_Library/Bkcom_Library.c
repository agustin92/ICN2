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
static int threadFunctionId_PID_temperature;
static int gExit_read_parameters = 1;
static int CVICALLBACK read_parameters (void *functionData); // Measure thread
static int CVICALLBACK update_interface (void *functionData); //Update interface
static int CVICALLBACK PID_temperature (void *functionData);



//==============================================================================
// Global variables
DefineThreadSafeArrayVar(unsigned char,Read_Value,18,0); // Thread safe variable => Read values of the Beckhoff coupler
DefineThreadSafeArrayVar(unsigned char,Write_Value,11,0); // Thread safe variable => Write values of the Beckhoff coupler
DefineThreadSafeScalarVar(int,Write_Flag, 0); // Thread safe variable => Write flag to indicate the program that it is necesary to write new values in the Beckhoff coupler
DefineThreadSafeScalarVar(int,TP_Flag, 0); // Thread safe variable => Write flag to indicate the program that it is necesary to write new values in the Turbo Pump controller
DefineThreadSafeScalarVar(int,TP_Velocity, 0); // Thread safe variable => Turbo pump velocity
DefineThreadSafeScalarVar(int,Temperature_Flag, 0); //Flag to let know the Temperature PID control that changes have been made on the rate and/or set point values


//==============================================================================
// Global functions
/// HIFN The main entry-point function.
int main (int argc, char *argv[])
{
    int error = 0;
	int error_2 = -1;
    unsigned char (*Read_Value_Ptr)[18];
	unsigned char (*Write_Value_Ptr)[11];
	int *Write_Flag_Ptr;
	int *TP_Flag_Ptr;
	int *TP_Velocity_Ptr;
	int *Temperature_Flag_Ptr;
	
	// Initialize read values thread safe
	InitializeRead_Value();
	InitializeWrite_Value();
	InitializeWrite_Flag();
	InitializeTP_Flag();
	InitializeTP_Velocity();
	InitializeTemperature_Flag();
	
	Read_Value_Ptr = GetPointerToRead_Value();
	FillBytes (*Read_Value_Ptr, 0, 18, 0x0);
	ReleasePointerToRead_Value();
	
	Write_Value_Ptr = GetPointerToWrite_Value();
	FillBytes (*Write_Value_Ptr, 0, 11, 0x0);
	ReleasePointerToWrite_Value();
	
	Write_Flag_Ptr = GetPointerToWrite_Flag();
	(*Write_Flag_Ptr) = 0;
	ReleasePointerToWrite_Flag();
	
	TP_Flag_Ptr = GetPointerToTP_Flag();
	(*TP_Flag_Ptr) = 0;
	ReleasePointerToTP_Flag();
	
	TP_Velocity_Ptr = GetPointerToTP_Velocity();
	(*TP_Velocity_Ptr) = 0;
	ReleasePointerToTP_Velocity();
	
	Temperature_Flag_Ptr = GetPointerToTemperature_Flag();
	(*Temperature_Flag_Ptr) = 0;
	ReleasePointerToTemperature_Flag();
	
    /* initialize and load resources */
    nullChk (InitCVIRTE (0, argv, 0));
    errChk (panelHandle = LoadPanel (0, "Bkcom_Library.uir", PANEL));
    
    /* display the panel and run the user interface */
    errChk (DisplayPanel (panelHandle));
    errChk (RunUserInterface ());
	//function_start_K1();
	
	
Error:
    /* clean up */
	//function_stop_K1();
	UninitializeRead_Value();
	UninitializeWrite_Value();
	UninitializeWrite_Flag();
	UninitializeTP_Flag();
	UninitializeTP_Velocity();
	UninitializeTemperature_Flag();
	
    DiscardPanel (panelHandle);
    return 0;
}

void conv_short_to_word (short v, unsigned char * buf)
{
	unsigned char	*p;

	p = (unsigned char *)&v;

	* (buf + 0) = * (p + 1);	 
	* (buf + 1) = * (p + 0);	

	return;
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
void Start_reading_thread (void)
{
	
	gExit_read_parameters = 0; 
	
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE,read_parameters, NULL,&threadFunctionId_read_parameters);
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE,update_interface, NULL,&threadFunctionId_update_interface);
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE,PID_temperature, NULL,&threadFunctionId_PID_temperature);

	return;
}
//==============================================================================     
void Stop_reading_thread (void)
{
	gExit_read_parameters = 1;  
	CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE,threadFunctionId_read_parameters,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE,threadFunctionId_update_interface,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtWaitForThreadPoolFunctionCompletion (DEFAULT_THREAD_POOL_HANDLE,threadFunctionId_PID_temperature,OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	
	CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionId_read_parameters); 
	CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionId_update_interface); 
	CmtReleaseThreadPoolFunctionID (DEFAULT_THREAD_POOL_HANDLE, threadFunctionId_PID_temperature); 
	
	
	//UninitializeRead_Value();
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
				Start_reading_thread();
			}
			else
			{
				Stop_reading_thread();
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
static int PortCOM_TP = 6;

//Error
int error = -5;
int error_TP = -1;

//Buffer send & read Beckhoff
unsigned char read_buffer[18]; // Beckhoff
unsigned char send_buffer[11]; // Beckhoff
unsigned char Checksumm[1]; //   Beckhoff
int write_flag;// Flag to write on the Beckhoff
unsigned char K1[2];
unsigned char TP[2];
int write_bytes_counter;

//Buffer send & read Turbo Pump
char* pdata_TP; // TP
char* rdata_TP; // TP
char p_datastr_TP[100]; // TP
char r_datastr_TP[100]; // TP
int tp_flag; // Flag to know if the TP is power up
int Velocity_TP = 0; // Velocity of the TP
int tp_status = 0; // 0 = Off; 1 = ON 


// Thread safe variables
unsigned char (*Read_Value_Ptr)[18]; // Beckhoff
unsigned char (*Write_Value_Ptr)[11]; // Beckhoff
int (*Write_Flag_Ptr); // Beckhoff writing flag
int (*TP_Flag_Ptr); // Turbo pump flag to know if the turbo pump is power up
int (*Velocity_TP_Ptr); // Velocity of the Turbo pump

			
			//Open COM port
			error = OpenComConfig (PortCOM, "", 38400, 2, 8, 1, 256, 256);
			
			//Flush COM Port
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			//printf("COM %d port flush = \t%d \n",PortCOM, error);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (read_buffer, 0, 18, 0x0);
			FillBytes (send_buffer, 0, 11, 0x0);
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
			send_buffer[4] = Checksumm[0]; //		
			
			//Write command to COM port
			write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			//READ: the first READ is empty because the input channels are off		
			for(int i=0; i < 4; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}

			// Start K1 and TP relays
			
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (read_buffer, 0, 18, 0x0);
			FillBytes (send_buffer, 0, 11, 0x0);
			FillBytes (Checksumm, 0, 1, 0x0);
			
			//Concatenate command bytes
			conv_short_to_word(1.0,K1);
			conv_short_to_word(32.0,TP);
			
			send_buffer[0] = 0x50; //"P"   	Start identifier
			send_buffer[1] = 0x03; //		Number of process data output words
			send_buffer[2] = 0x56; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)		
			send_buffer[4] = 0x00; //		Data byte 0
			send_buffer[5] = 0x00; //		Data byte 1
			send_buffer[6] = 0x00; //		Data byte 2
			send_buffer[7] = 0x00; //		Data byte 3
			send_buffer[8] = K1[1]+TP[1]; //		Data byte 4
			send_buffer[9] = 0x00; //		Data byte 5
			
			Write_Value_Ptr = GetPointerToWrite_Value();
			(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8] + K1[1]+TP[1];
			ReleasePointerToWrite_Value();
			
			for(int i=0; i < 10; i++)
			{
				Checksumm[0]= Checksumm[0] + send_buffer[i];
			}
			
			send_buffer[10] = Checksumm[0]; //		
			
			//Write command to COM port
			write_bytes_counter = 0;
			for(int i=0; i < 11; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			
			SetCtrlVal(panelHandle, PANEL_LED_K1,1);
			SetCtrlVal(panelHandle, PANEL_LED_TP,1);
			
			Delay(3.0);
			
			
			
			//
			
			
			
			while (!gExit_read_parameters)
			{
			Delay (0.1);
			
			Write_Flag_Ptr = GetPointerToWrite_Flag();
			write_flag = (*Write_Flag_Ptr) ;
			ReleasePointerToWrite_Flag();
			
			// Beckhoff Writing/Reading
			if (!write_flag)
			{
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			FillBytes (read_buffer, 0, 18, 0x0);
			FillBytes (send_buffer, 0, 11, 0x0);
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
			
			send_buffer[4] = Checksumm[0]; //
			
			write_bytes_counter = 0;
			for(int i=0; i < 5; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
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
			
			}
			else
			{
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (read_buffer, 0, 18, 0x0);
			FillBytes (send_buffer, 0, 11, 0x0);
			FillBytes (Checksumm, 0, 1, 0x0);
			
			Write_Value_Ptr = GetPointerToWrite_Value();
			
			
			//Concatenate command bytes
			
			send_buffer[0] = 0x50; //"P"   	Start identifier
			send_buffer[1] = 0x03; //		Number of process data output words
			send_buffer[2] = 0x56; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)		
			send_buffer[4] = (*Write_Value_Ptr)[4]; //		Data byte 0
			send_buffer[5] = (*Write_Value_Ptr)[5]; //		Data byte 1
			send_buffer[6] = (*Write_Value_Ptr)[6]; //		Data byte 2
			send_buffer[7] = (*Write_Value_Ptr)[7]; //		Data byte 3
			send_buffer[8] = (*Write_Value_Ptr)[8]; //		Data byte 4
			send_buffer[9] = (*Write_Value_Ptr)[9]; //		Data byte 5
			
			ReleasePointerToWrite_Value();
			
			for(int i=0; i < 10; i++)
			{
				Checksumm[0]= Checksumm[0] + send_buffer[i];
			}
			
			send_buffer[10] = Checksumm[0]; //		
				
			
			//Write command to COM port
			int write_bytes_counter = 0;
			for(int i=0; i < 11; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			
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
			
			Write_Flag_Ptr = GetPointerToWrite_Flag();
			(*Write_Flag_Ptr) = 0;
			ReleasePointerToWrite_Flag();
				
			}
			
			// TP Writing/Reading
			
				
			pdata_TP = p_datastr_TP; 
			rdata_TP = r_datastr_TP;     
				
			error_TP = OpenComConfig (PortCOM_TP, "", 9600, 0, 8, 1, 512, 512);//8
				
			error_TP = FlushInQ (PortCOM_TP);
			error_TP = FlushOutQ (PortCOM_TP);		

			FillBytes (pdata_TP, 0, 100, 0x0);
			FillBytes (rdata_TP, 0, 100, 0x0);
				
			error_TP = sprintf(pdata_TP,"0010030902=?107\r");  //Ask velocity
			error_TP = ComWrt (PortCOM_TP, pdata_TP,  strlen(pdata_TP));
				
			Delay (0.01);
			error_TP = ComRdTerm (PortCOM_TP, rdata_TP, 100, 13);
				
			sscanf(rdata_TP, "0011030906%06d", &Velocity_TP);
				
			TP_Flag_Ptr = GetPointerToTP_Flag();
			tp_flag = (*TP_Flag_Ptr);
			ReleasePointerToTP_Flag();
			
			if (tp_flag) {
				
				if (tp_status) {
					Delay(0.01);
					error_TP = FlushInQ (PortCOM_TP);
					error_TP = FlushOutQ (PortCOM_TP);		

					FillBytes (pdata_TP, 0, 100, 0x0);
					FillBytes (rdata_TP, 0, 100, 0x0);
				
					error_TP = sprintf(pdata_TP,"0011001006000000009\r");  //Ask velocity
					error_TP = ComWrt (PortCOM_TP, pdata_TP,  strlen(pdata_TP));
					tp_status = 0; // Flag to indicate that the TP is off
					TP_Flag_Ptr = GetPointerToTP_Flag();
					(*TP_Flag_Ptr) = 0;
					ReleasePointerToTP_Flag();
					Delay(0.01);
				}
				else 
				{
					Delay(0.01);
					error_TP = FlushInQ (PortCOM_TP);
					error_TP = FlushOutQ (PortCOM_TP);		

					FillBytes (pdata_TP, 0, 100, 0x0);
					FillBytes (rdata_TP, 0, 100, 0x0);
				
					error_TP = sprintf(pdata_TP,"0011001006111111015\r");  //Ask velocity
					error_TP = ComWrt (PortCOM_TP, pdata_TP,  strlen(pdata_TP));
					tp_status = 1; // Flag to indicate that the TP is on
					TP_Flag_Ptr = GetPointerToTP_Flag();
					(*TP_Flag_Ptr) = 0;
					ReleasePointerToTP_Flag();
					Delay(0.01);
				}
				
			}
			
			CloseCom (PortCOM_TP);
				
			Velocity_TP_Ptr = GetPointerToTP_Velocity();
			(*Velocity_TP_Ptr) = Velocity_TP;
			ReleasePointerToTP_Velocity();					
			
			
			}
			
			
			
			// Stop K1 and TP relays
			
			error = FlushInQ (PortCOM);
			error = FlushOutQ (PortCOM);
			
			//Fill buffer 0x0 (Clear) 
			FillBytes (read_buffer, 0, 18, 0x0);
			FillBytes (send_buffer, 0, 11, 0x0);
			FillBytes (Checksumm, 0, 1, 0x0);
			
			
			
			
			//Concatenate command bytes
			conv_short_to_word(1.0,K1);
			conv_short_to_word(32.0,TP);
			
			send_buffer[0] = 0x50; //"P"   	Start identifier
			send_buffer[1] = 0x03; //		Number of process data output words
			send_buffer[2] = 0x56; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)		
			send_buffer[4] = 0x00; //		Data byte 0
			send_buffer[5] = 0x00; //		Data byte 1
			send_buffer[6] = 0x00; //		Data byte 2
			send_buffer[7] = 0x00; //		Data byte 3
			send_buffer[8] = 0x00; //		Data byte 4
			send_buffer[9] = 0x00; //		Data byte 5
			
			
			Write_Value_Ptr = GetPointerToWrite_Value();
			(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8] - K1[1]-TP[1];
			
			ReleasePointerToWrite_Value();
			
			for(int i=0; i < 10; i++)
			{
				Checksumm[0]= Checksumm[0] + send_buffer[i];
			}
			
			//printf("Checksum = %X\n", Checksumm[0]);
			
			send_buffer[10] = Checksumm[0]; //		
			
				
			
			//Write command to COM port
			write_bytes_counter = 0;
			for(int i=0; i < 11; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			
			SetCtrlVal(panelHandle, PANEL_LED_K1,0);
			SetCtrlVal(panelHandle, PANEL_LED_TP,0);
			
			
			TP_Flag_Ptr = GetPointerToTP_Flag();
			(*TP_Flag_Ptr) = 0;
			ReleasePointerToTP_Flag();
			
			
			
			//
			
			
			
			
			CloseCom (PortCOM);	
			
			
				
			
	
	return 0;

}

//==============================================================================

int CVICALLBACK update_interface (void *functionData)
{
	// Beckhoff values
	unsigned char read_values[18];
	unsigned char (*Read_Value_Ptr)[18]; // Thread safe pointer to Beckhoff values
	double Pressure_01 = 0;
	double Pressure_02 = 0;
	double Gas_FLux = 0;
	double Temperature = 0;
	double digital;
	// Turbo pump
	int *Velocity_TP_Ptr; //Thread safe pointer of the velocity of turbo pump
	int Velocity_TP;
	
	read_values[15]=0x00;
	while(!gExit_read_parameters)
	{
			Delay (0.1);
			// Beckhoff
			Read_Value_Ptr = GetPointerToRead_Value();
			for(int i=0; i < 18; i++)
			{
			read_values[i] = (*Read_Value_Ptr)[i];
			}
			ReleasePointerToRead_Value();
			
			Pressure_01 = ( read_values[6] << 8 ) | read_values[5];   
			Pressure_02 = ( read_values[8] << 8 ) | read_values[7];  
			Gas_FLux = ( read_values[10] << 8 ) | read_values[9]; 
			Temperature = ( read_values[14] << 8 ) | read_values[13];   
			digital = ( read_values[16] << 8 ) | read_values[15];
			
			Pressure_01 = Pressure_01*10/32767; //Volts
			Pressure_01 = Pressure_01*100; //Pressure in mTorr
			
			Gas_FLux = Gas_FLux*10/32767; // Volts
			
			Pressure_02 = Pressure_02*10/32767; //Volts
			Pressure_02 = pow(10.0,1.667*Pressure_02-8.458); // Pressure in mTorr (See manual for parameters to convert in other units)
			
			Temperature = Temperature/10.0; // The output value from the module is almost converted to ºC
							
			SetCtrlVal(panelHandle, PANEL_pressure_ccr_n,Pressure_01);
			SetCtrlVal(panelHandle, PANEL_pressure_pkr_n,Pressure_02);
			SetCtrlVal(panelHandle, PANEL_flux_n,Gas_FLux);
			SetCtrlVal(panelHandle, PANEL_temperature_n,Temperature);
			
			// Turbo pump
			
			Velocity_TP_Ptr = GetPointerToTP_Velocity();
			Velocity_TP = (*Velocity_TP_Ptr);
			ReleasePointerToTP_Velocity();
			
			Velocity_TP = Velocity_TP*60; // Value in RPM
			
			SetCtrlVal(panelHandle, PANEL_tp_n,Velocity_TP);
			
			// Digital inputs
			// GV
			if (digital == 7.0)
			{
				SetCtrlVal(panelHandle, PANEL_TOGGLEBUTTON_GV,1);
			}
			if (digital == 11.0)
			{
				SetCtrlVal(panelHandle, PANEL_TOGGLEBUTTON_GV,0);
			}
			
			
	}	
	return 0;
}

//===========================================================================================
// ByPass valve control
int CVICALLBACK ByPass (int panel, int control, int event,
						void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;
	
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_ByPass, &status);
			if (status ==1)
			{
				
				conv_short_to_word(8.0,value);
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[9] = (*Write_Value_Ptr)[9]+ value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				
				
			}
			else
			{
				
				conv_short_to_word(8.0,value);
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[9] = (*Write_Value_Ptr)[9] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
			}
			
			
			break;
	}
	return 0;
}
//===========================================================================================
// GV valve control
int CVICALLBACK GV (int panel, int control, int event,
					void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;
	
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_GV, &status);
			if (status ==1)
			{
				
				conv_short_to_word(64.0,value); // Turn on the 6th Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8]+value[1];
				ReleasePointerToWrite_Value();
				//printf("%X",value[1]);
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				
				
			}
			else
			{
				
				conv_short_to_word(64.0,value); // Turn off the 6th Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
			}
			
			
			break;
	}
	return 0;
}
//============================================================================================
int CVICALLBACK AV (int panel, int control, int event,
					void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;
	
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_GV, &status);
			if (status ==1)
			{
				
				conv_short_to_word(128.0,value); // Turn on the 7th Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8]+value[1];
				ReleasePointerToWrite_Value();
				//printf("%X",value[1]);
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				
				
			}
			else
			{
				
				conv_short_to_word(128.0,value); // Turn off the 7th Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
			}
			
			
			break;
	}
	return 0;
}


//============================================================================================

int CVICALLBACK Gas_valve (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;
	
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_Gas, &status);
			if (status ==1)
			{
				
				conv_short_to_word(1.0,value); // Turn on the 7th Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[9] = (*Write_Value_Ptr)[9]+value[1];
				ReleasePointerToWrite_Value();
				//printf("%X",value[1]);
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				
				
			}
			else
			{
				
				conv_short_to_word(1.0,value); // Turn off the 7th Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[9] = (*Write_Value_Ptr)[9] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
			}
			
			
			break;
	}
	return 0;
}
//============================================================================================
int CVICALLBACK Vent (int panel, int control, int event,
					  void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;
	
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_Vent, &status);
			if (status ==1)
			{
				
				conv_short_to_word(2.0,value); // Turn on the 1st Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8]+value[1];
				ReleasePointerToWrite_Value();
			
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				
				
			}
			else
			{
				
				conv_short_to_word(2.0,value); // Turn off the 1st Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
			}
			
			
			break;
	}
	return 0;
}

//============================================================================================
int CVICALLBACK TP_on_off (int panel, int control, int event,
						   void *callbackData, int eventData1, int eventData2)
{
	int status = -1;
	
	
	// Thread safe variables
	int (*TP_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
				
			TP_Flag_Ptr = GetPointerToTP_Flag();
			(*TP_Flag_Ptr) = 1;
			ReleasePointerToTP_Flag();
			
			
			break;
	}
	return 0;
}

//============================================================================================

int CVICALLBACK heater_on_off (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;
	
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_Heater, &status);
			if (status ==1)
			{
				
				conv_short_to_word(2.0,value);
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[9] = (*Write_Value_Ptr)[9]+ value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				
				
			}
			else
			{
				
				conv_short_to_word(2.0,value);
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[9] = (*Write_Value_Ptr)[9] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
			}
			
			
			break;
	}
	return 0;
}
//============================================================================================
/*
int CVICALLBACK heater_output (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	double heater_output;
	int output;
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
			
			GetCtrlVal(panelHandle , PANEL_NUMERIC_4, &heater_output);	
			heater_output = heater_output*32767.0/10.0;
			output = (int)heater_output;
			conv_short_to_word(output,value);// Turn on the 1st Bite (starting from 0)
			Write_Value_Ptr = GetPointerToWrite_Value();
			(*Write_Value_Ptr)[6] = value[1];
			(*Write_Value_Ptr)[7] = value[0];
			ReleasePointerToWrite_Value();
			
			Write_Flag_Ptr = GetPointerToWrite_Flag();
			(*Write_Flag_Ptr) = 1;
			ReleasePointerToWrite_Flag();
			
			
			
			break;
	}
	return 0;
}
*/
//============================================================================================

int CVICALLBACK PID_temperature (void *functionData)
{
	// Local variables
	double temperature_real;
	double temperature_calc;
	double initial_temperature;
	double rate;
	double temperature_set_point; 
	double temperature_set_point_ini;
	double initial_time;
	double actual_time;
	double time_aux;
	double time_cicle;
	double time_old;
	double error_temp;
	double error_old;
	double output;
	
	int heater_output;
	
	unsigned char value[2];
	
	// PID parameters
	double kp;
	double ki;
	double kd;
	// PID correction
	double p;
	double i;
	double d;
	
	
	int temperature_flag;
	int ramp_flag; // To indicate if the ramp is going downward = 0 or upward = 1 
	
	
	// Thread safe variables
	int *Temperature_Flag_Ptr;
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	
	
	Delay(3.5);
	ramp_flag = 0;
	initial_time = Timer();
	time_aux = initial_time;
	time_cicle = 0.0;
	time_old = 0.0;
	//GetCtrlVal(panelHandle , PANEL_Rate, &rate);
	//GetCtrlVal(panelHandle , PANEL_Temperature_Set_Point, &temperature_set_point);
	GetCtrlVal(panelHandle , PANEL_temperature_n, &temperature_real);
	rate = 0;
	temperature_set_point = temperature_real;
	SetCtrlVal(panelHandle, PANEL_Temperature_Set_Point,temperature_set_point);
	temperature_set_point_ini = temperature_set_point;
	
	initial_temperature = temperature_real;
	temperature_calc = temperature_real;
	
	// PID definition
	error_temp = 0.0;
	error_old = 0.0;
	p = i = d = 0.0;
	output = 0.0;
	kp = 0.029;
	ki = 0.00098;
	kd = 0.1;
	
	SetCtrlVal(panelHandle, PANEL_kp,kp);
	SetCtrlVal(panelHandle, PANEL_ki,ki);
	SetCtrlVal(panelHandle, PANEL_kd,kd);
	
	GetCtrlVal(panelHandle , PANEL_kp, &kp);
	GetCtrlVal(panelHandle , PANEL_ki, &ki);
	GetCtrlVal(panelHandle , PANEL_kd, &kd);
	
	while (!gExit_read_parameters)
	{
		Temperature_Flag_Ptr = GetPointerToTemperature_Flag();
		temperature_flag = (*Temperature_Flag_Ptr);
		ReleasePointerToTemperature_Flag();
		Delay(0.1);
		if (temperature_flag)
		{
			GetCtrlVal(panelHandle , PANEL_Rate, &rate);
			GetCtrlVal(panelHandle , PANEL_Temperature_Set_Point, &temperature_set_point);
			GetCtrlVal(panelHandle , PANEL_temperature_n, &initial_temperature); 
			temperature_calc = initial_temperature;
			Temperature_Flag_Ptr = GetPointerToTemperature_Flag();
			(*Temperature_Flag_Ptr) = 0;
			ReleasePointerToTemperature_Flag();
			
			initial_time = Timer();
			time_cicle = initial_time;
			
			
			if (initial_temperature < temperature_set_point)
			{ 
				ramp_flag = 1;
			}
			else
			{
				ramp_flag = 0;
			}
		}
		
		if (ramp_flag)
		{
			if (temperature_calc < temperature_set_point) 
			{
				time_aux = Timer();
				temperature_calc = rate/60.0*(time_aux-initial_time)+initial_temperature;
			}
			else
			{
				temperature_calc = temperature_set_point;
			}
		}
		
		else
		{
			if (temperature_calc > temperature_set_point)
			{
				time_aux = Timer();
				temperature_calc = (-1.0)*rate/60.0*(time_aux-initial_time)+initial_temperature;
			}
			else
			{
				temperature_calc = temperature_set_point;
			}
		}
		GetCtrlVal(panelHandle , PANEL_temperature_n, &temperature_real);
		time_cicle = Timer() - time_old;
		error_temp = temperature_calc-temperature_real;
		SetCtrlVal(panelHandle, PANEL_error_temp,error_temp);
		SetCtrlVal(panelHandle, PANEL_temperature_calc,temperature_calc);
		
		
		GetCtrlVal(panelHandle , PANEL_kp, &kp);
		GetCtrlVal(panelHandle , PANEL_ki, &ki);
		GetCtrlVal(panelHandle , PANEL_kd, &kd);
		if (temperature_set_point != temperature_set_point_ini && rate !=0.0)
		{
			p = kp*error_temp;
			i = i + ki*error_temp*time_cicle;
			if (i>0.1)
			{
				i = 0.1;
			}
			d = kd*(error_temp-error_old)/(time_cicle);
			
			output = output + p + i + d;
			error_old = error_temp;
			
			if (temperature_real<40.0)
			{
				if (output >5.0)
				{
					output = 5.0;
				}
			}
			else if (temperature_real<900.0)
			{
				if (output > (2.262E-6*pow(temperature_real,2.0)+1.60E-3*temperature_real+3.8))
				{
					output = 2.262E-6*pow(temperature_real,2.0)+1.60E-3*temperature_real+3.8;
				}
			}
			
			if (output<0.0)
			{
				output = 0.0;
			}
			
			SetCtrlVal(panelHandle , PANEL_NUMERIC_4, output);
			
			heater_output = (int) (output*32767.0/10.0);
			SetCtrlVal(panelHandle , PANEL_NUMERIC, heater_output);
			conv_short_to_word(heater_output,value);
			Write_Value_Ptr = GetPointerToWrite_Value();
			(*Write_Value_Ptr)[6] = value[1];
			(*Write_Value_Ptr)[7] = value[0];
			ReleasePointerToWrite_Value();
				
			Write_Flag_Ptr = GetPointerToWrite_Flag();
			(*Write_Flag_Ptr) = 1;
			ReleasePointerToWrite_Flag();
			
			SetCtrlVal(panelHandle, PANEL_p,p);
			SetCtrlVal(panelHandle, PANEL_i,i);
			SetCtrlVal(panelHandle, PANEL_d,d);
		}
		
		time_old = Timer();
	}
	return 0;
}

//============================================================================================

int CVICALLBACK change_rate (int panel, int control, int event,
							 void *callbackData, int eventData1, int eventData2)
{
	int *Temperature_Flag_Ptr;
	switch (event)
	{
		case EVENT_COMMIT:
			Temperature_Flag_Ptr = GetPointerToTemperature_Flag();
			(*Temperature_Flag_Ptr) = 1;
			ReleasePointerToTemperature_Flag();
			break;
	}
	return 0;
}

//============================================================================================

int CVICALLBACK change_temperature_set_point (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int *Temperature_Flag_Ptr;
	switch (event)
	{
		case EVENT_COMMIT:
			Temperature_Flag_Ptr = GetPointerToTemperature_Flag();
			(*Temperature_Flag_Ptr) = 1;
			ReleasePointerToTemperature_Flag();
			break;
	}
	return 0;
}
//============================================================================================

int CVICALLBACK Ar_flux (int panel, int control, int event,
						 void *callbackData, int eventData1, int eventData2)
{
	// Local variables
	double ar_flux;
	int ar_output;
	unsigned char value[2];
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panelHandle , PANEL_Ar_output, &ar_flux);
			ar_output = (int) (ar_flux*32767.0/10.0);
			conv_short_to_word(ar_output,value);
			Write_Value_Ptr = GetPointerToWrite_Value();
			(*Write_Value_Ptr)[4] = value[1];
			(*Write_Value_Ptr)[5] = value[0];
			ReleasePointerToWrite_Value();
			
			Write_Flag_Ptr = GetPointerToWrite_Flag();
			(*Write_Flag_Ptr) = 1;
			ReleasePointerToWrite_Flag();
			break;
	}
	return 0;
}

//============================================================================================
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


//==============================================================================================================================
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


/*
int CVICALLBACK TP (int panel, int control, int event,
					void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;

	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	int (*TP_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_TP, &status);
			if (status ==1)
			{
				
				conv_short_to_word(32.0,value); // Turn on the 1st Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8]+value[1];
				ReleasePointerToWrite_Value();
			
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				TP_Flag_Ptr = GetPointerToTP_Flag();
				(*TP_Flag_Ptr) = 1;
				ReleasePointerToTP_Flag();
				
			}
			else
			{
				
				conv_short_to_word(32.0,value); // Turn off the 1st Bite (starting from 0)
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				TP_Flag_Ptr = GetPointerToTP_Flag();
				(*TP_Flag_Ptr) = 0;
				ReleasePointerToTP_Flag();
			}
			
			
			break;
	}
	return 0;
}
*/


/*
int CVICALLBACK start_K1 (int panel, int control, int event,
						  void *callbackData, int eventData1, int eventData2)
{
	unsigned char value[2];
	int status = -1;
	
	
	// Thread safe variables
	unsigned char write_values[18];
	unsigned char (*Write_Value_Ptr)[18];
	int (*Write_Flag_Ptr);
	
	
	switch (event)
		
	{
		case EVENT_COMMIT:
				
			
			
			GetCtrlVal(panelHandle , PANEL_LED_K1, &status);
			if (status ==1)
			{
				
				conv_short_to_word(1.0,value);
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8]+ value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
				
				
			}
			else
			{
				
				conv_short_to_word(1.0,value);
				Write_Value_Ptr = GetPointerToWrite_Value();
				(*Write_Value_Ptr)[8] = (*Write_Value_Ptr)[8] - value[1];
				ReleasePointerToWrite_Value();
				
				Write_Flag_Ptr = GetPointerToWrite_Flag();
				(*Write_Flag_Ptr) = 1;
				ReleasePointerToWrite_Flag();
				
			}
			
			
			break;
	}
	return 0;
}
*/

//tiempo = Timer ();













