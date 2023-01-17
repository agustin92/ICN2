/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: panelCB */
#define  PANEL_COMMANDBUTTON_2            2       /* control type: command, callback function: READ_FUNTION */
#define  PANEL_COMMANDBUTTON              3       /* control type: command, callback function: WRITE_FUNTION */
#define  PANEL_pressure_ccr_n             4       /* control type: numeric, callback function: (none) */
#define  PANEL_pressure_pkr_n             5       /* control type: numeric, callback function: (none) */
#define  PANEL_temperature_n              6       /* control type: numeric, callback function: (none) */
#define  PANEL_TOGGLEBUTTON               7       /* control type: textButton, callback function: start_reading */
#define  PANEL_LED_K1                     8       /* control type: LED, callback function: (none) */
#define  PANEL_flux_n                     9       /* control type: numeric, callback function: (none) */
#define  PANEL_LED_ByPass                 10      /* control type: LED, callback function: ByPass */
#define  PANEL_LED_GV                     11      /* control type: LED, callback function: GV */
#define  PANEL_LED_AV                     12      /* control type: LED, callback function: AV */
#define  PANEL_LED_Vent                   13      /* control type: LED, callback function: Vent */
#define  PANEL_LED_TP                     14      /* control type: LED, callback function: (none) */
#define  PANEL_tp_n                       15      /* control type: numeric, callback function: (none) */
#define  PANEL_LED_switch_TP              16      /* control type: LED, callback function: TP_on_off */
#define  PANEL_TOGGLEBUTTON_GV            17      /* control type: textButton, callback function: (none) */
#define  PANEL_LED_Heater                 18      /* control type: LED, callback function: heater_on_off */
#define  PANEL_NUMERIC_4                  19      /* control type: numeric, callback function: heater_output */
#define  PANEL_NUMERIC_3                  20      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_2                  21      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC                    22      /* control type: numeric, callback function: (none) */


     /* Control Arrays: */

#define  CTRLARRAY                        1

     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK AV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ByPass(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK heater_on_off(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK heater_output(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK READ_FUNTION(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK start_reading(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TP_on_off(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Vent(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK WRITE_FUNTION(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif