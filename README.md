# EdSoft_DCF77 

#  Functions
  time_t   getTime(tmElements_t I);    // Main function. Returns time every minute of 0<br>
  bool     toggleDCFDebugInfo(void);   // If ON statistics is printed in the Serial monitor<br>
  bool     getDCFDebugStatus(void);    // get the staus ON or OFF for Debug Info<br>
  uint32_t resetSignalFaults(void);    // Signal faults are reset to zero. NB do this every hour.<br>
  uint32_t getSignalFaults(void);      // Signal faults are counted until reset to zero or when reaching 0xFFFF faults<br>
<br>
  bool getDCFBitMinutemark(void);      // If true the Minutemark bit is set<br>
  bool getDCFBitReserveantenna(void);  // If true the Reserveantennabit is set<br>
  bool getDCFBitOnehourchange(void);   // If true the Onehourchange bit is set<br>
  bool getDCFBitSummertime(void);      // If true the Summertime bit is set <br>
  bool getDCFBitWintertime(void);      // If true the Wintertime bit is set<br>
  bool getDCFBitLeapsecond(void);      // If true the Leapsecond bit is setv
<br>
  byte getDCFCountOnehourchange(void);  // Get the number of bits set ranging between 0 and 60<br>
  byte getDCFCountSummertime(void);     // If the value is greater than say 30 one can assume the count is reliable <br>
  byte getDCFCountWintertime(void);     // When reception is bad a bit can be set erroniously.<br>
                                        // When the bit is false the counter decreases to zero  <br>
  char*    getDebugInfo(void);          // Returns the last time signal information string or zero. (Every second one is made) v
