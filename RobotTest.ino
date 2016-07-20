#include "SourceFiles\VirtualWire.h"
#include "SourceFiles\RobotCmd.h"
#define DEBUG 1

RobotCmd robotcmd;

String testPassed ="...passed";
String testFailed = "...failed";
String underscore="**************************************";
bool finishedTests = false;


// *******************
//     Radio Functions
// *******************
void radioReceiveSetup()
{
  vw_set_rx_pin(11);
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_setup(2000);   // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
}

void radioTransmitSetup()
{
    vw_set_tx_pin(12);
    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);   // Bits per sec
}

void radioTransmit(const char* msg){
  if(strlen(msg)>1){
      if(DEBUG){
        Serial.print("transmitting : ");
        Serial.println(msg);
      }
   
   vw_send((uint8_t *)msg, strlen(msg));
   vw_wait_tx(); // Wait until the whole message is gone    
  }
}

String radioReceive()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  String message ="";
  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    for (int i = 0; i < buflen; i++)
    {
       message += char(buf[i]);
    }
  }

  if(DEBUG){
   Serial.print("receiving : ");
   Serial.println(message);
  }
  return message="";
}


  
 
void setup() {
	Serial.begin(9600);
	radioReceiveSetup();
	radioTransmitSetup();
	robotcmd.setup(9); //buzzerPin
  }
  
  
  void loop()
  {
    int note,motorState;
    int Danced=0;
    int Sang=0;
   String outgoingMessage="";
   String newDance ="";
   String newSong="";
   String entireMessage = "";
  long currTime = millis();
  bool passed;
  int elapsedTime =0;
  if(!finishedTests)
  {
    
    robotcmd.currState = RBT_IDLE;
    robotcmd.commandTimeout = 1000; //set it so we don't wait so long
    delay(robotcmd.commandTimeout+100);
    currTime=millis();
    /*
	//Test 1
	Serial.print(F(" Test 1 - Testing if MARCO sent when robot is idle"));
	outgoingMessage = robotcmd.update(currTime,"");
  passed = outgoingMessage[0] == RobotCmd::MARCO 
        && robotcmd.currState == RBT_WAIT_POLO;
  Serial.println(passed?testPassed:testFailed);
	

	//Test 2
  Serial.print(F(" Test 2 - Testing if Dance Received after Receiving Polo"));
	newDance = robotcmd.update(currTime,String(RobotCmd::POLO));
  passed = newDance[0] == RobotCmd::SONG
        && robotcmd.currState == RBT_DANCE;
  Serial.println(passed?testPassed:testFailed);

  
	//Test 3
 Serial.print(F(" Test 3 - Testing if Song & Dance performed"));
	robotcmd.currState = RBT_WAIT_DANCE;
	outgoingMessage = robotcmd.update(currTime,newDance.c_str());
  passed = outgoingMessage == ""
        && robotcmd.currState == RBT_DANCE;
  Danced = 0;
  Sang = 0;
  while(robotcmd.currState != RBT_IDLE)
  {
    note = robotcmd.singer.getCurrSongIx();
    motorState = robotcmd.dancer.motor.motorState;
		delay(100);
    robotcmd.update(millis(),"");
    Sang += note != robotcmd.singer.getCurrSongIx()?1:0;
    Danced += motorState != robotcmd.dancer.motor.motorState?1:0;
  }
  passed = passed 
        && Danced >= robotcmd.dancer.danceLen
        && Sang >= robotcmd.singer.songLen
        && robotcmd.currState == RBT_IDLE;
  Serial.println(passed?testPassed:testFailed);

  //Test 4
  Serial.print(F(" Test 4 - Testing MARCO sent after time out period expires"));
  robotcmd.currState = RBT_IDLE;
  robotcmd.lastCommandSent = millis();
  robotcmd.freeTime = millis(); //how long we must wait before the robot will send Marco;
  robotcmd.commandTimeout = 5000;
  outgoingMessage ="";
  elapsedTime = robotcmd.lastCommandSent;
  while(robotcmd.currState == RBT_IDLE
    && outgoingMessage[0] != RobotCmd::MARCO)
  {
    delay(100);
    outgoingMessage = robotcmd.update(millis(),"");
  }
  elapsedTime = millis() - elapsedTime;
  passed = elapsedTime> robotcmd.commandTimeout;
  Serial.println(passed?testPassed:testFailed);


  //Test 5
  Serial.print(F(" Test 5 - Testing if  MARCO resent if robot never receives POLO"));
  robotcmd.currState = RBT_IDLE;
  robotcmd.commandTimeout = 5000;
  delay(robotcmd.commandTimeout);
  outgoingMessage = robotcmd.update(millis(),""); //ensure we get MARCO
  passed = outgoingMessage[0] == RobotCmd::MARCO;
  outgoingMessage ="";
  elapsedTime =  robotcmd.lastCommandSent;
  while(robotcmd.currState == RBT_WAIT_POLO
    && outgoingMessage[0] != RobotCmd::MARCO)
  {
    outgoingMessage = robotcmd.update(millis(),"");
    delay(100);
  }
  elapsedTime = millis() - elapsedTime;
  bool robotReturnToIdle = robotcmd.currState == RBT_IDLE;
  delay(robotcmd.freeTime+100);
  String nextMessage = robotcmd.update(millis(),"");

  passed = passed //Marco was originally Sent
      && elapsedTime> robotcmd.commandTimeout
      && outgoingMessage == ""
      && nextMessage[0] == RobotCmd::MARCO
      && robotReturnToIdle
      && robotcmd.currState == RBT_WAIT_POLO; //we gave up waiting for Polo
  Serial.println(passed?testPassed:testFailed);


  //Test 6 
  Serial.print(F(" Test 6 - Testing if robot goes Idle -  if robot never receives DANCE after sending POLO "));
  robotcmd.currState = RBT_IDLE;
  robotcmd.commandTimeout = 5000;
  outgoingMessage = robotcmd.update(millis(),String(RobotCmd::MARCO)); //ensure we get MARCO
  elapsedTime = robotcmd.lastCommandSent;
  passed = outgoingMessage[0] == RobotCmd::POLO;
  outgoingMessage ="";
  while(robotcmd.currState == RBT_WAIT_DANCE
    && outgoingMessage == "")
  {
    outgoingMessage = robotcmd.update(millis(),"");
    delay(100);
  }
  elapsedTime = millis() - elapsedTime;
  passed = passed 
    && outgoingMessage==""
    && robotcmd.currState == RBT_IDLE
    && elapsedTime >  robotcmd.commandTimeout;
 Serial.println(passed?testPassed:testFailed);

  //Test 6b
  Serial.print(F(" Test 6b - Send up DANCE when robot is IDLE ensure that robot does not dance"));
    newSong = robotcmd.singer.createSong(0,50);
    newDance = robotcmd.dancer.createDance();
    entireMessage = RobotCmd::SONG+newSong+RobotCmd::DANCE+newDance;
    robotcmd.lastCommandSent = millis();
    outgoingMessage = robotcmd.update(millis(), entireMessage);
    passed = outgoingMessage == "" && robotcmd.currState == RBT_IDLE;
  Serial.println(passed?testPassed:testFailed);


  Serial.print(F(" Test 7 - DANGER Testing, robot should run when danger message sent"));
  robotcmd.currState = RBT_IDLE;
  robotcmd.commandTimeout = 5000;
  outgoingMessage = robotcmd.update(millis(),String(RobotCmd::DANGER)); //ensure we get MARCO
  outgoingMessage ="";
  int initialState = robotcmd.currState;
  long timeRunningAway = robotcmd.dangerTime;
  long timeHiding =  robotcmd.dangerTime;
  long timeReturning = robotcmd.dangerTime;
  int currMotorState = robotcmd.motor.motorState;
  
  while(robotcmd.currState == RBT_DANGER
    && outgoingMessage == "")
  {
    timeRunningAway =  currMotorState == STATE_BACKWARD &&  currMotorState != robotcmd.motor.motorState
              ?millis() - timeRunningAway
              :timeRunningAway;
    timeHiding = currMotorState == STATE_IDLE &&  currMotorState != robotcmd.motor.motorState
              ?millis() - timeHiding
              :timeHiding;
    currMotorState = robotcmd.motor.motorState;
    outgoingMessage = robotcmd.update(millis(),"");
    timeReturning =  robotcmd.currState != RBT_DANGER
              ?millis() - timeReturning
              :timeReturning;             
    delay(100);
  }
  
  passed = initialState == RBT_DANGER
    && outgoingMessage==""
    && robotcmd.currState == RBT_IDLE
    && timeRunningAway >  robotcmd.dangerNear
    && timeHiding >  robotcmd.dangerCautious
    && timeReturning >  robotcmd.dangerOver;
 Serial.println(passed?testPassed:testFailed);



  Serial.print(F(" Test 8 - DANGER Testing, robot should run when danger occurs in middle of dance"));
   robotcmd.currState = RBT_WAIT_POLO;
   newDance = robotcmd.update(currTime,String(RobotCmd::POLO));
   int dangerDelay = 3000;
   robotcmd.currState = RBT_WAIT_DANCE;
    robotcmd.update(currTime,newDance);
   while(  (millis() -robotcmd.lastCommandSent )<dangerDelay ){
    delay(100);
    robotcmd.update(millis(),"");
   }
   outgoingMessage = robotcmd.update(currTime,String(RobotCmd::DANGER));
   passed = robotcmd.currState == RBT_DANGER
          && !robotcmd.dancer.dancing
          && robotcmd.motor.motorState == STATE_BACKWARD
          && robotcmd.singer.getCurrSongIx()> robotcmd.singer.songLen;
    Serial.println(passed?testPassed:testFailed);
     
    robotcmd.update(millis(),"");

     Serial.print(F(" Test 9 - DANGER Testing, robot should send DANGER message if it is hit"));
   robotcmd.currState = RBT_WAIT_POLO;
   newDance = robotcmd.update(currTime,String(RobotCmd::POLO));
   int dangerDelay = 3000;
   robotcmd.accel.attacked=true;
  
   
   outgoingMessage = robotcmd.update(millis(),"");
   passed = robotcmd.currState == RBT_DANGER
          && outgoingMessage[0] == RobotCmd::DANGER
          && !robotcmd.dancer.dancing
          && robotcmd.singer.getCurrSongIx()> robotcmd.singer.songLen;
    Serial.println(passed?testPassed:testFailed);
     

        //send down Bad Song
         Serial.print(F(" Test 10 - BAD SONG  - Testing, robot should skip invalid notes"));
         Danced=0;
         Sang = 0;
        robotcmd.currState = RBT_WAIT_DANCE;
        robotcmd.commandTimeout = 5000;
        newSong = robotcmd.singer.createSong(0,50);
        newDance = robotcmd.dancer.createDance();
        newSong[10]='a';
        entireMessage = RobotCmd::SONG+newSong+RobotCmd::DANCE+newDance;
          
        outgoingMessage = robotcmd.update(millis(),entireMessage); //ensure we get MARCO
        elapsedTime = robotcmd.lastCommandSent;
        outgoingMessage ="";
        while(robotcmd.dancer.dancing)
        {
          motorState = robotcmd.dancer.motor.motorState;
          note = robotcmd.singer.getCurrSongIx();
          outgoingMessage = robotcmd.update(millis(),"");
          delay(100);  
          Sang += note != robotcmd.singer.getCurrSongIx()?1:0;
          Danced += motorState != robotcmd.dancer.motor.motorState?1:0;
        }
        Serial.println(Sang);
        Serial.println(Danced);
        passed = Danced >= robotcmd.dancer.danceLen
        && Sang >= robotcmd.singer.songLen
        && robotcmd.currState == RBT_IDLE;
       Serial.println(passed?testPassed:testFailed);

  

        Serial.print(F(" Test 11 - BAD SONG  - robot should dance but not sing if no song provided"));
         Danced=0;
         Sang = 0;
        robotcmd.currState = RBT_WAIT_DANCE;
        robotcmd.commandTimeout = 5000;
        newDance = robotcmd.dancer.createDance();
        String noSongMessage = String(RobotCmd::SONG);
        noSongMessage += String(RobotCmd::DANCE);
        noSongMessage +=newDance;
        outgoingMessage = robotcmd.update(millis(),noSongMessage); //ensure we get MARCO
        while(robotcmd.dancer.dancing)
        {
          motorState = robotcmd.dancer.motor.motorState;
          note = robotcmd.singer.getCurrSongIx();
          outgoingMessage = robotcmd.update(millis(),"");
          delay(100);  
          Sang += note != robotcmd.singer.getCurrSongIx()?1:0;
          Danced += motorState != robotcmd.dancer.motor.motorState?1:0;
        }
      
        passed = Danced >= robotcmd.dancer.danceLen
        && Sang ==0
        && robotcmd.currState == RBT_IDLE;
       Serial.println(passed?testPassed:testFailed);
    

      Serial.print(F(" Test 12 - BAD DANCE - robot should sing but not dance if no dance provided"));
         Danced=0;
         Sang = 0;
        robotcmd.currState = RBT_WAIT_DANCE;
        robotcmd.commandTimeout = 5000;
        newDance = robotcmd.dancer.createDance();
        String noDanceMessage = String(RobotCmd::SONG);
        noDanceMessage += robotcmd.singer.createSong(0,50);
        noDanceMessage += String(RobotCmd::DANCE);
       
        outgoingMessage = robotcmd.update(millis(),noDanceMessage); //ensure we get MARCO
        while(robotcmd.currState==RBT_DANCE)
        {
          motorState = robotcmd.dancer.motor.motorState;
          note = robotcmd.singer.getCurrSongIx();
          outgoingMessage = robotcmd.update(millis(),"");
          delay(100);  
          Sang += note != robotcmd.singer.getCurrSongIx()?1:0;
          Danced += motorState != robotcmd.dancer.motor.motorState?1:0;
        }
        passed = Danced==0
        && Sang >= robotcmd.singer.songLen
        && robotcmd.currState == RBT_IDLE;
       Serial.println(passed?testPassed:testFailed);
        
*/
Serial.print(F(" Test 13 - BAD DANCE - robot should pause dance if dance provides invalid commands "));
         Danced=0;
         Sang = 0;
        robotcmd.currState = RBT_WAIT_DANCE;
        robotcmd.commandTimeout = 5000;
        newDance = robotcmd.dancer.createDance();
        newDance[2]='a';
        newDance[3]='b';
        newDance[4]='f';
       String badDanceMessage = String(RobotCmd::SONG);
        badDanceMessage += robotcmd.singer.createSong(0,50);
        badDanceMessage += String(RobotCmd::DANCE);
        badDanceMessage += newDance;
      
        outgoingMessage = robotcmd.update(millis(),badDanceMessage); //ensure we get MARCO
        while(robotcmd.currState==RBT_DANCE)
        {
          motorState = robotcmd.dancer.motor.motorState;
          note = robotcmd.singer.getCurrSongIx();
          outgoingMessage = robotcmd.update(millis(),"");
          delay(100);  
          Sang += note != robotcmd.singer.getCurrSongIx()?1:0;
          Danced += motorState != robotcmd.dancer.motor.motorState?1:0;
        }
        passed = Danced>=1
        && Sang >= robotcmd.singer.songLen
        && robotcmd.currState == RBT_IDLE;
       Serial.println(passed?testPassed:testFailed);
         
  finishedTests = true;
	}
 
  }
