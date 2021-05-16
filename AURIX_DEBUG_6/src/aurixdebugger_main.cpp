/*****************************************************************************
 *
 * Copyright (C) 2007-2012 Infineon Technologies AG. All rights reserved.
 *
 * Infineon Technologies AG (Infineon) is supplying this software for use with
 * Infineon's microcontrollers.  This file can be freely used for creating
 * development tools that are supporting such microcontrollers.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON 
 * WHATSOEVER.
 *
 ******************************************************************************
 * MODULE:  AurixDebugger is based on mcd_demo_main.cpp
 * VERSION: see GitHub at https://github.com/aurixinino/aurixocd
 ******************************************************************************
 * DESCRIPTION: 
 * Simple example for the usage of the MCD API
 * Tested with TriCore AURIX evaluation board using DAS release V4.1. 
 *		(Server library V3.0 and DAS API V4.1)
 * DAS Installer (includes mcdxdas.dll) is available at www.infineon.com/DAS             
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mcd_api.h"
#include "mcd_tools.h"
#include "mcd_loader_class.h"


//-------------------------------------------------------------------------------------------------
// Global MCD API loader class pointer
McdLoaderClass* mcd;

//-------------------------------------------------------------------------------------------------
void mcdd_get_core_ip_addr(mcd_core_st *core, mcd_register_info_st *core_ip_reg);

void mcdd_handle_err(FILE *lf, mcd_core_st **core, mcd_return_et ret);

mcd_return_et mcdd_read_block(mcd_core_st *core, const mcd_addr_st *addr, uint32_t num_bytes);

mcd_return_et mcdd_read_core_ip(mcd_core_st *core, const mcd_register_info_st *core_ip_reg, 
                                uint32_t *core_ip);

//-------------------------------------------------------------------------------------------------
// Server related functions
void mcdd_open_servers(const char *g_sysKey, const char *config_string,
                       uint32_t *num_servers, mcd_server_st *server);

void mcdd_select_running_server(const char *g_serverIP, const char *g_sysKey, uint32_t *num_servers, 
                                mcd_server_st **server);

void mcdd_start_servers(const char *g_serverIP, const char *g_sysKey, uint32_t *num_servers, 
                        mcd_server_st **server);

mcd_return_et mcdd_set_acc_hw_frequency(mcd_server_st *server, uint32_t frequ);


//****************************************************************************
// @Prototypes Of Local Functions
//****************************************************************************
void print_help(void);
void print_CPU_registers(mcd_core_st *core, uint32_t i_core, mcd_return_et ret);
void Debug_Trace(mcd_core_st *core, uint32_t i_core, mcd_return_et ret);


//****************************************************************************
// @MAIN
//****************************************************************************
int main(int argc, char** argv) 
{
    /* MAIN GLOBAL VARIABLES */
	mcd_return_et			ret, ret1;
	mcd_core_con_info_st	coreConInfo;
	uint32_t				i, sv, tmp, numOpenServers, numSystems;
	mcd_impl_version_info_st mcd_impl_info;
	mcd_api_version_st		versionReq;
	mcd_core_state_st		state;
	mcd_core_con_info_st	core_con_info_system_common;
	mcd_tx_st				txDemo;
	mcd_trig_simple_core_st	trigDemo;


	/* Command Line related variables */
	char					g_mcdapilib[64]	= "mcdxdas.dll";	// DAS Library DLL name
	char					g_serverIP[64]	= "localhost";		// DAS Server name
	char					g_sysKey[MCD_KEY_LEN];				// System protection Key
	const uint32_t			maxNumServers	= 16;				// max Number of DAS servers
	mcd_server_st			*openServers[maxNumServers];		// pointer to DAS server list
	uint32_t				i_device		= 0;				// DAS device index (>0 if more devices are connected)
	uint32_t				i_system		= 0;				// DAS system index (>0 if more systems are connected)
	uint32_t				num_cores		= 0;				// Target number of cores (e.g. TC275x has 3 cores)
	uint32_t				i_core			= 0;				// Target selected core (e.g. TC275x has 3 cores, we like to debug CPU_x)
	char					line[256], cmd[16], parStr0[64], parStr1[64], parStr2[64];

	/* Variables initialization */
	g_sysKey[0] = 0;
	versionReq.v_api_major  = MCD_API_VER_MAJOR;
	versionReq.v_api_minor  = MCD_API_VER_MINOR;
	strcpy(versionReq.author, MCD_API_VER_AUTHOR);

	/*
	 * COMMAND LINE INTERFACE
	 */

	for (int iCnt = 0; iCnt < argc; iCnt++)
	{
		// print Help
		if (strncmp(argv[iCnt], "-help", 10) == 0) 
		{
			printf("\n###############################################################################\n");
			printf ("AurixDebugger Help - Command Line Interface:\n\n");
			printf ("    -help:\tprint this help.\n");
			printf ("    -about:\tgives the GitHub location where to find the latest version.\n");
			printf ("    -mcdapilib:\tpermit to set the DAS API Library name (e.g.mcdxdas.dll).\n");
			printf ("    -serverIP:\tpermit to set the DAS server address for remote connection to\n");
			printf ("              \ttarget (e.g. localhost).\n");
			printf ("    -key:\tpermit to set the device key in case it requires one to permi\n");
			printf ("         \tthe debug access.\n");
			printf("\n###############################################################################\n");
			printf ("\n\n");
			return (0);
		}
		// Provide GitHub address
		if (strncmp(argv[iCnt], "-about", 10) == 0) 
		{
			printf("\n###############################################################################\n");
			printf ("AurixDebugger About.\n\n");
			printf ("    Visit:\t\t https://github.com/aurixinino/aurixocd \n");
			printf ("    Documentation:\t https://github.com/aurixinino/aurixocd/wiki \n");
			printf("\n###############################################################################\n");
			return (0);
		}
		// Set MCD API Lib name
		if (strncmp(argv[iCnt], "-mcdapilib", 10) == 0) 
		{
			iCnt += 1; // next is the MCD API Lib name
			if (strlen(argv[iCnt]) > 0)
			{
				strcpy(g_mcdapilib, argv[iCnt]);
			}
		}
		// Set IP address of server g_serverIP
		if (strncmp(argv[iCnt], "-serverIP", 10) == 0) 
		{
			iCnt += 1; // next is the IP address
			if (strlen(argv[iCnt]) > 0)
			{
				strcpy(g_serverIP, argv[iCnt]);
			}
		}
		// Set System key
		if (strncmp(argv[iCnt], "-key", 8) == 0) 
		{
			iCnt += 1; // next is the IP address
			if (strlen(argv[iCnt]) > 0)
			{
				strcpy(g_sysKey, argv[iCnt]);
			}
		}
	}

	printf("\nAURIX DEBUG CONFIG ############################################################\n");
	printf("MCD API lib: \t\t%s\n",						g_mcdapilib);
	printf("IP address of server: \t%s\n",				g_serverIP);
	printf("System key: \t\t%s\n",						g_sysKey);
	printf("###############################################################################\n");

	// Load and initialize MCD API 	
	mcd = new McdLoaderClass(g_mcdapilib);
	ret = mcd->mcd_initialize_f(&versionReq, &mcd_impl_info);
	mcdt_print_mcd_impl_info(stdout, &mcd_impl_info);

	// Query number of running servers and start server if none is running
	numOpenServers = 0;
	ret = mcd->mcd_qry_servers_f(g_serverIP, TRUE, 0, &numOpenServers, 0);
	//assert(ret == MCD_RET_ACT_NONE);

	if (numOpenServers == 0) {
		numOpenServers = maxNumServers;
		mcdd_start_servers(g_serverIP, g_sysKey, &numOpenServers, openServers);
	}
	else {
		numOpenServers = maxNumServers;
		mcdd_select_running_server(g_serverIP, g_sysKey, &numOpenServers, openServers);
	}
	if(numOpenServers == 0) {
		printf("\n###############################################################################\n");
		printf("ERROR: Cannot start - missing running Hardware - please connect an AURIX board.");
		printf("\n###############################################################################\n");
		return (-1);
	}


	/* 
	 * DAS Server Open and Initialization
	 */

	printf("\nSYSTEM LEVEL ##################################################################\n");

	// Check and count number of available systems targets connected to this DAS server
	numSystems = 0;
	ret = mcd->mcd_qry_systems_f(0, &numSystems, 0);
	printf("Found %d systems on host %s\n\n", numSystems, g_serverIP);
	for (i = 0; i < numSystems; i++) 
	{
		tmp = 1;
		ret = mcd->mcd_qry_systems_f(i, &tmp, &coreConInfo);
		mcdt_print_core_con_info(stdout, &coreConInfo);
		printf("\n");
	}
	if(numSystems == 0) 
	{
		printf("\n###############################################################################\n");
		printf("ERROR: Cannot start - missing running Hardware - please connect an AURIX board.");
		printf("\n###############################################################################\n");
		return (-1);
	}

	// Select on what target system to operate 
	if (numSystems == 1) 
		i_system = 0; // if only 1 target available, select that without questioning
	else
	{
		printf("\nEnter system index (0...%d):\n", numSystems - 1);
		scanf("%i", &i_system);
	}

	tmp = 1;
	ret = mcd->mcd_qry_systems_f(i_system, &tmp, &core_con_info_system_common);


	printf("\nDEVICE LEVEL ##################################################################\n");

	// Number of devices
	uint32_t num_devices = 0;
	ret = mcd->mcd_qry_devices_f(&core_con_info_system_common, 0, &num_devices, 0);
	//assert(ret == MCD_RET_ACT_NONE);
	//assert(num_devices > 0);


	printf("Found %d devices within system %s\n\n", num_devices, core_con_info_system_common.system);

	for (i = 0; i < num_devices; i++) {
	tmp = 1;
	ret = mcd->mcd_qry_devices_f(&core_con_info_system_common, i, &tmp, &coreConInfo);
	//assert(ret == MCD_RET_ACT_NONE);
	mcdt_print_core_con_info(stdout, &coreConInfo);
	printf("\n");
	}

	// Select device
	if (num_devices > 1) {
	printf("\nEnter device index (0...%d):\n", num_devices - 1);
	scanf("%i", &i_device);
	}

	mcd_core_con_info_st core_con_info_device_common;
	tmp = 1;
	ret = mcd->mcd_qry_devices_f(&core_con_info_system_common, i_device, &tmp, 
							&core_con_info_device_common);
	//assert(ret == MCD_RET_ACT_NONE);

	printf("\nCORE LEVEL ####################################################################\n");

	// Number of cores
	ret = mcd->mcd_qry_cores_f(&core_con_info_device_common, 0, &num_cores, 0);
	//assert(ret == MCD_RET_ACT_NONE);
	//assert(num_cores >= 1);

	// Print Core information (e.g. CPU type, etc..)
	printf("Found %d cores within device %s\n\n", num_cores, core_con_info_device_common.device);
	for (i = 0; i < num_cores; i++) 
	{
		tmp = 1;
		ret = mcd->mcd_qry_cores_f(&core_con_info_device_common, i, &tmp, &coreConInfo);
		//assert(ret == MCD_RET_ACT_NONE);
		//assert(strcmp(coreConInfo.g_serverIP, core_con_info_device_common.g_serverIP) == 0);
		//assert(strcmp(coreConInfo.g_sysKey, core_con_info_device_common.g_sysKey) == 0);
		//assert(coreConInfo.device_key[0] == 0);  // Safe assumption for models
		//assert(strcmp(coreConInfo.system, core_con_info_device_common.system) == 0);
		mcdt_print_core_con_info(stdout, &coreConInfo);
		printf("\n");
	}

	// Select core
	if (num_cores > 1) {
	printf("\nEnter core index (0...%d):\n", num_cores - 1);
	scanf("%i", &i_core);
	}

	mcd_core_con_info_st core_con_info_core;
	tmp = 1;
	ret = mcd->mcd_qry_cores_f(&core_con_info_device_common, i_core, &tmp, &core_con_info_core);
	//assert(ret == MCD_RET_ACT_NONE);

	// Open core
	mcd_core_st *core;
	ret = mcd->mcd_open_core_f(&core_con_info_core, &core);
	mcdd_handle_err(stdout, 0, ret);
	//assert(core != NULL);

	mcd_register_info_st core_ip_reg;
	mcdd_get_core_ip_addr(core, &core_ip_reg);

	// Close not needed open servers
	mcd_server_st *server = NULL; // Needed to set frequency of Access HW
	for (sv = 0; sv < numOpenServers; sv++) {
	if (mcdt_check_if_server_used(&core_con_info_core, openServers[sv]->config_string)) {
		//assert(server == NULL);
		server = openServers[sv];
		continue;
	}
	ret = mcd->mcd_close_server_f(openServers[sv]);
	}
	//assert(server != NULL);

  // Key(s) for Locked Devices
  // It is assumed that a device type recognition (device type ID) is possible for a locked device.
  // Note that as a difference to silicon, device models are never locked (server key is sufficient).
  // The device key only needs to be provided with mcd_core_con_info_st, when the core is openend.

  uint32_t  value, core_ip;

  // Use strongest reset
  uint32_t rstClassVectorAvail, rstClassVector = 1; 
  ret = mcd->mcd_qry_rst_classes_f(core, &rstClassVectorAvail);
  //assert(ret == MCD_RET_ACT_NONE);
  //assert(rstClassVectorAvail & rstClassVector);

  // tx and txlist setup
  memset(&txDemo, 0, sizeof(txDemo));  // Set all to default values
  mcd_txlist_st  txlistDemo;
  txlistDemo.tx     = &txDemo;
  txlistDemo.num_tx = 1;
  txDemo.num_bytes = sizeof(value);
  txDemo.data      = (uint8_t*) &value;

  // trig setup
  memset(&trigDemo, 0, sizeof(mcd_trig_simple_core_st));  // Set all to default values
  trigDemo.struct_size = sizeof(mcd_trig_simple_core_st);
  trigDemo.type        = MCD_TRIG_TYPE_IP;
  trigDemo.action      = MCD_TRIG_ACTION_DBG_DEBUG;
  trigDemo.option      = MCD_TRIG_OPT_DEFAULT;
  trigDemo.addr_range  = 0;  // Single IP trigger


  	/* 
	 * INTERACTIVE INTERFACE
	 */

	// Menu Help function
	print_help();

	// Command parser
	gets(line);  // Not clear why this is needed here to empty the input buffer?

  // Main loop
  while (true) {

    core_ip = 0xEEEEEEEE;
    state.state = MCD_CORE_STATE_UNKNOWN;

	// E.g. miniWiggler was unplugged
    if (core == NULL) {  
      // Trying to reconnect core
      ret = mcd->mcd_open_core_f(&core_con_info_core, &core);
      if (ret == MCD_RET_ACT_NONE)
        printf("Core successfully reconnected\n");
      else
        mcdd_handle_err(stdout, &core, ret);
    }

    if (core != NULL) {
      ret1 = mcd->mcd_qry_state_f(core, &state);

      if (ret1 != MCD_RET_ACT_HANDLE_ERROR)  // Avoid double notification
        ret1 = mcdd_read_core_ip(core, &core_ip_reg, &core_ip);

      mcdd_handle_err(stdout, &core, ret1);
    }

    printf("IP 0x%8.8X State %-8s Enter command: ", core_ip, mcdt_get_core_state_string(state.state)); 

    uint32_t  param0, param1, param2, n_items;
    gets(line);  // Not safe I know...
    n_items = sscanf(line, "%s %s %s %s", cmd, parStr0, parStr1, parStr2);

    if (core == NULL)
      continue; // Try to reconnect core in the beginning of the while loop

    if (n_items > 1) {
      if (strncmp(parStr0, "0x", 2) == 0)
        sscanf(parStr0, "%x", &param0);
      else
        sscanf(parStr0, "%d", &param0);
    }
    if (n_items > 2) {
      if (strncmp(parStr1, "0x", 2) == 0)
        sscanf(parStr1, "%x", &param1);
      else
        sscanf(parStr1, "%d", &param1);
    }
    if (n_items > 3) {
      if (strncmp(parStr2, "0x", 2) == 0)
        sscanf(parStr2, "%x", &param2);
      else
        sscanf(parStr2, "%d", &param2);
    }

	// EXIT or QUIT
    if ( (strncmp(cmd, "exit", 8) == 0) || (strncmp(cmd, "q", 1) == 0) ) {
      break;
    }
	// HELP or ?
	if ( (strncmp(cmd, "help", 8) == 0) || (strncmp(cmd, "?", 1) == 0) ) {
		print_help();
		continue;
	}
	// BREAKPOINT
    else if (strncmp(cmd, "bpt", 8) == 0) {
      if (state.state != MCD_CORE_STATE_DEBUG) {
        printf("Breakpoints can be only changed in Debug state\n");
      }
      else {
        ret = mcd->mcd_remove_trig_set_f(core); 
        mcdd_handle_err(stdout, &core, ret);
        if (n_items > 1) { // Setup new IP breakpoint
          trigDemo.addr_start.address = param0;
          uint32_t trigId;
          ret = mcd->mcd_create_trig_f(core, &trigDemo, &trigId);
          mcdd_handle_err(stdout, &core, ret);
        }
      }
	  continue;
    }
	// DUMP viewer
	else if (strncmp(cmd, "dump", 8) == 0) {
      if (n_items < 3) {
        printf("Syntax: dump <addr> <n_bytes>\n");
        continue;
      }

      txDemo.addr.address = param0;
	  for (uint32_t i=0; i < param1; i++)
	  {
		if (i%4 == 0)
			printf("\n0x%8.8X: ", txDemo.addr.address);
		value = 0xEEEEEEEE;   
		ret = mcd->read32(core, &txDemo.addr, &value);
		mcdd_handle_err(stdout, &core, ret);
		printf("0x%8.8X ", value);
		txDemo.addr.address += 4;
	  }
	  printf("\n");
	  continue;
	}
	// ASCII viewer
	else if (strncmp(cmd, "ascii", 8) == 0) {
      if (n_items < 3) {
        printf("Syntax: ascii <addr> <n_bytes>\n");
        continue;
      }

	  // Local variables
	  char c0, c1, c2, c3;

      txDemo.addr.address = param0;
	  for (uint32_t i=0; i < param1; i++)
	  {
	    value = 0xEEEEEEEE;   
        ret = mcd->read32(core, &txDemo.addr, &value);
        mcdd_handle_err(stdout, &core, ret);
		// Now print it as HEX and ASCII - byte by byte... little to big endian format
		printf("0x%8.8X: ", txDemo.addr.address);
		c0 = (value&0xFF);
		printf("0x%2.2X-", c0);
		c1 = (value>>8)&0xFF;
		printf("0x%2.2X-", c1);
		c2 = (value>>16)&0xFF;
		printf("0x%2.2X-", c2);
		c3 = (value>>24)&0xFF;
		printf("0x%2.2X  ", c3);
		printf("%c %c %c %c\n", c0, c1, c2, c3);
		txDemo.addr.address += 4;
	  }
	  continue;
	}	
	// SEEK
	else if (strncmp(cmd, "seek", 8) == 0) {
      if (n_items < 4) {
        printf("Syntax: seek <addr> <value> <size>\n");
        continue;
      }

	  bool bFound = FALSE;
      txDemo.addr.address = param0;
	  for (uint32_t i=0; i < param2; i++)
	  {
		  // Scan the memory range seeking for the given 32-bit value
	    value = 0xEEEEEEEE;   
        ret = mcd->read32(core, &txDemo.addr, &value);
        mcdd_handle_err(stdout, &core, ret);
		if (ret != MCD_RET_ACT_NONE) break;
		if (value == param1) {
			printf("\nSeek FOUND: 0x%8.8X", param1);
			printf(" at addr: 0x%8.8X\n", txDemo.addr.address);
			bFound = TRUE;
			break;
		}
		else
			printf(".");
		txDemo.addr.address += 4;
	  }
	  if (!bFound) 
		  printf("\nSeek FAIL: <value> unfound in the given range [0x%8.8X - 0x%8.8X]\n", param0, txDemo.addr.address);
	  continue;
	}
    else if (strncmp(cmd, "read", 8) == 0) {
      if (n_items < 2) {
        printf("Syntax: read <addr>\n");
        continue;
      }

      txDemo.addr.address = param0;

      if (n_items == 3) {
        ret = mcdd_read_block(core, &txDemo.addr, param1);
        mcdd_handle_err(stdout, &core, ret);
      }
      else {
        value = 0xEEEEEEEE;   
        ret = mcd->read32(core, &txDemo.addr, &value); 
        mcdd_handle_err(stdout, &core, ret);
        printf("Read value: 0x%8.8X\n", value);
      }
	  continue;
    }
    // READ 8-bit
    else if (strncmp(cmd, "r8", 8) == 0) {
      if (n_items < 2) {
        printf("Syntax: read <addr>\n");
        continue;
      } else {
		uint8_t  value_8 = 0xEE;   
		txDemo.addr.address = param0;
		ret = mcd->read8(core, &txDemo.addr, &value_8); 
		mcdd_handle_err(stdout, &core, ret);
		printf("Read value: 0x%2.2X\n", value_8);
	  }
	  continue;
    }    
	else if (strncmp(cmd, "rst", 8) == 0) {
      ret =  mcd->mcd_rst_f(core, rstClassVector, FALSE); 
      mcdd_handle_err(stdout, &core, ret);
	  continue;
    }
    else if (strncmp(cmd, "rsthlt", 8) == 0) {
      ret =  mcd->mcd_rst_f(core, rstClassVector, TRUE); 
      mcdd_handle_err(stdout, &core, ret);
	  continue;
    }
    if ( (strncmp(cmd, "run", 8) == 0) || (strncmp(cmd, "g", 1) == 0) ) {
      ret =  mcd->mcd_activate_trig_set_f(core); 
      mcdd_handle_err(stdout, &core, ret);
      ret =  mcd->mcd_run_f(core, FALSE); 
      mcdd_handle_err(stdout, &core, ret);
	  continue;
    }
    else if (strncmp(cmd, "stop", 8) == 0) {
      ret =  mcd->mcd_stop_f(core, FALSE); 
      mcdd_handle_err(stdout, &core, ret);
	  continue;
    }
    else if (strncmp(cmd, "step", 8) == 0) {
      uint32_t n_steps = 1;
      if (n_items > 1)
        n_steps = param0;
      ret =  mcd->mcd_step_f(core, FALSE, MCD_CORE_STEP_TYPE_INSTR, n_steps); 
      mcdd_handle_err(stdout, &core, ret);
	  continue;
    }
	if ( (strncmp(cmd, "trace", 8) == 0) || (strncmp(cmd, "t", 1) == 0) ) {
      	Debug_Trace(core, i_core, ret);
	  continue;
    }

	// CPU Registers
	if ( strncmp(cmd, "r", 1) == 0) {				
		print_CPU_registers(core, i_core, ret);
		continue;
	}
    // WRITE 8-bit
	else if (strncmp(cmd, "write", 8) == 0) {
      if (n_items < 3) {
        printf("Syntax: write <addr> <value>\n");
        continue;
      }
      txDemo.access_type  = MCD_TX_AT_W;
      txDemo.addr.address = param0;
      value               = param1;

      ret = mcd->mcd_execute_txlist_f(core, &txlistDemo);
      mcdd_handle_err(stdout, &core, ret);
	  continue;
    }
    else if (strncmp(cmd, "w8", 8) == 0) {
      if (n_items < 3) {
        printf("Syntax: w8 <addr> <value>\n");
        continue;
      }
		mcd_return_et ret;
		txDemo.addr.address = param0;
		value               = param1;
		ret = mcd->write8(core, &txDemo.addr, value);
		//assert(ret == MCD_RET_ACT_NONE);		
		continue;
    }    
	else if (strncmp(cmd, "frequ", 8) == 0) {
      if (n_items < 2) {
        printf("Syntax: frequ <f_hz>\n");
        continue;
      }
      ret = mcdd_set_acc_hw_frequency(server, param0);
      mcdd_handle_err(stdout, NULL, ret);  // Note not core related
	  continue;
    }
    else {
      if (strlen(cmd) != 0) {
        printf("Unknown command\n");
      }
    }
  }

  // Close core
  ret = mcd->mcd_close_core_f(core);
  //assert(ret == MCD_RET_ACT_NONE);

  // Cleanup
  mcd->mcd_exit_f(); // Enforce cleanup of all core and server connections 
  delete mcd;        // Unloads lib (destructor of McdLoaderClass)

  return 0;
}


//****************************************************************************
// @Local Functions
//****************************************************************************

//-------------------------------------------------------------------------------------------------
void mcdd_get_core_ip_addr(mcd_core_st *core, mcd_register_info_st *core_ip_reg)
{
  mcd_return_et ret;
  uint32_t reg_group_id, i, num_regs, num_regs_tmp;

  reg_group_id = 0; // Default (at least for TriCore, XMC4000, XC2000, XE166 and XC800)

  num_regs = 0; // Just query number
  ret = mcd->mcd_qry_reg_map_f(core, reg_group_id, 0, &num_regs, core_ip_reg);
  //assert(ret == MCD_RET_ACT_NONE);

  for (i = 0; i < num_regs; i++) {
    num_regs_tmp = 1;
    ret = mcd->mcd_qry_reg_map_f(core, reg_group_id, i, &num_regs_tmp, core_ip_reg);
    //assert(ret == MCD_RET_ACT_NONE);
    if (   (strcmp(core_ip_reg->regname, "PC") == 0) 
        || (strcmp(core_ip_reg->regname, "IP") == 0) ) {
      break;
    }
  }
  //assert(i < num_regs);
}


//-------------------------------------------------------------------------------------------------
void mcdd_handle_err(FILE *lf, mcd_core_st **core, mcd_return_et ret)
{
  if (ret == MCD_RET_ACT_NONE)
    return;

  mcd_error_info_st errInfo;

  if (core == NULL)
    mcd->mcd_qry_error_info_f(NULL, &errInfo);
  else
    mcd->mcd_qry_error_info_f(*core, &errInfo);

  // Handle events
  if (errInfo.error_events & MCD_ERR_EVT_RESET)
    fprintf(lf, "EVENT: Target has been reset\n");
  if (errInfo.error_events & MCD_ERR_EVT_PWRDN)
    fprintf(lf, "EVENT: Target has been powered down\n");
  if (errInfo.error_events & MCD_ERR_EVT_HWFAILURE)
    fprintf(lf, "EVENT: There has been a target hardware failure\n");
  if (errInfo.error_events & ~7) { // Not MCD_ERR_EVT_RESET, _PWRDN, _HWFAILURE
    //assert(false);
    fprintf(lf, "EVENT: There has been an unknown event\n");
  }

  if (ret == MCD_RET_ACT_HANDLE_EVENT) {
    return;  // Nothing to do
  }
  
  //assert(ret == MCD_RET_ACT_HANDLE_ERROR);
  //assert(errInfo.error_str[0] != 0);
  fprintf(lf, "ERROR: %s\n", errInfo.error_str);

  if (errInfo.error_code == MCD_ERR_CONNECTION) {  // E.g. miniWiggler was unplugged
    if ((core != NULL) && (*core != NULL)) {
      mcd->mcd_close_core_f(*core);
      *core = NULL;  // Will try to reconnect in main loop
    }
  }
}


//-------------------------------------------------------------------------------------------------
// The number of opened servers depends on how specific the config string is.
// E.g. in case of Real HW, whether it contains the name of the tool Access HW.
void mcdd_open_servers(const char *g_sysKey, const char *config_string,
                           uint32_t *num_servers, mcd_server_st **server)
{
  mcd_return_et ret;
  uint32_t sv;

  printf("\nOpen Servers\n\n");

  for (sv = 0; sv < *num_servers; sv++) { 

    ret = mcd->mcd_open_server_f(g_sysKey, config_string, &server[sv]);

    if (ret != MCD_RET_ACT_NONE) {
      //assert(ret == MCD_RET_ACT_HANDLE_ERROR);
      mcd_error_info_st errInfo;
      mcd->mcd_qry_error_info_f(0, &errInfo);
      break;  // while
    }
    printf("%s\n", server[sv]->config_string);
  }

  *num_servers = sv;
}

//-------------------------------------------------------------------------------------------------
mcd_return_et mcdd_read_core_ip(mcd_core_st *core, const mcd_register_info_st *core_ip_reg, 
                                uint32_t *core_ip)
{
  mcd_return_et ret = MCD_RET_ACT_HANDLE_ERROR;

  switch (core_ip_reg->regsize) {
    case 32:
      ret = mcd->read32(core, &core_ip_reg->addr, core_ip);
      break;
    case 16:
      uint16_t core_ip16;
      ret = mcd->read16(core, &core_ip_reg->addr, &core_ip16);
      *core_ip = core_ip16;
    break;
    default:
      //assert(false);
		ret = MCD_RET_ACT_HANDLE_ERROR; 
  }
  return ret;
}


//-------------------------------------------------------------------------------------------------
mcd_return_et mcdd_set_acc_hw_frequency(mcd_server_st *server, uint32_t frequ)
{
  mcd_return_et ret;

  uint32_t frequNew = frequ;
  ret = mcd->set_acc_hw_frequency(server, &frequNew);

  if (ret == MCD_RET_ACT_NONE)
    printf("Frequency set to %d kHz\n", frequNew/1000);
  else
    printf("Could not set frequency\n");
 
  return ret;
}


//-------------------------------------------------------------------------------------------------
// In mcdxdas.dll V1.4.0 the standard approach doesn't work due to an implementation bug
void mcdd_set_acc_hw_frequency_mcdxdas_v140_workaround(mcd_core_st *core, uint32_t frequ)
{
  // Using some magic...
  mcd_addr_st addr;
  memset(&addr, 0, sizeof(mcd_addr_st));
  addr.address      = 0x000C0100;
  addr.addr_space_id = 0xDADADA84;

  mcd_return_et ret;
  ret = mcd->write32(core, &addr, frequ);
  //assert(ret == MCD_RET_ACT_NONE);

  uint32_t frequNew;
  ret = mcd->read32(core, &addr, &frequNew);
  //assert(ret == MCD_RET_ACT_NONE);

  printf("Frequency set to %d kHz\n", frequNew/1000);
}


//-------------------------------------------------------------------------------------------------
void mcdd_select_running_server(const char *g_serverIP, const char *g_sysKey, uint32_t *num_servers, 
                                mcd_server_st **server)
{ 
  mcd_return_et ret;

  uint32_t i, sv, numRunningServers, notOnlyTheSelectedAccHw;
  const uint32_t maxNumServers = 16;
  mcd_server_info_st serverInfo[maxNumServers];

  numRunningServers = maxNumServers;
  ret = mcd->mcd_qry_servers_f(g_serverIP, TRUE, 0, &numRunningServers, serverInfo);
  //assert(ret == MCD_RET_ACT_NONE);

  printf("\nRunning Servers\n");
  mcdt_print_server_info(stdout, numRunningServers, serverInfo);
  printf("\n");
  
  // Select the only running server if only one available
  if (numRunningServers == 1) 
  {
	sv = 0;	
  }
  else if (numRunningServers > 1) 
  {
    printf("\nEnter server index (0...%d):\n", numRunningServers - 1);
    scanf("%i", &sv);
    //assert(sv < numRunningServers);

    // Check if there are different Access HWs for the same kind of server
    for (i = 0; i < numRunningServers; i++) {
      if (i == sv)
        continue;
      if (serverInfo[i].acc_hw[0] == 0)
        continue;
      if (strncmp(serverInfo[i].server, serverInfo[sv].server, MCD_UNIQUE_NAME_LEN) == 0)
        break;
    }
    if (i < numRunningServers) 
	{
      printf("\nEnter 0 to open only the selected Access HW server:\n");
      scanf("%i", &notOnlyTheSelectedAccHw);
    }
  }

  char  configString[256];
  if (serverInfo[sv].acc_hw[0] != 0) {  // Real HW
    //assert(serverInfo[sv].system_instance[0] == 0); 
    if (notOnlyTheSelectedAccHw)
      sprintf(configString, "McdHostName=\"%s\"\nMcdServerName=\"%s\" ", 
              g_serverIP, serverInfo[sv].server);
    else
      sprintf(configString, "McdHostName=\"%s\"\nMcdServerName=\"%s\"\nMcdAccHw=\"%s\" ", 
              g_serverIP, serverInfo[sv].server, serverInfo[sv].acc_hw);
  }
  else if (serverInfo[sv].system_instance[0] != 0) {  // Simulation model
    //assert(serverInfo[sv].acc_hw[0] == 0); 
    sprintf(configString, "McdHostName=\"%s\"\nMcdServerName=\"%s\"\nMcdSystemInstance=\"%s\" ", 
            g_serverIP, serverInfo[sv].server, serverInfo[sv].system_instance);
  } else {  // Not a good MCD API implementation
    //assert(false);
    sprintf(configString, "McdHostName=\"%s\"\nMcdServerName=\"%s\" ", 
            g_serverIP, serverInfo[sv].server);
  }

  mcdd_open_servers(g_sysKey, configString, num_servers, server);
}


//-------------------------------------------------------------------------------------------------
void mcdd_start_servers(const char *g_serverIP, const char *g_sysKey, uint32_t *num_servers, 
                        mcd_server_st **server)
{
  mcd_return_et ret;
  uint32_t      sv, numInstalledServers;

  const uint32_t maxNumInstalledServers = 16;
  mcd_server_info_st serverInfo[maxNumInstalledServers];

  // Query installed servers
  numInstalledServers = maxNumInstalledServers;
  ret = mcd->mcd_qry_servers_f(g_serverIP, FALSE, 0, &numInstalledServers, serverInfo);
  //assert(ret == MCD_RET_ACT_NONE);

  printf("\nInstalled Servers:\n");
  mcdt_print_server_info(stdout, numInstalledServers, serverInfo);

  //assert(sv < numInstalledServers);
  if (numInstalledServers == 1)
	  sv = 0;
  else
  {
	printf("\n\nEnter server index (0...%d) to start server\n", num_servers - 1);
	scanf("%i", &sv);
  }

  char configString[128];
  sprintf(configString, "McdHostName=\"%s\"\nMcdServerName=\"%s\" ", g_serverIP, serverInfo[sv].server);
  
  // In case of Real HW, servers for all different Access HWs will be openend
  // If several boards are connected, all devices will be available for the selection process
  mcdd_open_servers(g_sysKey, configString, num_servers, server);
}


//-------------------------------------------------------------------------------------------------
mcd_return_et mcdd_read_block(mcd_core_st *core, const mcd_addr_st *addr, uint32_t num_bytes)
{
  mcd_return_et ret;

  uint32_t maxPayload;
  ret = mcd->mcd_qry_max_payload_size_f(core, &maxPayload);
  //assert(ret == MCD_RET_ACT_NONE);
  if (ret != MCD_RET_ACT_NONE)
    return ret;

  // //assert(maxPayload >= MCD_GUARANTEED_MIN_PAYLOAD); Not for mcdxdas.dll V4.0.5
  //assert(num_bytes <= 204800); // 200 KB -> few seconds for 400 kHz JTAG/DAP clock

  uint8_t *data = new uint8_t[num_bytes];
    
  mcd_tx_st tx;

  mcd_txlist_st  txlist;
  txlist.tx     = &tx;
  txlist.num_tx = 1;

  // Prepare transactions
  memset(&tx, 0, sizeof(tx));  // Set all to default values
  memcpy(&tx.addr, addr, sizeof(mcd_addr_st));
  tx.access_type = MCD_TX_AT_R;
  tx.data        = data;

  int numBytesRemaining = num_bytes;
  while (numBytesRemaining > 0) {
    if (numBytesRemaining > (int)maxPayload)
      tx.num_bytes = maxPayload;
    else
      tx.num_bytes = numBytesRemaining;

    ret = mcd->mcd_execute_txlist_f(core, &txlist);
    
    numBytesRemaining -= tx.num_bytes_ok;

    if (ret != MCD_RET_ACT_NONE)
      break;

    tx.data           += tx.num_bytes;
    tx.addr.address   += tx.num_bytes;
  }

  printf("Read %d byte\n", num_bytes - numBytesRemaining);
 
  delete data;  // No usage for the data in this simple demo

  return ret;
}


//-------------------------------------------------------------------------------------------------
// Print Help menu
void print_help(void)
{
	printf("\n\n");
	printf(    "Control core:         run, stop, step <steps>, trace\n");
	printf(    "Core registers:       r\n");

	printf(    "Reset (and Halt):     rst, rsthlt\n");
  

	printf(    "Dump memory:          dump <addr> <n_bytes>\n");
	printf(    "ASCII viewer:         ascii <addr> <n_bytes>\n");
	printf(    "Seek in memory:       seek <addr> <value> <size>\n");

	printf(    "Read 32 bit word:     read <addr>\n");
//	printf(    "Read N bytes:         read <addr> <n_bytes>\n");
	printf(    "Read 8 bit word:      r8 <addr>\n");


	printf(    "Write 32 bit word:    write <addr> <value>\n");
	printf(    "Write 8 bit word:     w8 <addr> <value>\n");
  
	printf(    "Set IP breakpoint:    bpt <addr> \n");
	printf(    "Clear IP breakpoint:  bpt\n");
  
	printf(    "Access HW frequency:  frequ <f_hz>\n");

	printf(    "Help:                 ? (or help)\n");  
	printf(    "Exit:                 q (or exit)\n");
	printf("\n");
}

//-------------------------------------------------------------------------------------------------
// Loop capturing Address Ax or Dx registers or any other 32-bit register file
void capture_CPU_regfile(mcd_core_st *core, uint32_t i_core, mcd_return_et ret, 
					   uint32_t uiAddress, uint32_t uiLen, uint32_t aRegFile[])
{
	mcd_tx_st	txDemo;
	uint32_t	value;
		
	// Loop on registers file
	txDemo.addr.address = uiAddress;
	for (uint32_t i=0; i<uiLen; i++) 
	{
		value = 0xEEEEEEEE;   
		ret = mcd->read32(core, &txDemo.addr, &value);
		mcdd_handle_err(stdout, &core, ret);
		aRegFile[i] =  value;
		txDemo.addr.address += 4;
	}
}

//-------------------------------------------------------------------------------------------------
// Loop and Print on Address Ax or Dx registers or any other 32-bit register file
void print_CPU_regfile(mcd_core_st *core, uint32_t i_core, mcd_return_et ret, 
					   uint32_t uiAddress, uint32_t uiLen, char chName)
{
	mcd_tx_st	txDemo;
	uint32_t	value;
		
	// Loop on registers file
	txDemo.addr.address = uiAddress;
	for (uint32_t i=0; i<uiLen; i++) 
	{
		if (i%4 == 0) printf("\n");
		value = 0xEEEEEEEE;   
		ret = mcd->read32(core, &txDemo.addr, &value);
		mcdd_handle_err(stdout, &core, ret);
		printf("%c%2.2d=0x%8.8X ", chName, i, value);
		txDemo.addr.address += 4;
	}
}

//-------------------------------------------------------------------------------------------------
// CPU Registers
void print_CPU_registers(mcd_core_st *core, uint32_t i_core, mcd_return_et ret)
{
	mcd_tx_st	txDemo;
	uint32_t	value;

	printf("\n");

	// Display CPU Core ID
	txDemo.addr.address = 0xF881FE1C + (i_core * 0x20000);
	value = 0xEEEEEEEE;  
	ret = mcd->read32(core, &txDemo.addr, &value);
	mcdd_handle_err(stdout, &core, ret);
	printf("CORE-ID=0x%1.1X  ", value);		
									
	// Display CPU Identification Register TC1.6P - not needed since is already dispayed at tool start
	//txDemo.addr.address = 0xF881FE18 + (i_core * 0x20000);
	//value = 0xEEEEEEEE;  
	//ret = mcd->read32(core, &txDemo.addr, &value);
	//mcdd_handle_err(stdout, &core, ret);
	//printf("CPU-ID=0x%8.8X ", value);

	// Display CPU Debug Status Register
	txDemo.addr.address = 0xF881FD00 + (i_core * 0x20000);
	value = 0xEEEEEEEE;  
	ret = mcd->read32(core, &txDemo.addr, &value);
	mcdd_handle_err(stdout, &core, ret);
	printf("DBGSR=0x%8.8X ", value);

	// TODO: DBGSR
	printf("\n");

	// Display CPU Interrupt Control Register
	txDemo.addr.address = 0xF881FE2C + (i_core * 0x20000);
	value = 0xEEEEEEEE;  
	ret = mcd->read32(core, &txDemo.addr, &value);
	mcdd_handle_err(stdout, &core, ret);
	printf("ICR=0x%8.8X ", value);

	// Display CPU Interrupt Stack Pointer
	txDemo.addr.address = 0xF881FE28 + (i_core * 0x20000);
	value = 0xEEEEEEEE;  
	ret = mcd->read32(core, &txDemo.addr, &value);
	mcdd_handle_err(stdout, &core, ret);
	printf("ISP=0x%8.8X ", value);

	// Display CPU Instruction Count
	txDemo.addr.address = 0xF881FC08 + (i_core * 0x20000);
	value = 0xEEEEEEEE;  
	ret = mcd->read32(core, &txDemo.addr, &value);
	mcdd_handle_err(stdout, &core, ret);
	printf("ICNT=0x%8.8X ", value);

	//// Display CPU Program Counter
	//txDemo.addr.address = 0xF881FE08 + (i_core * 0x20000);
	//value = 0xEEEEEEEE;  
	//ret = mcd->read32(core, &txDemo.addr, &value);
	//mcdd_handle_err(stdout, &core, ret);
	//printf("  PC=0x%8.8X ", value);
	//printf("\n");

	// Loop on Address registers Ax
	print_CPU_regfile(core, i_core, ret, 0xF881FF80 + (i_core * 0x20000), 16, 'A');

	// Loop on Data registers Dx
	print_CPU_regfile(core, i_core, ret, 0xF881FF00 + (i_core * 0x20000), 16, 'A');
	printf("\n");
}


//-------------------------------------------------------------------------------------------------
// Mnemonic Opcode Identification
extern void GetInstructionMnemonic(uint32_t uiInstruction);


//-------------------------------------------------------------------------------------------------
// Debug Trace
// Executes one instruction and displays the contents of all registers, the status of all flags, and the decoded form of the instruction executed.
void Debug_Trace(mcd_core_st *core, uint32_t i_core, mcd_return_et ret)
{
	mcd_tx_st	txDemo;
	uint32_t	uiIR_before;		// IR before code execution
	uint32_t	uiIR_after;			// IR after code execution
	uint32_t	uiInstruction;
	uint32_t	aRegA_before[16];
	uint32_t	aRegD_before[16];
	uint32_t	aRegA_after[16];
	uint32_t	aRegD_after[16];

	printf("\n");

	// Capture CPU Program Counter
	txDemo.addr.address = 0xF881FE08 + (i_core * 0x20000);
	uiIR_before = 0xEEEEEEEE;  
	ret = mcd->read32(core, &txDemo.addr, &uiIR_before);
	mcdd_handle_err(stdout, &core, ret);

	// Acquire the Instruction (as pointed by the PC)
	txDemo.addr.address = uiIR_before;
	uiInstruction = 0xEEEEEEEE;
	ret = mcd->read32(core, &txDemo.addr, &uiInstruction);
	mcdd_handle_err(stdout, &core, ret);

	// Capture Address registers Ax  - BEFORE INSTRUCTION EXECUTION
	capture_CPU_regfile(core, i_core, ret, 0xF881FF80 + (i_core * 0x20000), 16, aRegA_before);

	// Capture Data registers Dx  - BEFORE INSTRUCTION EXECUTION
	capture_CPU_regfile(core, i_core, ret, 0xF881FF00 + (i_core * 0x20000), 16, aRegD_before);

	// Execute one istruction and Display it
	ret =  mcd->mcd_step_f(core, FALSE, MCD_CORE_STEP_TYPE_INSTR, 1); 
	mcdd_handle_err(stdout, &core, ret);
	printf("PC@0x%8.8X INSTRUCTION 0x%8.8X ", uiIR_before, uiInstruction);

	// Mnemonic decoding
	GetInstructionMnemonic(uiInstruction);

	// show modifications
	printf("\n modify the following registers:");

	// Capture Address registers Ax  - AFTER INSTRUCTION EXECUTION
	capture_CPU_regfile(core, i_core, ret, 0xF881FF80 + (i_core * 0x20000), 16, aRegA_after);

	// Capture Data registers Dx  - AFTER INSTRUCTION EXECUTION
	capture_CPU_regfile(core, i_core, ret, 0xF881FF00 + (i_core * 0x20000), 16, aRegD_after);

	// Capture CPU Program Counter
	txDemo.addr.address = 0xF881FE08 + (i_core * 0x20000);
	uiIR_after = 0xEEEEEEEE;  
	ret = mcd->read32(core, &txDemo.addr, &uiIR_after);
	mcdd_handle_err(stdout, &core, ret);

	printf("\n");

	// Show the Modified Registers only
	bool bMod=false;
	for (int i=0; i<16; i++)
	{
		if (aRegA_before[i] != aRegA_after[i]) {
			printf("   A%2.2d=0x%8.8X => A%2.2d=0x%8.8X\n", i, aRegA_before[i], i, aRegA_after[i]);
			bMod = true;
		}
	}
	for (int i=0; i<16; i++)
	{
		if (aRegD_before[i] != aRegD_after[i]) {
			printf("   D%2.2d=0x%8.8X => D%2.2d=0x%8.8X\n", i, aRegD_before[i], i, aRegD_after[i]);
			bMod = true;
		}
	}
	// Check if IR has been modified (beside the standard +4 increment)
	if ( ((uiIR_before+4) != uiIR_after) && ((uiIR_before+2) != uiIR_after) ) {
			printf("   IP=0x%8.8X => IP=0x%8.8X\n", uiIR_before, uiIR_after);
			bMod = true;
		}
	if (!bMod)
		printf("   none\n");

	printf("\n");
}

//****************************************************************************
//                                 END OF FILE
//****************************************************************************
