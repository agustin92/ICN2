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


//============================================================================== 

int CVICALLBACK Test (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{

//Port COM
static int PortCOM = 4;

//Error
int error = -5;

//Buffer send & read
unsigned char send_buffer[16]; //
unsigned char read_buffer[16]; //  


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
			FillBytes (send_buffer, 0, 16, 0x0);
			FillBytes (read_buffer, 0, 16, 0x0);
			
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
			send_buffer[1] = 0x00; //		Number of process data output words
			send_buffer[2] = 0x12; //		Message ident
			send_buffer[3] = 0x01; //		Multipoint address SLAVE 1(The address 1 has been set on the coupler)
			send_buffer[4] = 0x00; //		
			send_buffer[5] = 0x00; //		
			send_buffer[6] = 0x07; //		
			
			
			
			//Request command from master to slave
			for(int i=0; i < 8; i++)
			{
				printf("send_buffer[%d] = \t%X\n",i,send_buffer[i]);
			}
					
			//Write command to COM port
			int write_bytes_counter = 0;
			for(int i=0; i < 2; i++)
			{
				write_bytes_counter = write_bytes_counter + error;
				error = ComWrtByte (PortCOM, send_buffer[i]);
			}
			
			printf("Written bytes = \t%d \n", write_bytes_counter);
			printf("OK\n");
			
			
			//READ			
			for(int i=0; i < 2; i++)
			{
			read_buffer[i] = ComRdByte (PortCOM);
			}
			
			//PRINT
			for(int i=0; i < 2; i++)
			{
			printf("Lee bytes [%d] %x\n",i,  read_buffer[i]);
			}
			
			
			
			
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
			
			//CloseCom (PortCOM);	
			
			
				
			break;
	}
	return 0;
}





//============================================================================== 

/*
unsigned char byte = 33;// unsigned char 1 byte
unsigned char mask = 1; // Bit mask
unsigned char bits[8] ;

*/


/*

// BkComExample.cpp : Defines the entry point for the console application.
//

int main()
{
	long nStatus, nSendLength, nRecLength;
	long lSendBuf[20], lRecBuf[20], lRet;
	int idy, idx;
	char msg[30];

	// Handle ,	Baudrate 38400, COM 1,  2000ms Timeout,	BK8100,
	BkComData aBkComObj = { NULL,	Baud_38400,		2l,		2000l,	BKxType_RS232 };

	if (!OpenBkComPort(&aBkComObj)) 
	{
	    nSendLength = 1; // ein Wort des Prozeßabbildes schreiben
		nRecLength = 1;		// erstes Wort des Prozeßabbildes lesen

		// Lauflicht erzeugen
		for (idy = 0; idy <= 1000; idy++)
		{
			for(idx = 0; idx <= 20; idx++)
			{
				lSendBuf[idx] = lSendBuf[idx] + 1;
				if (lSendBuf[idx] > 10000) 
					lSendBuf[idx] = 0;
			}
										// Multipoint = 2
			lRet = BK8xProcSyncReadWriteReq(&aBkComObj, 2l, &nStatus, nSendLength, lSendBuf, &nRecLength, lRecBuf);
			if (lRet != 0) 
			{
				sprintf(msg, "\nFehler: %ld\n", lRet);
				MessageBox(NULL, msg, "Read Write COM-Port", MB_OK |MB_ICONEXCLAMATION);
			}
		}
	}

	CloseBkComPort(&aBkComObj);
	return 0;
}



	unsigned char	msg[256];	// Message towards slave
	unsigned char	ans[256];	// Message from slave


// Clear buffers
	memset (mb.msg, 0, 256);
	memset (mb.ans, 0, 256);

	// Prepare message
	mb.adr = slave;
	mb.msg[ 0] = mb.adr;		// Slave address 
	mb.msg[ 1] = 0x03;			// modbus function number (3)
	conv_short_to_word (memAdr, mb.msg + 2);
	conv_short_to_word (nRegs, mb.msg + 4);

	// Calculate buffer lenght
	mb.bSent = 6;
	mb.bRcvd = 5 + nRegs * 2;


	printf("hex_msg[0] 0x%02x\n", mb.msg[ 0]);    	//OK
	printf("hex_msg[1] 0x%02x\n", mb.msg[ 1]);    	//OK
	printf("hex_msg[2] 0x%02x\n", mb.msg[ 2]);  	//OK
	printf("hex_msg[3] 0x%02x\n", mb.msg[ 3]);  	//OK
	printf("hex_msg[4] 0x%02x\n", mb.msg[ 4]);  	//OK
	printf("hex_msg[5] 0x%02x\n", mb.msg[ 5]);  	//OK
	printf("hex_msg[6] 0x%02x\n", mb.msg[ 6]);  	//OK
	printf("hex_msg[7] 0x%02x\n", mb.msg[ 7]);  	//OK
	

	
	// Compute CRC and send message
	// Buffer, Size
	
	Calc_CRC (mb.msg, (char *)&mb.bSent);  //CRC and bSent updated
	
	//printf("bSent: %d\n",mb.bSent);	
	
	comerr = ComWrt (portNo, mb.msg, mb.bSent);
	if (comerr < 0) goto Error;
*/

