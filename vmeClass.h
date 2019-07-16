/*
 * This header is for building the class to run the DAQ
 * through libxxusb.
 * 
 * Kolby Kiesling
 * kjk15b@acu.edu
 * 07 / 15 / 2019
 * 
 */
#ifndef vmeClass_H
#define vmeClass_H

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <cstdlib>
#include <string.h>
#include <cstdint>
//#include <libxxusb.h>

class vme 
{

public:
    
  virtual int vmUSBInit (usb_dev_handle* udev);
  
  virtual int moduleReset (usb_dev_handle* udev, long module_addr);
  
  virtual int mvmeInit (usb_dev_handle* udev, long module_addr);
  
  virtual int moduleInit (usb_dev_handle* udev, long module_addr);
  
  virtual int registerDump (usb_dev_handle* udev);
  
  virtual int daqStart (usb_dev_handle* udev);
  
  virtual int daqInit (usb_dev_handle* udev);
  
  virtual int daqStop ();
  
};

#endif