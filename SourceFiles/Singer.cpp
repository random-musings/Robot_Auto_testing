#include <Arduino.h>
#include "Singer.h"

	Singer::Singer()
	{
		buzzerPin = 13;
		currSongIx = 0;
		lastPlay =0 ;
		singing = false;
	}
	
	void Singer::setup(int buzzPin)
	{
		buzzerPin = buzzPin;
	}
	
	char *  Singer::strwrd(char *s, char *buf, size_t len, char *delim)
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


	String Singer::createSong(int minValue,int maxValue)
	{
	  int currNote= 0;
	  int noteIx;
	  int durationSign=1;
	  int noteSign = 1;
	  String noteIxStr = "0";
	  String durationStr="0";
	  for(int i=0;i<songLen;i++)
	  {
		if( currNote ==i)
		{
		  currNote = i+( rand()%5)+1;
		  noteIx =  minValue + (rand() % maxValue); //keep song in lower bars if we had danger recently    
		  noteSign = (noteIx %2 ==0)?1:-1;
		}
		noteIx += noteSign;
		noteIx = noteIx<0|| noteIx>49? ((minValue+maxValue)/2) :noteIx; //reset if we went off the note array
		noteIxBar[i] = noteIx;
		noteIxStr = noteIxStr +delim+noteIx;
		
		int duration = millis();
		durationSign = duration%2==0?1:-1;
		duration = (duration +50 * durationSign)%700;
		duration =  duration<50? 50:duration;  
		durationBar[i] = duration;
		durationStr = durationStr+delim+duration;
	  }
	  String fullSong = noteIxStr+sep+durationStr;
	  return fullSong.c_str();
	}
	
	void Singer::playSong(char* newSong)
	{
			//load the song
		char * song = new char[2*songLen* sizeof(char)];
		char * pauses = new char[2*songLen* sizeof(char)];
		newSong=strwrd ( newSong,song,2*songLen* sizeof(char), const_cast<char*>(sep));
		strwrd ( newSong,pauses,2*songLen* sizeof(char), const_cast<char*>(sep));
	

		char *note = new char[songLen];
		char *noteHold = new char[songLen];
		int i=0;
		while(song !=NULL && pauses !=NULL)
		{
			song = strwrd ( song,note,songLen* sizeof(char), const_cast<char*>(delim));
			pauses= strwrd( pauses,noteHold,songLen* sizeof(char), const_cast<char*>(delim));
			int noteIx = atoi(note);
			int pauseMillis = atoi (noteHold);
			noteIxBar[i] = noteIx;
			durationBar[i] = pauseMillis;
			i++;
		}

		delete note;
		delete noteHold;
		currSongIx =i<2?songLen:0;
		singing = currSongIx ==0;

	}

	
	void Singer::stop()
	{
		noTone(buzzerPin);
		currSongIx= songLen+1;
		singing = false;
	}
	
void Singer::update(long currTime)
	{
	   if(  (currTime - lastPlay)>durationBar[currSongIx])
	  {
			if(currSongIx<songLen)
			{
			/*
			   Serial.print(" zznote ");
		Serial.print(noteIxBar[currSongIx]);
		Serial.print("zz delay ");
		Serial.println(durationBar[currSongIx]);
		*/
			 tone(buzzerPin,notes[noteIxBar[currSongIx]],durationBar[currSongIx]);
			 lastPlay = currTime;
			 currSongIx++;
			 }else{	
				noTone(buzzerPin);
				singing=false;
			 }

	  }
	}
	

	int Singer::getCurrSongIx()
	{
		return currSongIx;
	}