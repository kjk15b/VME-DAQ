/*
 * This program describes the standard routines for data acquisition with the 
 * VM USB as a master unit. Comments will be scattered throughout describing
 * Functions and their purpose. Please refer to the Wiener VM USB manual for 
 * more assistance.
 * 
 * Kolby Kiesling
 * kjk15b@acu.edu
 * 07 / 15 / 2019
 * 
 */

#include <libxxusb.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <cstdlib>
#include <string.h>
#include <cstdint>
#include "vmeClass.h"
#include <unistd.h>

// Plenty of #define becuase of all of the constants in setting up the framework
#define GMODE 0x04
#define GLOBAL_SETTINGS 0x0099
#define DAQMODE 0x08
#define DAQ_SETTINGS 0x000000EF
#define SCALAR_A 0x1C
#define SCALAR_B 0x20
#define AR_IRQ 0xFF00 // Action registers access
#define AR_DAQ_START 0x0001
#define AR_DAQ_STOP 0x0000

#define ADDR_R 0x0D // supervisory read, I do not know if this changes anything
#define ADDR_W 0x0E // limited read command, probably good to keep like this right now
#define MBLT_ADDR_R 0x0F // Supervisory BLT read, for stack execution

/*
 * I am lazy a lot of stuff is redundant on the VME bus, so I am going to make a lot of
 * definitions to call later in the program, please refer back to here for later aid.
 * All addresses are pulled from "Interfacing Mesytec VME Modules" Page 11.
 * This is an early design, I am copying the setup scripts provided from MVME, 
 * refer to the init-##-Module Init.vmescripts for help.
 * 
 * You will notice my syntax is close to what is in the user manuals, I tried to keep things
 * looking similar to help out...
 * 
 * Some registers are never interfaced with.
 * I should note that these are mainly for interfacing with the Mesytec VME devices, below I will append
 * stuff for the VM-USB specifically, this should hopefully help out with debugging in the future.
 */
// These commands are for reading and writing to the VME bus, need certain priviledges to perform specific tasks
#define ADDR_R 0x0D // supervisory read, I do not know if this changes anything
#define ADDR_W 0x0E // limited read command, probably good to keep like this right now
#define MBLT_ADDR_R 0x0F // Supervisory BLT read, for stack execution
// Mesytec Addresses
#define soft_reset 0x6008
#define firmware_revision 0x600E
#define marking_type 0x6038 // event counter, timestamp, extended timestamp
#define irq_level 0x6010
#define irq_vector 0x6012 // probably should not change from 0
#define IRQ_source 0x601C // 0 for event threshold and 1 for data threshold
#define irq_event_threshold 0x601E // should always be 1
#define multi_event 0x6036 // 0->single event, 0x3-> multi event, 0xb->transmits # events specified
#define Max_transfer_data 0x601A // one event is transmitted, should always be 1
#define cblt_mcst_control 0x6020 // Setup Multicast, DO NOT TOUCH
#define cblt_address 0x6024 // set 8 high bits of Multicast address, DO NOT TOUCH
#define output_format 0x6044 // 0->standard, 1->timestamp
#define bank_operation 0x6040 // 0->bank connected, 1->independent
#define tdc_resolution 0x6042 // timing resolution, refer to user manuals
#define first_hit 0x6052 // 0bRT-> R: bank0, T: bank1, 1->transmit first hit, 0->transmit all hits
#define bank0_win_start 0x6050 // leave at 16368 unless you know what you are doing
#define bank1_win_start 0x6052 // see above
#define bank0_win_width 0x6054 // ns of bin width (32 normally)
#define bank1_win_width 0x6056
#define bank0_trig_source 0x6058 // 0x001 leaves in split bank mode
#define bank1_trig_source 0x605A
#define Negative_edge 0x6060 // DO NOT CHANGE
#define bank0_input_thr 0x6078 // leave at 105 unless you know what you are doing
#define bank1_input_thr 0x607A
#define ECL_term 0x6062 // to turn terminators on (APPLT TO MQDC ONLY)
#define ECL_trig1_osc 0x6064
#define Trig_select 0x6068
#define NIM_trig1_osc 0x606A
#define NIM_busy 0x606E
#define pulser_status 0x6070
#define ts_sources 0x6096 // VME source for frequency (DO NOT CHANGE)
#define ts_divisor 0x6098
#define stop_ctr 0x60AE
#define ECL_gate1_osc 0x6064 // MQDC32 stuff now plenty of overlap in registers
#define ECL_fc_res 0x6066 // a lot of this stuff just straight up should not be touched
#define Gate_select 0x6068 // unless of course you know what you are doing
#define NIM_gat1_osc 0x606A
#define NIM_fc_reset 0x606C
#define pulser_dac 0x6072
#define chn0 0x4000 // channels used for setting thresholds on the MQDC32
#define chn1 0x4002 // Again I am lazy so I made them all definitions, this is so people
#define chn2 0x4004 // can actually come back and half-read what I wrote instead of seeing
#define chn3 0x4006 // a lot of cryptic hexadecimal addresses
#define chn4 0x4008
#define chn5 0x400A
#define chn6 0x400C
#define chn7 0x400E
#define chn8 0x4010
#define chn9 0x4012
#define chn10 0x4014
#define chn11 0x4016
#define chn12 0x4018
#define chn13 0x401A
#define chn14 0x401C
#define chn15 0x401E
#define chn16 0x4020
#define chn17 0x4022
#define chn18 0x4024
#define chn19 0x4026
#define chn20 0x4028
#define chn21 0x402A
#define chn22 0x402C
#define chn23 0x402E
#define chn24 0x4030
#define chn25 0x4032
#define chn26 0x4034
#define chn27 0x4036
#define chn28 0x4038
#define chn29 0x403A
#define chn30 0x403C
#define chn31 0x403E
#define ignore_thresholds 0x604C
#define offset_bank0 0x6044
#define offset_bank1 0x6046
#define limit_bank0 0x6050
#define limit_bank1 0x6052
#define trig_delay0 0x6054
#define trig_delay1 0x6056
#define input_coupling 0x6060
#define skip_oorange 0x604A
#define start_acq 0x603A // I am still copying the syntax of what they are declared as in the manual for reference.
#define reset_ctr_ab 0x6090
#define FIFO_reset 0x603C
#define readout_reset 0x6034
#define base_addr 0xbb000000
#define module_id 0x6004
#define data_reg 0x6030
#define fifo_threshold 0x6018
#define MTDC 0x06060000
#define MQDC 0x01060000


/*
 * vme::vmUSBInit
 * This function starts the initialization process for the VM USB, e.g.
 * setting up the internal file registers for data acquisition.
 */

int
vme::vmUSBInit (usb_dev_handle* udev) {
  long d_back=0, delay=200;
  VME_DGG (udev, 0, 3, 0, delay, 2, 0, 1);
  VME_DGG (udev, 1, 3, 0, delay, 2, 0, 1);
  VME_LED_settings (udev, 0, 0, 0, 1);
  VME_LED_settings (udev, 1, 0, 0, 1);
  VME_LED_settings (udev, 2, 1, 0, 1);
  VME_LED_settings (udev, 3, 3, 0, 1);
  VME_Output_settings (udev, 1, 0, 0, 1);
  VME_Output_settings (udev, 2, 1, 0, 1);
  VME_register_write (udev, GMODE, GLOBAL_SETTINGS);
  VME_register_write (udev, DAQMODE, DAQ_SETTINGS);
  
  printf("--------------------\nRegister Dump\n--------------------\nWriting and reading from registers\n");
  for(short i = 0x00; i <= 0x40; i+=0x04)
  {
	VME_register_read(udev, i, &d_back);
	printf("Value read at register 0x%0x:\t 0x%0x \n", i, d_back);
  }
  return 0;
}


/*
 * vme::moduleReset
 * This function performs a soft power cycle and prints the firmware and hardware ID of the modules on the VME
 * bus.
 */

int
vme::moduleReset (usb_dev_handle* udev, long module_addr) {
  static long w_data=1, r_data=0;
  module_addr = module_addr | soft_reset;
  printf("\n--------------------\nSoft Reset Starting\n--------------------\n");
  VME_write_16 (udev, ADDR_W, module_addr|soft_reset, w_data);
  usleep(400);
  VME_read_16 (udev, ADDR_R, module_addr, &r_data);
  printf("Module ID:\t\t0x%0x\n", r_data);
  module_addr = (module_addr ^ soft_reset) | firmware_revision;
  VME_read_16 (udev, ADDR_R, module_addr, &r_data);
  printf("Firmware Revision:\t0x%0x\n", r_data);
  printf("\n--------------------\nSoft Reset Finished\n--------------------\n");
  return 0;
}


/*
 * vme::mvmeInit
 * This function initializes the Mesytec modules for interfacing with the VME bus. See MVME for more help.
 * 
 */

int
vme::mvmeInit (usb_dev_handle* udev, long module_addr) {
  static long reg[9]={irq_level, irq_vector, IRQ_source, irq_event_threshold, marking_type, multi_event};
  static long reg_data[9]={0, 0, 0, 1, 0x1, 0x1000};
  printf("\n--------------------\nStarting VME Interfacing\n--------------------\n");
  for (int i=0;i<9;++i) {
     module_addr |= reg[i];
     VME_write_16 (udev, ADDR_W, module_addr, reg_data[i]);
     module_addr ^= reg[i];
     printf(".\t");
     usleep(200);
  }
  printf("\n--------------------\nVME Interfacing Finished\n--------------------\n");
  return 0;
}


/*
 * vme::moduleInit 
 * This function initializes the Mesytec modules for data acquisition, see MVME for more help.
 * 
 */

int
vme::moduleInit (usb_dev_handle* udev, long module_addr) {
  static long mqdc_reg[53]={ECL_term, ECL_gate1_osc, ECL_fc_res, Gate_select, NIM_gat1_osc, NIM_fc_reset, NIM_busy, pulser_status, pulser_dac, ts_sources, ts_divisor, chn0, chn1, chn2, chn3, chn4, chn5, chn6, chn7, chn8, chn9, chn10, chn11, chn12, chn13, chn14, chn15, chn16, chn17, chn18, chn19, chn20, chn21, chn22, chn23, chn24, chn25, chn26, chn27, chn28, chn29, chn30, chn31, ignore_thresholds, bank_operation, offset_bank0, offset_bank1, limit_bank0, limit_bank1, trig_delay0, trig_delay1, input_coupling, skip_oorange};
  static long mqdc_data[53]={0b11000, 0, 1, 0, 0, 0, 0, 5 /*5PULSER*/, 32, 0b00, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 130, 130, 255, 255, 0, 0, 0b000, 0};

  static long mtdc_reg[22]={output_format, tdc_resolution, first_hit, bank0_win_start, bank1_win_start, bank0_win_width, bank1_win_width, bank0_trig_source, bank1_trig_source, Negative_edge, bank0_input_thr, bank1_input_thr, ECL_term, ECL_trig1_osc, Trig_select, NIM_trig1_osc, NIM_busy, pulser_status, ts_sources, ts_divisor, stop_ctr};
  static long mtdc_data[22]={0, 0, 3, 0b11, 16368, 16368, 32, 32, 0x001, 0x002, 0b00, 105, 105, 0b000, 0, 0, 0, 0, 3 /*3PULSER*/, 0b00, 1, 0b00};

  static long r_data=0;
  module_addr |= firmware_revision;
  VME_read_16 (udev, ADDR_R, module_addr, &r_data);
  module_addr ^= firmware_revision;
  
  printf("\n--------------------\nStarting initialization process\n--------------------\n");
  
  if (r_data == 0x204) {
    for (int i=0;i<53;++i) {
      module_addr |= mqdc_reg[i];
      VME_write_16 (udev, ADDR_W, module_addr, mqdc_data[i]);
      usleep(200);
      module_addr ^= mqdc_reg[i];
      printf(".\t");
    }
  }
  
  if (r_data == 0x206) {
    for (int i=0;i<22;++i) {
      module_addr |= mtdc_reg[i];
      VME_write_16 (udev, ADDR_W, module_addr, mtdc_data[i]);
      usleep(200);
      module_addr ^= mtdc_reg[i];
      printf(".\t");
    }
  }
  
  printf("\n--------------------\nFinished initialization process\n--------------------\n");
  return 0;
}


/*
 * vme::registerDump
 * This function is a debugging tool to dump the internal registers of the VM USB.
 * 
 */

int 
vme::registerDump (usb_dev_handle* udev) {
  long d_back;
  printf("--------------------\nRegister Dump\n--------------------\n");
  for(short i = 0x00; i <= 0x40; i+=0x04)
  {
	VME_register_read(udev, i, &d_back);
	printf("Value read at register 0x%0x:\t 0x%0x \n", i, d_back);
  }
  return 0;
}


/*
 * vme::daqStart
 * 
 * 
 */

int
vme::daqStart (usb_dev_handle* udev) {
  printf("\n--------------------\nStarting Data Acquisition\n--------------------\n");
  VME_register_write (udev, 0x01, AR_IRQ|AR_DAQ_STOP);
  long d_back;
  VME_register_write (udev, 0x01, AR_IRQ|AR_DAQ_START);
  VME_read_16 (udev, ADDR_R, MQDC | firmware_revision, &d_back);
  printf("Data:\t0x%0x\n", d_back);
  VME_read_16 (udev, ADDR_R, MQDC | 0x6030, &d_back);
  printf("Data:\t0x%0x\n", d_back);
  return 0;
}


/*
 * vme::daqInit
 * This function initializes the VM USB and Mesytec modules for data acquisition. Uses MBLT, refer to MVME and
 * Mesytec modules for setup.
 */

int
vme::daqInit (usb_dev_handle* udev) { 
  static long s_data[5]={0, 3, 1, 1, 1};
  static long reg_addr[5]={start_acq, reset_ctr_ab, FIFO_reset, start_acq, readout_reset};
  long addr = base_addr;
  printf("\n--------------------\nInitializing Data Acquisition\n--------------------\n");
  for (int i=0;i<5;++i) {
    addr |= reg_addr[i];
    VME_write_16 (udev, ADDR_W, addr, s_data[i]);
    addr ^= reg_addr[i];
    usleep(400);
    printf(".\t");
  }
  return 0;
}

int
vme::daqStop () {
  return 0;
}