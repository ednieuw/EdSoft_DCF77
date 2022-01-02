// 
#include "EdSoft_DCF77.h"
//--------------------------------------------
//  Initialyse 
//--------------------------------------------

EdSoft_DCF77::EdSoft_DCF77(uint8_t DCF_PIN,bool Inverted)
{
 SignalInverted = Inverted;
 DCFpin = DCF_PIN;
}
//--------------------------------------------
//  Exit
//--------------------------------------------
EdSoft_DCF77::~EdSoft_DCF77()
{

}

//------------------------------------------------------------------------------
// toggleDCFDebugInfo
//------------------------------------------------------------------------------
bool EdSoft_DCF77::toggleDCFDebugInfo(void)
 {
  PrintDebugInfo = 1-PrintDebugInfo; 
  return (PrintDebugInfo);	
 }
//------------------------------------------------------------------------------
// toggleDCFDebugInfo
//------------------------------------------------------------------------------
bool EdSoft_DCF77::getDCFDebugStatus(void)       { return(PrintDebugInfo); } 
uint32_t EdSoft_DCF77::getSignalFaults(void)     { return(SignalFaults);	} 
uint32_t EdSoft_DCF77::resetSignalFaults(void)   { return(SignalFaults=0);	} 
char*  EdSoft_DCF77::getDebugInfo(void)          { return(strlen(Infotext)>40?Infotext:0);}

bool EdSoft_DCF77::getDCFBitMinutemark(void)     { return(BitMinutemark);} 
bool EdSoft_DCF77::getDCFBitReserveantenna(void) { return(BitReserveantenna);} 
bool EdSoft_DCF77::getDCFBitOnehourchange(void)  { return(BitOnehourchange);} 
bool EdSoft_DCF77::getDCFBitSummertime(void)     { return(BitSummertime);} 
bool EdSoft_DCF77::getDCFBitWintertime(void)     { return(BitWintertime);} 
bool EdSoft_DCF77::getDCFBitLeapsecond(void)     { return(BitLeapsecond);} 
byte EdSoft_DCF77::getDCFCountOnehourchange(void){ return(COnehourchange);} 
byte EdSoft_DCF77::getDCFCountSummertime(void)   { return(CSummertime);} 
byte EdSoft_DCF77::getDCFCountWintertime(void)   { return(CWintertime);} 

//------------------------------------------------------------------------------
// DCFNoInt Make the time from the received DCF-bits
// Return time in seconds since 1-1-2000
//------------------------------------------------------------------------------
time_t EdSoft_DCF77::getTime(tmElements_t ActualTime)
{
        byte     TimeOK     , Receivebit     , Receivebitinfo;                                // Return flag is proper time is computed and received bit value
        bool     TimeSignaldoubtfull = false;                                                 // If a bit length signal is not between 5% and 30%
 static byte     Bitpos = 15,  Paritybit;                                                     //  
 static byte     MinOK = 2, HourOK = 2, YearOK = 2;                                           // 2 means not set. static because the content must be remembered
 static int      TimeMinutes     = 999;
 static int      LastTimeMinutes = 0;
 static byte     Lastyear        = ActualTime.Year;                                           // Remember the last valid year
 static int      TimeMinutesDiff, DateDaysDiff;
 static uint32_t DateDays ,LastDateDays;
 static uint32_t PreviousSumSecondSignal = 0;
  
 SumSignalCounts++; 
 SumSecondSignal += !digitalRead(DCFpin);                                                    // invert 0 to 1 and 1 to 0
                                                                                             // SignalBin[( (millis() - DCFmsTick)/50)] += r;
 if( (millis() - DCFmsTick) <996) return(0);                                                 // Compute every second the received DCF-bit to a time 
 while (!digitalRead(DCFpin))                                                                // Avoid splitting a positive signal and wait until the signal hets low
   {
    SumSignalCounts++;        
    SumSecondSignal++;
    if( (millis() - DCFmsTick) >1005) break;                                                 // A little retard of loop speed is fine 
   }
 DCFmsTick = millis();                                                                       // Set one second counter
 SumSignalCounts *= 1.10;                                                                    // Correction to correct pulses to an average of 100 and 200 msecs 
 if(Bitpos >= 60) {   Bitpos = 0;                                                            // Reset the second counter
    // if(PrintDebugInfo) 
       // {sprintf(Infotext,"EdT:%ld Ed:%ld Th:%ld EdW:%ld ThW:%ld Both:%ld Valid:%ld Min:%ld Signal:%d%% Lock:%s",    
          // Mem.EdMinTijd, Mem.EdMin, Mem.ThMin, Mem.EdWrong, Mem.ThWrong, Mem.EdThMin, Mem.ValidTimes, Mem.MinutesSinceStart,DCF_signal, DCFlocked?"Y":"N");
		  // Serial.println(Infotext); }                                        
   } 
 int msec  = (int)(1000 * SumSecondSignal / SumSignalCounts);                                 // This is roughly the milliseconds times 10 of the signal length
 switch(msec)
  {
   case   0 ...  10: if(SumSignalCounts > 500)// && (Bitpos > 1))                                // Check if there were enough signals and there were several positive signals 
                        { 
                         Receivebitinfo = 2; Receivebit = 0;
                         if(PreviousSumSecondSignal!=0)  Bitpos = 59; 
                        }                                                                     // If enough signalcounts and for one second no signal found. This is position zero
                      else { Receivebitinfo = 9; Receivebit = 0; }                            // If not it is a bad signal probably a zero bit
                                                                         break;               // If signal is less than 0.05 sec long than it is the start of minute signal
   case  11 ...  50: Receivebitinfo = 8; Receivebit = 0;                 break;               // In all other cases it is an error
   case  51 ... 160: Receivebitinfo = 0; Receivebit = 0;                 break;               // If signal is 0.1 sec long than it is a 0 
   case 161 ... 320: Receivebitinfo = 1; Receivebit = 1;                 break;               // If signal is 0.2 sec long than it is a 1 
   default:          Receivebitinfo = 9; Receivebit = 1;                 break;               // In all other cases it is an error probably a one bit
  } 
 PreviousSumSecondSignal = SumSecondSignal;
 if(Receivebitinfo > 2)  SignalFaults++;                                                      // Count the number of faults per hour
 TimeOK = 0;                                                                                  // Set Time return code to false  
//                                                                                            //
 switch (Bitpos)                                                                              // Decode the 59 bits to a time and date.
  {                                                                                           // It is not binary but "Binary-coded decimal". Blocks are checked with an even parity bit. 
   case   0: BitMinutemark = Receivebit;                                                      // Suppress compiler warning   
             MinOK = HourOK  = YearOK = 2;                               break;               // Gives the dot (.) at MHY or mhy   
   case   1: TimeSignaldoubtfull = 0;                                                         // Start a fresh minute          
             D.Minute = D.Hour = D.Day = D.Wday = D.Month = D.Year = 0;                       // Reset the variables
   case 2 ... 14:                                                        break;               // reserved for wheather info we will not use
   case  15: BitReserveantenna = Receivebit;
             BitReserveantenna = BitReserveantenna;                      break;                // 1 if reserve antenna is in use
   case  16: BitOnehourchange  = Receivebit;
             if(BitOnehourchange) COnehourchange++;
             else                 COnehourchange--;
             COnehourchange = max(COnehourchange,60);
//             if(COnehourchange >25)
//               {
//                 DCFlocked = false; 
//                 Serial.println("DCFLock unlocked One hour change");                // Unlock if one hour change comes up 
//              }                                                       
                                                                         break;
   case  17: BitSummertime = Receivebit;
             if(BitSummertime) CSummertime++; 
             else           CSummertime--;
             CSummertime = max(CSummertime, 60);                
                                                                         break;                // 1 if summer time 
   case  18: BitWintertime = Receivebit;
             if(BitWintertime) CWintertime++;
             else           CWintertime--;
             CWintertime = max(CWintertime, 60);
                                                                         break;                // 1 if winter time
   case  19: BitLeapsecond = Receivebit; BitLeapsecond = BitLeapsecond;  break;                // Announcement of leap second in time setting is coming up
   case  20: StartOfEncodedTime=Receivebit; Paritybit = 0;               break;                // Bit must always be 1 
   case  21: if(Receivebit) {D.Minute  = 1;  Paritybit = 1 - Paritybit;} break;
   case  22: if(Receivebit) {D.Minute += 2 ; Paritybit = 1 - Paritybit;} break;
   case  23: if(Receivebit) {D.Minute += 4 ; Paritybit = 1 - Paritybit;} break;
   case  24: if(Receivebit) {D.Minute += 8 ; Paritybit = 1 - Paritybit;} break;
   case  25: if(Receivebit) {D.Minute += 10; Paritybit = 1 - Paritybit;} break;
   case  26: if(Receivebit) {D.Minute += 20; Paritybit = 1 - Paritybit;} break;
   case  27: if(Receivebit) {D.Minute += 40; Paritybit = 1 - Paritybit;} break;
   case  28: MinOK = (Receivebit==Paritybit);  Paritybit = 0;           
             if(D.Minute>59) MinOK=0;                                    
			 break;                // The minute parity is OK or NOK    
   case  29: if(Receivebit) {D.Hour   =  1; Paritybit = 1 - Paritybit;}  break;
   case  30: if(Receivebit) {D.Hour  +=  2; Paritybit = 1 - Paritybit;}  break;
   case  31: if(Receivebit) {D.Hour  +=  4; Paritybit = 1 - Paritybit;}  break;
   case  32: if(Receivebit) {D.Hour  +=  8; Paritybit = 1 - Paritybit;}  break;
   case  33: if(Receivebit) {D.Hour  += 10; Paritybit = 1 - Paritybit;}  break;
   case  34: if(Receivebit) {D.Hour  += 20; Paritybit = 1 - Paritybit;}  break;
   case  35: HourOK = (Receivebit==Paritybit); Paritybit = 0;           
             if(D.Hour>23) HourOK=0;                                     
			 break;                // The hour parity is OK or NOK 
   case  36: if(Receivebit) {D.Day    =  1; Paritybit = 1 - Paritybit;}  break;
   case  37: if(Receivebit) {D.Day   +=  2; Paritybit = 1 - Paritybit;}  break;
   case  38: if(Receivebit) {D.Day   +=  4; Paritybit = 1 - Paritybit;}  break;
   case  39: if(Receivebit) {D.Day   +=  8; Paritybit = 1 - Paritybit;}  break;
   case  40: if(Receivebit) {D.Day   += 10; Paritybit = 1 - Paritybit;}  break;
   case  41: if(Receivebit) {D.Day   += 20; Paritybit = 1 - Paritybit;}  break;
   case  42: if(Receivebit) {D.Wday   =  1; Paritybit = 1 - Paritybit;}  break;
   case  43: if(Receivebit) {D.Wday  +=  2; Paritybit = 1 - Paritybit;}  break;
   case  44: if(Receivebit) {D.Wday  +=  4; Paritybit = 1 - Paritybit;}  break;
   case  45: if(Receivebit) {D.Month  =  1; Paritybit = 1 - Paritybit;}  break;
   case  46: if(Receivebit) {D.Month +=  2; Paritybit = 1 - Paritybit;}  break;
   case  47: if(Receivebit) {D.Month +=  4; Paritybit = 1 - Paritybit;}  break;
   case  48: if(Receivebit) {D.Month +=  8; Paritybit = 1 - Paritybit;}  break;
   case  49: if(Receivebit) {D.Month += 10; Paritybit = 1 - Paritybit;}  break;
   case  50: if(Receivebit) {D.Year   =  1; Paritybit = 1 - Paritybit;}  break;
   case  51: if(Receivebit) {D.Year  +=  2; Paritybit = 1 - Paritybit;}  break;
   case  52: if(Receivebit) {D.Year  +=  4; Paritybit = 1 - Paritybit;}  break;
   case  53: if(Receivebit) {D.Year  +=  8; Paritybit = 1 - Paritybit;}  break;
   case  54: if(Receivebit) {D.Year  += 10; Paritybit = 1 - Paritybit;}  break;
   case  55: if(Receivebit) {D.Year  += 20; Paritybit = 1 - Paritybit;}  break;
   case  56: if(Receivebit) {D.Year  += 40; Paritybit = 1 - Paritybit;}  break;
   case  57: if(Receivebit) {D.Year  += 80; Paritybit = 1 - Paritybit;}  break;
   case  58: YearOK = (Receivebit==Paritybit);                                                // The year month day parity is OK or NOK   
             Paritybit = 0;
             if(D.Day>31 || D.Month==0 || D.Month>12 || Lastyear!=D.Year) YearOK=0; 
//             Mem.MinutesSinceStart++;                                                           // 
                                                                         break;              
   case  59:                                                            // 
            if(MinOK && HourOK) TimeMinutes = D.Hour*60 + D.Minute;
            else TimeMinutes++;                                                               // 
            if(D.Month==0) break;                                                             // Definitely a wrong date 
            if(YearOK) DateDays = ((D.Year*12 + D.Month)*31) + D.Day;                         //
            TimeMinutesDiff = abs(TimeMinutes - LastTimeMinutes);                             //
            DateDaysDiff    = (uint32_t) abs(DateDays - LastDateDays);                        //
            if(PrintDebugInfo)
             { 
               sprintf(Infotext,"%d minute%s difference between Now:%d and Last:%d minutes",
                    TimeMinutesDiff, TimeMinutesDiff==1?"":"s", TimeMinutes, LastTimeMinutes);            
               Serial.println(Infotext);
              }
            LastTimeMinutes = TimeMinutes;                                                    //
            LastDateDays    = DateDays;  
			
            if(MinOK && HourOK &&  YearOK && TimeMinutesDiff<2 && DateDaysDiff<2) TimeOK = 1; // All OK          
            if(MinOK && HourOK && !YearOK && TimeMinutesDiff<2)                   TimeOK = 2; // If a date difference time remains usable
            if(TimeOK) LastTimeMinutes = TimeMinutes; 
            if(TimeOK == 1) Lastyear = D.Year;   
            if(TimeSignaldoubtfull)      {}//  {Dday = 0; TimeOK = 5;}                        // If a signal lenght was too short or long this last minute time is not OK
                                                                        break;
    default:  Serial.println(F("Default in BitPos loop. Impossible"));  break;
   }   // End switch(bitpos)
   if(TimeOK==2)
     {
      D.Day = ActualTime.Day; D.Month = ActualTime.Month; D.Year = ActualTime.Year;  D.Wday = ActualTime.Wday;
 //     Mem.EdMinTijd++;   
      TimeOK = 1;
     }
  if((Bitpos>19 && Bitpos<36) && Receivebitinfo>2) TimeSignaldoubtfull = true;                // If a date time bit is received and the received bit is not 0 or 1 then time becomes doubtfull
  if(PrintDebugInfo)
    {
     sprintf(Infotext,"@%s%s%s%6ld %5ld %02ld%% [%d] %02d:%02d|%02d| %02d-%02d-%04d/%01d F%ld OK:%d",
             (MinOK<2?(MinOK?"M":"m"):"."),(HourOK<2?(HourOK?"H":"h"):"."),(YearOK<2?(YearOK?"Y":"y"):"."),
             SumSecondSignal, SumSignalCounts, 100*SumSecondSignal/SumSignalCounts, Receivebitinfo, 
             D.Hour, D.Minute, Bitpos, D.Day, D.Month, D.Year+2000, D.Wday, SignalFaults, TimeOK);  
	//		 Serial.println(Infotext);
     //if(Bitpos==10) {   
     //for(int i=0 ;i<10;i++){sprintf(Infotext,"%4lu ", SignalBin[i]);  Serial.print(Infotext); SignalBin[i]=0;}
     //for(int i=10;i<20;i++){sprintf(Infotext,"%4lu ", SignalBin[i]);  Serial.print(Infotext); SignalBin[i]=0;}   Serial.println("");}
    }
 SumSecondSignal = SumSignalCounts = 0;
 D.Second = Bitpos++;
 return(TimeOK == 1?makeTime(D):0);                                                           // If D = DCF time struct tmElements_t
}