// VM-USB test program
// WIENER, Plein & Baus Corp. 01/2014
// Revision 2.0 - 01/22/14 tested with VM-DBA
// Andreas Ruben, aruben@wiener-us.com

/* Simple program to demonstrate VM_USB in EASY mode
 * 
 * Copyright (C) 2005-2009 WIENER, Plein & Baus, Ltd (thoagland@wiener-us.com)
 *
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation, version 2.
 *
 *	Edited By Kolby Kiesling
 *	Added register read of Mesytec Modules... More to come
 *	Refer to MIDAS CCUSB for pointers...
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
#define base_addr 0xbb000000
#define soft_reset 0x6008

/* DEFINITIONS */
#define MTDC 0x06060000
#define MQDC 0x01060000

int main (int argc,  char *argv[])
{
  //long d_back=0, m_back=0;
  xxusb_device_type devices[100];
  //unsigned int VM_reg [20];
  struct usb_device *dev;
  usb_dev_handle *udev;       // Device Handle 
  //int Address_Modifier= 0x09;


//Find XX_USB devices and open the first one found
  xxusb_devices_find(devices);
  dev = devices[0].usbdev;
  udev = xxusb_device_open(dev); 
  
// Make sure VM_USB opened OK
  if(!udev) {
    printf ("\n\nFailed to Open VM_USB \n\n");
    return 0;
  } else {
    printf("\n\nDevice open\n\n"); 
  }
  vme VME;
  VME.vmUSBInit (udev);
  VME.moduleReset (udev, MQDC); 
  VME.moduleReset (udev, MTDC);
  VME.mvmeInit (udev, MQDC);
  VME.mvmeInit (udev, MTDC);
  VME.moduleInit (udev, MQDC);
  VME.moduleInit (udev, MTDC);
  VME.registerDump (udev);
  //VME.daqInit (udev);
  //VME.daqStart (udev);
  
  int status;
  
  long pData[2]={MQDC, MTDC};
  long r_data;
  //status = xxusb_bulk_write (udev, pData, sizeof(pData), 1000);
  //printf("\n\nStatus:\t\t%d\n\n\n", status);
  
  for (int i=0;i<64;++i) {
    //VME_read_16 (udev, 0x0D, MTDC|0x603E, &r_data);
    //printf("Data ready:\t\t%02x\n", r_data);
    VME_read_32 (udev, 0x0D, MTDC|0x6030, &r_data);
    printf("Data32 Polled:\t\t%02x\n", r_data);
    if (r_data > 0) {
    VME_read_32 (udev, 0x0D, MTDC, &r_data);
    printf("Data32 Read:\t\t%02x\n", r_data);
    //if (r_data == 0xFFFFFFFF) {
      printf ("--------------------\n");
      VME_write_32 (udev, 0x0E, MTDC|(0x0000603A), 0);
      VME_write_32 (udev, 0x0E, MTDC|(0x0000603C), 1);
      VME_write_32 (udev, 0x0E, MTDC|(0x00006034), 1);
      VME_write_32 (udev, 0x0E, MTDC|(0x0000603A), 1);
    //}
      
    usleep(400000);
    }
  }
  
  //char data[64];
  //memset (data, 0, sizeof(data));
  //status = xxusb_bulk_read (udev, &data, sizeof(data), 1000);
  //for (int i=0; i<sizeof(data);++i) {
    //printf("%02x | ",data[i]);
  //}
  //printf("\n\nStatus:\t\t%d\n", status);
  //  return errors;
// Close the Device
  xxusb_device_close(udev);
  printf("\n\n\n");
  
  return 0;
}

