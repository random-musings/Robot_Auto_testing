#include "Arduino.h"

#include "RobotCmd.h"

//messages that can be sent/processed
	const char RobotCmd::MARCO ='m';
	const char RobotCmd::POLO ='p';
	const char RobotCmd::DANGER ='d';
	const char RobotCmd::DANCE ='_';
	const char RobotCmd::SONG ='&';
	
	
// *************************
// STRING PARSING FUNCTIONS
// *************************

  char *  strwrd(char *s, char *buf, size_t len, const char *delim)
  {
    s += strspn(s, delim);
    int n = strcspn(s, delim); 
    if (len-1 < n)           
      n = len-1;
    memcpy(buf, s, n);
    buf[n] = 0;
    s += n;
    return (*s == 0) ? NULL : s;
  }
  

RobotCmd::RobotCmd(){
	dangerNear = 5000;
	dangerCautious = 9000;
	dangerOver = 15000;
	commandTimeout = 15000;

	stateChanged = 0;
	currTimeout = 0;
	safeFeeling=25;
	freeTime =0;
	lastCommandSent = 0; 
}


void RobotCmd::setup(int buzzerPin) {
	  accel.init(20,        // int sampleSize,
				 -7,-7,8,   // int offsetX, int offsetY, int offsetZ,
				 0.5,0.13,0.03,  // newAttackTolerance, newCollisionTolerance, double newMovingTolerance,
				 2000);     // long newAccelShakeTimeout);

	  singer.setup(buzzerPin);
	  dancer.setup(motor);
	  singer.stop();
	  dancer.stop();
  }
  
  
  String RobotCmd::update(long currTime,String incomingMessage )
  {
	accel.update(currTime);
	//Serial.print ("... accel...");
	//Serial.print ("... process...");
	dancer.update(currTime);
	singer.update(currTime); 
	//Serial.print ("... singer...");
	//motor.update(currTime);
	//Serial.print ("... motor...");
	String outGoingMessage =processMessage(incomingMessage,currTime); 
	return outGoingMessage;
  }
  
  
  
String RobotCmd::processMessage(String incomingMessage, long currTime)
{
  char* msg = const_cast<char*>( incomingMessage.c_str());

  if(detectNewDanger(msg,currTime)){
    return String(DANGER);
  }
  
 if( processDanger(currTime))
 {
  Serial.println("process Danger");
  return String("");
 }
 
  if(accel.collision){
  Serial.println("process Collision");
   motor.setState(STATE_COLLISION);
   accel.collision = false;
   return "";
  }

  if(currState == RBT_IDLE && msg[0]==MARCO) //received marco
  {
    //Serial.println("process MARCO");
    currState = RBT_WAIT_DANCE;
    return  String(POLO); //send back polo;
  }
  if(currState == RBT_WAIT_POLO && msg[0]==POLO) //recived polo
  {
    //Serial.println("process POLO");
    currState = RBT_DANCE;
    String newSong = singer.createSong(safeFeeling-10,safeFeeling+10);
    String newDance =dancer.createDance();
    return String(SONG+newSong+DANCE+newDance);
  }
  if(currState == RBT_WAIT_DANCE && msg[0]==SONG)
  {
  //  Serial.println("process SONG");

    String sSep(SONG);
    String dSep(DANCE);
    
    //split message into song and dance
    char * song = new char[255* sizeof(char)];
    char * dance = new char[255* sizeof(char)];
    dance = strwrd ( msg,song,255* sizeof(char),dSep.c_str());
    //strip first char off of theDance and song and everything is correct; or handle it within the routine
    singer.playSong(const_cast<char*>(song+1));
    dancer.performDance(const_cast<char*>(dance+1));
    currState = RBT_DANCE;
    return "";
  }
  
  //we just finished dancing
  if(currState == RBT_DANCE)
  {
	if( !dancer.dancing)
	{
		currState = RBT_IDLE;
		freeTime = rand() *1000;//set the time before we connect to other robot
	}
	return "";
  }

 if(currState == RBT_IDLE)
 {

    if((currTime - freeTime )>  commandTimeout)
    { //see if robot is responding
      currState = RBT_WAIT_POLO;
      lastCommandSent = currTime;
	  Serial.print(".");
      return String(MARCO);
    }
 }

  if((currTime - lastCommandSent) >commandTimeout)
  {
      currState = RBT_IDLE;
      lastCommandSent = currTime;
      freeTime = rand() *500+currTime;
  }
 
  return "";
}

bool RobotCmd::processDanger(long currTime)
{
  bool processed = false;
  if(currState== RBT_DANGER )
  {
    if((currTime - dangerTime) >dangerNear){ //1st stage after running away
      motor.Stop();
      motor.setState(STATE_IDLE);
    }
    if((currTime - dangerTime) >dangerCautious){ //2nd stage creep back slowly
      motor.setState(dir=='f'?STATE_FORWARD:STATE_BACKWARD);
      motor.motorSpeed = SLOW;
    }
    if((currTime - dangerTime) >dangerOver){ //3rd stage being playing
      motor.setState(STATE_IDLE);
      motor.Stop();
      currState = RBT_IDLE;
      //reset the wait for Marco - Polo to some random interval
    }
    processed = true;
  }
  return processed;
}



bool RobotCmd::detectNewDanger(char* incomingMessage,long currTime)
{
  if(currState!= RBT_DANGER )
  {
    if(incomingMessage[0]=='d' || accel.attacked)
    {
      singer.stop();
      dancer.stop();
      motor.motorSpeed =FAST;
      motor.setState(dir=='f'?STATE_FORWARD:STATE_BACKWARD);
      currState = RBT_DANGER;
      dangerTime = currTime;
      return true;
    }
  }
  return false;
}