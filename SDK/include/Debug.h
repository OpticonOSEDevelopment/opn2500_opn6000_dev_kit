/*
 * Debug.h
 *  
 *  Mod. Nr:  
 *  Terminal: 
 *  Compiler: 
 * 
 *  Mod:    
 *  Author: Ronny de Winter
 */

#ifndef DEBUG_H_
#define DEBUG_H_

//Debug levels:
//  0 : Every thing is output. Including rawdata.
//  1 : Only Rawdata is not output.
//  2 : Higher level output.
//  3 : Only function calls, with parameters..
//  4 : Only important function calls
#define DEBUG_LVL 3

#ifdef DEBUG

extern int debug_printf(const char *fmt,...);

#define DEBUGPUTStr     debug_printf 
#define DEBUGPutString	debug_printf 

#if DEBUG_LVL <= 0
#define DEBUG_LVL_0_Printf debug_printf
#else
#define DEBUG_LVL_0_Printf(...)
#endif
#if DEBUG_LVL <= 1
#define DEBUG_LVL_1_Printf debug_printf
#else
#define DEBUG_LVL_1_Printf(...)
#endif
#if DEBUG_LVL <= 2
#define DEBUG_LVL_2_Printf debug_printf
#else
#define DEBUG_LVL_2_Printf(...)
#endif
#if DEBUG_LVL <= 3
#define DEBUG_LVL_3_Printf debug_printf
#else
#define DEBUG_LVL_3_Printf(...)
#endif
#if DEBUG_LVL <= 4
#define DEBUG_LVL_4_Printf	debug_printf
#else
#define DEBUG_LVL_4_Printf(...)
#endif

void NRF_CLI_PROCESS(void);

extern bool nrf_log_frontend_dequeue(void);

#define NRF_LOG_PROCESS	 	nrf_log_frontend_dequeue

#else // No debug
	
#define DEBUGPUTStr
#define DEBUGPutString

#define DEBUG_LVL_0_Printf(...)
#define DEBUG_LVL_1_Printf(...)
#define DEBUG_LVL_2_Printf(...)
#define DEBUG_LVL_3_Printf(...)
#define DEBUG_LVL_4_Printf(...)

#define NRF_LOG_PROCESS()	 	

#endif

#endif /* DEBUG_H_ */
