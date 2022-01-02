#ifndef EdSoft_DCF77_H_
#define EdSoft_DCF77_H_

#include <Arduino.h>
#include <Time.h>

class EdSoft_DCF77 
{ 
 public: 
  EdSoft_DCF77(uint8_t DCF_PIN,bool Inverted);
  ~EdSoft_DCF77();

  time_t   getTime(tmElements_t I);    // Main function. Returns time every minute of 0
  bool     toggleDCFDebugInfo(void);   // If ON statistics is printed in the Serial monitor
  bool     getDCFDebugStatus(void);    // get the staus ON or OFF for Debug Info
  uint32_t resetSignalFaults(void);    // Signal faults are reset to zero. NB do this every hour.
  uint32_t getSignalFaults(void);      // Signal faults are counted until reset to zero or when reaching 0xFFFF faults

  bool getDCFBitMinutemark(void);      // If true the Minutemark bit is set
  bool getDCFBitReserveantenna(void);  // If true the Reserveantennabit is set
  bool getDCFBitOnehourchange(void);   // If true the Onehourchange bit is set
  bool getDCFBitSummertime(void);      // If true the Summertime bit is set 
  bool getDCFBitWintertime(void);      // If true the Wintertime bit is set
  bool getDCFBitLeapsecond(void);      // If true the Leapsecond bit is set

  byte getDCFCountOnehourchange(void);  // Get the number of bits set ranging between 0 and 60
  byte getDCFCountSummertime(void);     // If the value is greater than say 30 one can assume the count is reliable 
  byte getDCFCountWintertime(void);     // When reception is bad a bit can be set erroniously.
                                        // When the bit is false the counter decreases to zero  
  char*    getDebugInfo(void);          // Returns the last time signal information string or zero. (Every second one is made) 
 	
	
private:

  bool    SignalInverted;
  uint8_t DCFpin;

  char     Infotext[100];		
  bool     PrintDebugInfo;
  bool     BitMinutemark, BitReserveantenna, BitOnehourchange; 
  bool     BitSummertime, BitWintertime    , BitLeapsecond;
//  bool     DCFtimeCanBeLocked = false;
  byte     StartOfEncodedTime = 0;                                  // Should be always one. Start if time signal
  int      CSummertime        = 0;                                  // Counter. If >30 times CSummertime is received Summertime = true
  int      CWintertime        = 0;                                  // Counter. If >30 times CWintertime is received Wintertime = true
  int      COnehourchange     = 0;                                  // Counter. If >10 times COnehourchange is received Onehourchange = true
  uint32_t DCFmsTick          = 0;                                  // the number of millisecond ticks since we last incremented the second counter
  uint32_t SumSecondSignal    = 0;                                  // sum of digital signals ( 0 or 1) in 1 second
  uint32_t SumSignalCounts    = 0;                                  // Noof of counted signals
  uint32_t SignalFaults       = 0;                                  // Counter for SignalFaults per hour  
  time_t   TimeDCFNoIntSecs   = 0;                                  // Time since 1-1-2000 from DCF
  time_t   LastDCFNoIntSecs   = 0;                                  // Time since 1-1-2000 from DCF
//uint32_t SignalBin[20];                                           // For debugging to see in which time interval reception takes place
  tmElements_t D;                                                   // The time struct for times used in DCFNoInt getTime() function	 

};

#endif /* EdSoft_DCF77_H_ */
