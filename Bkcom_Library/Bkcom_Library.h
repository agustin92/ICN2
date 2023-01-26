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
#define  PANEL_pressure_ccr_n             3       /* control type: numeric, callback function: (none) */
#define  PANEL_pressure_pkr_n             4       /* control type: numeric, callback function: (none) */
#define  PANEL_temperature_n              5       /* control type: numeric, callback function: (none) */
#define  PANEL_TOGGLEBUTTON               6       /* control type: textButton, callback function: start_reading */
#define  PANEL_LED_K1                     7       /* control type: LED, callback function: (none) */
#define  PANEL_flux_n                     8       /* control type: numeric, callback function: (none) */
#define  PANEL_LED_ByPass                 9       /* control type: LED, callback function: ByPass */
#define  PANEL_LED_GV                     10      /* control type: LED, callback function: GV */
#define  PANEL_LED_AV                     11      /* control type: LED, callback function: AV */
#define  PANEL_LED_Vent                   12      /* control type: LED, callback function: Vent */
#define  PANEL_LED_TP                     13      /* control type: LED, callback function: (none) */
#define  PANEL_tp_n                       14      /* control type: numeric, callback function: (none) */
#define  PANEL_LED_switch_TP              15      /* control type: LED, callback function: TP_on_off */
#define  PANEL_TOGGLEBUTTON_GV            16      /* control type: textButton, callback function: (none) */
#define  PANEL_LED_Heater                 17      /* control type: LED, callback function: heater_on_off */
#define  PANEL_NUMERIC_4                  18      /* control type: numeric, callback function: (none) */
#define  PANEL_Temperature_Set_Point      19      /* control type: numeric, callback function: change_temperature_set_point */
#define  PANEL_Rate                       20      /* control type: numeric, callback function: change_rate */
#define  PANEL_temperature_calc           21      /* control type: numeric, callback function: (none) */
#define  PANEL_error_temp                 22      /* control type: numeric, callback function: (none) */
#define  PANEL_d                          23      /* control type: numeric, callback function: (none) */
#define  PANEL_i                          24      /* control type: numeric, callback function: (none) */
#define  PANEL_p                          25      /* control type: numeric, callback function: (none) */
#define  PANEL_kd                         26      /* control type: numeric, callback function: (none) */
#define  PANEL_ki                         27      /* control type: numeric, callback function: (none) */
#define  PANEL_kp                         28      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC                    29      /* control type: numeric, callback function: (none) */
#define  PANEL_Ar_output                  30      /* control type: numeric, callback function: Ar_flux */
#define  PANEL_LED_Gas                    31      /* control type: LED, callback function: Gas_valve */
#define  PANEL_LED_Motor_rotation         32      /* control type: LED, callback function: (none) */
#define  PANEL_Motor_index                33      /* control type: numeric, callback function: (none) */
#define  PANEL_LED_toggle                 34      /* control type: LED, callback function: (none) */
#define  PANEL_toggle_time                35      /* control type: numeric, callback function: (none) */
#define  PANEL_amplitude_steps            36      /* control type: numeric, callback function: (none) */


     /* Control Arrays: */

#define  CTRLARRAY                        1

     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK Ar_flux(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK AV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ByPass(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK change_rate(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK change_temperature_set_point(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Gas_valve(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK GV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK heater_on_off(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK panelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK READ_FUNTION(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK start_reading(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK TP_on_off(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Vent(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif