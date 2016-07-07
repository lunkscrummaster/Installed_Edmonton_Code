/*
 * AS.cpp
 *
 *  Created on: May 30, 2016
 *      Author: Prospect
 */
#include "AS.h"
//--------------------------Variables------------------------------------------------------------

byte beeperToDo = 0;
byte beeperCountdown = 0;

/* --------------------------beeperSetup()------------------------------------------------------------
 *  Called from: Accustat::Accustat()
 *  1. Setups up oBeeper (pin 37) as output
 *  2. Writes the pin to low
 */
void beeperSetup() {
	pinMode(oBeeper, OUTPUT);
	digitalWrite(oBeeper, LOW);
} //end beeperSetup()

/*--------------------------beeperHeartbeat------------------------------------------------------------
 * CALLED FROM Accustat::heartbeat()
 * byte    beeperToDo      = 0;
 * byte    beeperCountdown = 0;
 * #define BEEPER_PERIODS  1    // beeps & pauses are this many heartbeats long
 * 1. starts or stops beeper, by writing to the oBeeper pin
 * 2. Write oBeeper to low just in case
 */
void beeperHeartbeat() {

	switch (accustat.returnmode()) {
	case ASM_INDIVIDUAL: {
		if (beeperToDo > 0) {
			if (beeperCountdown-- <= 1) {
				// start or stop beeper
				digitalWrite(oBeeper, (--beeperToDo & 1) ? HIGH : LOW); // write the beep on or off
//				Serial.print("beeperToDo :");
//				Serial.println((beeperToDo));
				beeperCountdown = BEEPER_PERIODS;
			} // end if (beeperCountdown-- <= 1)
		} else {
			digitalWrite(oBeeper, LOW);  // just to make sure
//				Serial.println(" beeperFlag = false");
			accustat.beeperFlag = false;
		}
		break;
	}
	case ASM_POWER:
	case ASM_STRENGTH: {
//		if (accustat.earlyPush == true) {
//			int dispIndex = accustat.pbAvg.getAverage() - accustat.getNaturalPreCharge();
//			Serial.print("set earlyPush false side disp: "); Serial.print(accustat.pressureArray[dispIndex]);
//			//Serial.print(" NAT_PRE: "); Serial.print(accustat.pressureArray[dispIndex]);
//			if (accustat.pressureArray[dispIndex] < ui.getVar(UIVM_POWER_THRESHOLD) * 100) {
//				digitalWrite(oBeeper, LOW);
//				accustat.earlyPush = false;
//				Serial.println("  &$%#$%^ early push beep off ");
//			}
//		}
		//if(accustat.returnState() == (AS_PREHIT | AS_POSTHIT))
		if (beeperToDo > 0) {
			if (beeperCountdown-- <= 1) {
				// start or stop beeper
				digitalWrite(oBeeper, (--beeperToDo & 1) ? HIGH : LOW); // write the beep on or off
				//				Serial.print("beeperToDo :");
				//				Serial.println((beeperToDo));
				beeperCountdown = BEEPER_PERIODS;
			} // end if (beeperCountdown-- <= 1)
		} else if (!accustat.earlyPush){
			digitalWrite(oBeeper, LOW);  // just to make sure
			//Serial.println(" beeperFlag = false");
			accustat.beeperFlag = false;
		}

		break;
	} // end strength
	} // end switch

	//Serial.println("beeper Heartbeat");

} //end beeperHeartbeat()

/* ------------------------beep(byte)-----------------------------------------------------------------------
 This is called from enterState below, and heartbeat below
 Count can be any of the below
 // number of times beeper sounds for new peaks
 //#define BEEP_COUNT_NEW_PEAK         1  // new peak for this cycle
 #define BEEP_NEW_SESS_PEAK          2  // new peak since last powerup/reset
 */
void beep(byte count) {
	beeperToDo = count * 2;  // odd values are beeps, even values are pauses
	beeperCountdown = BEEPER_PERIODS;
}  // end beep(byte)

const byte ledDigits[] = { 0xfc, 0x60, 0xda, 0xf2, 0x66, 0xb6, 0xbe, 0xe0, 0xfe,
		0xf6, 0x02 };  // dash

/*------------------------------------------display_num()------------------------------------------
 *   it is called from Accustat::displayHeartbeat() , Accustat::isEnabled()
 *  Displays on the large LED 4 DIGIT DISPLAY
 * 1. Just gets the values based on the number passed to it
 * 2. The values passed to it are the session peaks
 * Max possible values are just above ^^
 */
void display_num(int number) {

	if (number < LED_MIN)
		number = LED_MIN;
	if (number > LED_MAX)
		number = LED_MAX;

	byte thousands;
	if (number < 0) {
		number = -number;  // if number is negative, reset it to be positive
		thousands = LED_DASH_CODE; // writes a dash because of negative number
	} else {
		thousands = (number % 10000) / 1000; //this gets the first digit of the display from the left
	} //end else

	byte hundreds = (number % 1000) / 100; //gets the second didgit from the left
	byte tens = (number % 100) / 10; // get the third digit from the left
	byte ones = (number % 10); // gets the final single digits
	digitalWrite(oLEDClear, HIGH); // set the bit so we can write to the screen
	shiftOut(oLEDData, oLEDClock, LSBFIRST, ledDigits[ones]);
	shiftOut(oLEDData, oLEDClock, LSBFIRST, ledDigits[tens]);
	shiftOut(oLEDData, oLEDClock, LSBFIRST, ledDigits[hundreds]);
	shiftOut(oLEDData, oLEDClock, LSBFIRST, ledDigits[thousands]);
	digitalWrite(oLEDClear, LOW); //clear the bit so we CANT write to the screen

	delay(4);
} // end display_num

/* --------------------------displayHeartbeat------------------------------------------------------------
 *  Called from: Accustat::heartbeat()
 1. calls display_num, which updates the LED screen with either the sessionPeak or currentPeak
 */
void Accustat::displayHeartbeat() {
	// Serial.println("Accustat displayHeartbeat start");
	// if displayAlternateIndex == 0 then display sessionPeak else display currentPeak;
	// this cycles so that it displays sessionPeak, currentPeak, currentPeak, currentPeak, ...
	//Serial.print("dispAltInd: ");Serial.print(displayAlternateIndex); Serial.print("  sessionPeak: "); Serial.print(sessionPeak);Serial.print("  currentPeak: "); Serial.println(currentPeak);
	if(mode == ASM_STRENGTH){
    if(master.strengthDisplayFlag == true){
	    if(++accustat.strengthDisplayCounter > 50 ){
            accustat.strengthDisplayCounter = 100;
//            Serial.print("stDispCount: "); Serial.println(accustat.strengthDisplayCounter);
	          if (currentPeak == 0)
                displayAlternateIndex = 0;
            display_num(displayAlternateIndex == 0 ? sessionPeak : currentPeak);
	      } else {
          display_num(sessionPeak);
	      }
      }else{
          display_num(sessionPeak);
        }
	  }else{
    	if (currentPeak == 0)
      		displayAlternateIndex = 0;
      	  display_num(displayAlternateIndex == 0 ? sessionPeak : currentPeak);
	  }
	if (displayAlternateCountdown-- <= 1) {
		displayAlternateCountdown = DISPLAY_PERIOD;
		switch (displayAlternateIndex) {
		case 0:  // (fall into next)
		case 1:  // (fall into next)
		case 2:
			displayAlternateIndex++;
			break;
		case 3:
			displayAlternateIndex = 0;
			break;
		}  // end switch
	}
} // end Accustat::displayHeartbeat()

//--------------------------ACCUSTAT_Constructor -----------------------------------------------------
Accustat::Accustat() :
  pressureArray { 0,  10,  20,  29,  39,  49,  59,  69,  78,  88,  98, 108, 118,
				127, 137, 147, 157, 160, 163, 166, 169, 172, 175, 178, 181, 184,
				187, 190, 193, 196, 211, 226, 241, 256, 271, 286, 302, 317, 332,
				347, 362, 377, 392, 409, 425, 442, 458, 474, 490, 507, 523, 539,
				555, 571, 588, 611, 632, 655, 676, 698, 720, 741, 763, 785, 805,
				827, 849, 871, 893, 915, 937, 959, 981, 1021, 1059, 1099, 1138,
				1177, 1205, 1233, 1261, 1289, 1317, 1345, 1373, 1398, 1422,
				1447, 1471, 1496, 1521, 1523, 1539, 1555, 1571, 1587, 1603,
				1619, 1635, 1651, 1667, 1683, 1699, 1715, 1766, 1806, 1845,
				1884, 1923, 1962, 1990, 2018, 2046, 2074, 2102, 2130, 2158,
				2183, 2207, 2232, 2256, 2281, 2305, 2330, 2354, 2373, 2393,
				2413, 2432, 2452, 2471, 2491, 2510, 2530, 2550, 2661, 2671,
				2682, 2691, 2702, 2712, 2722, 2732, 2746, 2765, 2780, 2795,
				2810, 2825, 2840, 2856, 2871, 2886, 2901, 2916, 2931, 2943,
				2960, 2973, 2987, 3000, 3013, 3026, 3039, 3052, 3065, 3078,
				3091, 3104, 3117, 3131, 3139, 3164, 3186, 3208, 3230, 3252,
				3274, 3295, 3317, 3335, 3345, 3361, 3378, 3394, 3410, 3426,
				3443, 3459, 3475, 3492, 3508, 3531, 3541, 3558, 3574, 3590,
				3607, 3623, 3639, 3656, 3672, 3689, 3705, 3728, 3747, 3763,
				3778, 3793, 3808, 3823, 3838, 3853, 3868, 3883, 3898, 3913, 3924 } {
	beeperSetup();
	cooldownCounter           = 0;
	currentPeak               = 0;
	hiddenPeak                = 0;
	sessionPeak               = 0;
	lastReading               = 0;
	displayAlternateCountdown = 0;
	displayAlternateIndex     = 0;
	hasSeenBall               = false;
	mode                      = ASM_INDIVIDUAL;
	state                     = AS_QUIET;
	precharge = 0;
	naturalPreCharge = 0;
	hitTrip = 4;
	aveTimerStart = false;
	aveTimer = 0;
	earlyPush = false;
	beeperFlag = false;
	lookForBall = false;
	printBall = false;
	dumpValveFlag = false;
	stDurFailFlag = false;
	stNoBallFailFlag = false;
	setPreChargeFlag = false;
  postHitCycle = 0;
  offEarlyFlag = false;
  offEarlyStart = 0;
  offEarlyEnd = 0;
  offEarlyTime = 0;
  successCount = 0;
  strengthDisplayCounter = 0;

}
/* --------------------------Accustat::loop()------------------------------------------------------------
 * #define AS_QUIET      0    #define AS_PREHIT     1   #define AS_HITTING    2 #define AS_POSTHIT    3
 * Called from: main loop
 * 1. Checks for pad hit to wake up machine
 */
void Accustat::loop() {
	if (mode == ASM_INDIVIDUAL)
		hitTrip = 5;
	else
		hitTrip = 8;

	if (isEnabled() && state != AS_QUIET && master.arrayFullFlag) {
		// update running sum
//		int p = analogRead(aiAchievedPin); //read current value and assign to p
//		pbAvg.update(p); //update the average, using the last read value
//		master.noInterrupt = true;
		int avg = master.pushbackPresAve; //updated average
//		master.noInterrupt = false;
//    Serial.print("p: "); Serial.print(p); Serial.print(" avg: "); Serial.println(avg);
//    Serial.print(" diff: "); Serial.println(p-avg);
		switch (state) {
		case AS_PREHIT: {
//			Serial.print(" live: "); Serial.print(analogRead(aiAchievedPin));
//			Serial.print(" avg: "); Serial.print(avg);
//			Serial.print(" naturalprecharge: "); Serial.println(naturalPreCharge);
//			for(int i = 0; i < AVG_NUM_READINGS; i++){
//				Serial.print(i); Serial.print(": "); Serial.print(master.pushbackPresArray[i]); Serial.print("  ");
//			}
//          int diff = p - avg; //difference between the current and average reading
			int diff = avg - naturalPreCharge;
			lastReading = avg;
			//Serial.print(" avg: "); Serial.print(avg); Serial.print(" diff: "); Serial.println(diff);
//			Serial.print(" NAT_PRE: "); Serial.print(naturalPreCharge);
//			Serial.print(" diff: "); Serial.println(diff);
//			master.noInterrupts = true;
			if (diff > hitTrip && outriggers.getBalanceMode() == false) {
//				Serial.print(" avg: ");
//				Serial.print(avg);
//				Serial.print(" lastReading: ");
//				Serial.print(lastReading);

//				Serial.println("");
//				master.noInterrupts = false;
				ui.accustatDiff = diff;
				enterState(AS_HITTING);	//enter AS_HITTING since the pads are being hit
				//assign the last reading to the average
			}
		}	//end case AS_PREHIT
			break;

		case AS_HITTING:
			switch (mode) {
			case ASM_INDIVIDUAL:
				lastReading = avg;

//            if (lastReading < p)
//              // update peak reading
//              lastReading = p;
				break;
			case ASM_POWER: //since no break, will execute ASM_STRENGTH even though its ASM_POWER
			case ASM_STRENGTH:
          strengthDisplayCounter = 0;
//				if (currentPeak < hiddenPeak) {
//					saveHiddenPeak();
////					Serial.print(" currentPeak: ");
////					Serial.println(currentPeak);
//				}
				lastReading = avg;
//				if (hasSeenBall) {
//					int presArrIndex = avg - naturalPreCharge;
//					if (presArrIndex < 0)
//						presArrIndex = 0;
//					float change = (float) pressureArray[presArrIndex]
//							/ (float) currentPeak;
//					//Serial.print(" pressureArray[presArrIndex]: "); Serial.print(pressureArray[presArrIndex]);
//					//Serial.print("  CurrPeak: "); Serial.print(currentPeak);
//					//Serial.print(" disp "); Serial.print(pressureArray[lastReading - naturalPreCharge]);
////						Serial.print(" last reading: "); Serial.print(lastReading);
////						Serial.print(" pressureArray[presArrIndex]: "); Serial.print(pressureArray[presArrIndex]);
////						Serial.print("  CurrPeak: "); Serial.print(currentPeak);
////						Serial.print(" chagne: "); Serial.println(change);
//
//					if (change < 0.6 && currentPeak > 0) {
//						enterState(AS_POSTHIT);
//						Serial.println(" hasSeenball false ");
//						setHasSeenBall(false);
//					}
//
//				}
				break;
			} // end switch (mode)
			break;
		} // end switch (state)
//   if(pressureArray[lastReading-naturalPreCharge] > hiddenPeak)
//       hiddenPeak = pressureArray[lastReading-naturalPreCharge];
	} // end first  if (isEabled() && state != AS_QUIET) {
} // end of Accustat loop

/* ---------------------------------------Accustat::isEnabled()---------------------------------------
 Function is called from Accustat::enable (below) , and Accustat::loop()
 1. The function returns true/false depending on oDisplayPowerPin (pin 27)
 2. this oDisplayPowerPin controls large LED DISPLAY. display is off while sleep, else on
 */
boolean Accustat::isEnabled() {
	return digitalRead(oDisplayPowerPin);
} // end Accustat::isEnabled()

/* ---------------------------------------Accustat::enable---------------------------------------
 Function is called from nowhere, this is not implemented yet
 This function appears to impliment a sleep state for the LED DISPLAY
 */
void Accustat::enable(boolean en) {
	if (isEnabled()) { //if enabled,
		if (!en) {
			// disabling
			digitalWrite(oDisplayPowerPin, LOW); //turn off display
			enterState(AS_QUIET); //enter quiet mode
//			Serial.println(" Accustat::enablen called AS_Quiet");
		}
	} else {
		if (en) {
			// enabling
//			digitalWrite(oDisplayPowerPin, HIGH);
			display_num(0);
//			enterState(AS_QUIET);
//			Serial.println(" Accustat::enablen AS_Quiet");
		}
	} // end else
} // end Accustat::enable(boolean)

/* ---------------------------------------Accustat::reset()------------------------------------------------------------
 This function is called from UISystem::loop()
 1. The function resets the statistics when the set button is pressed
 2. turns off the beeper
 */
void Accustat::reset() {
	sessionPeak = currentPeak = 0;
	ui.accustatDiff = 0;
//	Serial.println("ACCUSTAT RESET THE SESSION PEAK AND CURRENT PEAK");
	digitalWrite(oBeeper, LOW);
} // end Accustat::reset()

/* ---------------------------------------Accustat::pause()------------------------------------------------------------
 This function is called from PushbackSystem::enterState(byte newState)
 Also possibly called from PushbackSystem::reReady(), but this is currently not implemented
 1. This function sets the accustat state to AS_QUIET
 */
void Accustat::pause() {
	enterState(AS_QUIET);
//	Serial.println(" Accustat::pause AS_Quiet");

} // end Accustat::pause()

/* ---------------------------------------Accustat::resume()---------------------------------------
 This function is called from MasterSystem::pushbackIsReady()
 1. This function changes the accustat state too AS_PREHIT
 */
void Accustat::resume() {
//  Serial.println("resume called AS_PREHIT");
	enterState(AS_PREHIT);
} // end Accustat::resume()

/* ---------------------------------------Accustat::setMode (byte)---------------------------------------
 This function is called from PushbackSystem::goReady(
 1. This function assigns the Accustat mode to whatever was passed in
 */
void Accustat::setMode(byte m) {
	mode = m;
} //end Accustat::setMode

/* ---------------------------------------Accustat::saveHiddenPeak()---------------------------------------
 This function is called from MasterSystem::loop()
 1. set currentPeak = hiddenPeak
 */
void Accustat::saveHiddenPeak() {
	currentPeak = hiddenPeak;
} // end Accustat::saveHiddenPeak

/* ---------------------------------------Accustat::enterState(byte)---------------------------------------
 This function is called from Accustat::resume() , Accustat::pause() , Accustat::isEnabled() , Accustat::loop() , Accustat::heartbeat()
 1. sets the accustat state to whatever was passed to it
 2.depending on the state, perform diffent duties
 */
void Accustat::enterState(byte newState) {
	state = newState;
//	Serial.print("Accustat enterState: ");
//	Serial.println(newState);
	switch (state) {
	case AS_QUIET:
		displayAlternateIndex = DISPLAYMODE_ALTERNATE; // this code indicates "alternate between current peak and session peak"
		break;

	case AS_PREHIT:
		// reset the running average
//		pbAvg.reset();
//		accustat.setNaturalPreCharge();
//		Serial.print(" NATTYPRECHARGY: "); Serial.println(naturalPreCharge);
		break;

	case AS_HITTING:
		DEBUG_PRINT_S("\n AS->HITTING\n");
		//hasSeenBall = false;
    master.strengthDisplayFlag = false;
    strengthDisplayCounter = 0;
		if (mode != ASM_INDIVIDUAL)
			lookForBall = true;
//		Serial.println(" lookForBall = true");
		cooldownCounter = COOLDOWN_PERIODS; // COOLDOWN_PERIODS  8 minimum hitting phase is this # of heartbeats long
		precharge = lastReading;
		currentPeak = hiddenPeak = 0; //reset the peaks
//		displayAlternateIndex = DISPLAYMODE_CURRPEAK; // this code indicates "display current peak"
		break;

	case AS_POSTHIT:
		DEBUG_PRINT_S(" AS->POSTHIT\n\n");
		digitalWrite(oBeeper, LOW); //turn off any noise
    sleep.awakeStartTime = millis();
    if(currentPeak < hiddenPeak){
      currentPeak = hiddenPeak;
      hiddenPeak = 0;
      //beep(BEEP_NEW_SESS_PEAK);
    }
		if (mode == ASM_POWER || mode == ASM_STRENGTH) {
			dumpValveFlag = false;
		}
		if (sessionPeak < currentPeak) {
			sessionPeak = currentPeak; //reset new session peak
      currentPeak = 0;
//			Serial.println(" sessionPeak set inside accustat.enterState");
			if (beeperFlag == false) {
//				Serial.println("beepFLAG = true");
				beeperFlag = true;
				beep(BEEP_NEW_SESS_PEAK);
//				Serial.println(" AS_POSTIHT beep");
			} //#define BEEP_NEW_SESS_PEAK 2  // new peak since last powerup/reset
		}
		displayAlternateIndex = DISPLAYMODE_ALTERNATE; // this code indicates "alternate between current peak and session peak"
		if(!stDurFailFlag && !stNoBallFailFlag) {
			master.accustatEnteringPosthit();
		}
			else if (stNoBallFailFlag) {
				stNoBallFailFlag = false;
			}
		else {
			ui.goStrengthPosthit(UISPH_TOO_FAST, ui.getVar(UIVM_STRENGTH_DURATION)-1);
		}
   hiddenPeak = 0;
		break;
	} //end switch (state)
} // end Accustat::enterState

/* ---------------------------------------Accustat::heartbeat()---------------------------------------
 1. go into beeperHeartbeat, which starts or stops the beep
 2. go into displayHeartbeat, which updates the LED screen
 3. Checks state, and does things depending on the state
 */
void Accustat::heartbeat() {
//	Serial.println("Accustat Heartbeat Started");
	beeperHeartbeat(); //just above
	displayHeartbeat(); //just above
//	Serial.print("lastReading: "); Serial.println(lastReading);

	switch (state) {
	case AS_HITTING: {
		int x = master.pushbackPresAve - naturalPreCharge;
		if (x < 0)
			x = 0;
//		Serial.print(" diff: ");
//		Serial.print(x);

		int disp = pressureArray[x];
//		Serial.print(" display: ");
//		Serial.print(disp);
//		Serial.print(" aianalogread: ");
//		Serial.println(analogRead(aiAchievedPin));

		switch (mode) {
		case ASM_INDIVIDUAL:
			if (hiddenPeak < disp) {
				  hiddenPeak = disp;
			} 
			break;
      
			// below will call a penalty if the players are pushing to hard before the ball is in???
		case ASM_POWER: {
			if (disp > ENGAGED_MIN) {
				int threshold = ui.getVar(UIVM_POWER_THRESHOLD) * 100;
        if (hiddenPeak < disp) {
              hiddenPeak = disp;
        }
				if (disp <= threshold) {
					if (earlyPush == true) {
						digitalWrite(oBeeper,LOW);
//						Serial.println(" disp<thresh, turn off beep");
						earlyPush = false;
					}
					if (hasSeenBall) {
						if (printBall) {
//							Serial.println(" ball true");
							printBall = false;
						}
//						if (hiddenPeak < disp) {
//							hiddenPeak = disp;
////							if (hiddenPeak > currentPeak)
////								currentPeak = hiddenPeak;
//						}
					} else {
						//Serial.println("disp < thresh, noBall");
					}
				} else { //disp > thresh
					if (lookForBall) {
						digitalWrite(oBeeper, HIGH);
//						Serial.println("beeper turned on for the early push        ");
						//Serial.print(" disp: "); Serial.println(disp);
						earlyPush = true;
					} else {
						if (hiddenPeak < disp) {
							hiddenPeak = disp; //TADD
//							if (hiddenPeak > currentPeak)
//								currentPeak = hiddenPeak;
						}
						//	Serial.println("disp > thresh, yesBall");
					}
				} // end else
				  // closes if (disp > threshold)
			} // end if (disp > ENGAGED_MIN)
		} // end case ASM_POWER
			break;

		case ASM_STRENGTH: {
			/* ADDED, by trevor and zach May 21
			 So if the machine has a reading of 100lbs of force or more, then is can possibly pushback
			 */
			if (disp > ENGAGED_MIN) {
				int threshold = ui.getVar(UIVM_STRENGTH_THRESHOLD) * 100;
				//Serial.println(" hasSeenBall: "); //Serial.println(hasSeenBall);

				if (disp <= threshold) {
					if (earlyPush == true) {
						digitalWrite(oBeeper,LOW);
//						master.noInterrupt = true;
////						Serial.println("  turn off ");
//						master.noInterrupt = false;
						earlyPush = false;
//						Serial.print(" off disp: "); Serial.print(disp);
//						Serial.print("	off AVE: "); Serial.print(master.pushbackPresAve);
//						Serial.print("	 off NAT_PRE: "); Serial.println(naturalPreCharge);
					}
					if (hasSeenBall) {
						if (printBall) {
//							Serial.println(" ball true");
							printBall = false;
						}
						if (hiddenPeak < disp) {
							hiddenPeak = disp;
//							if (hiddenPeak > currentPeak)
//								currentPeak = hiddenPeak;
						}
					} else {
						//Serial.println("disp < thresh, noBall");
					}
				} else { //if disp > thresh
					if (lookForBall) {
						digitalWrite(oBeeper, HIGH);
//						master.noInterrupt = true;
//						Serial.print("__earlyPush ");
//						Serial.print(" disp: "); Serial.print(disp);
//						Serial.print("	 AVE: "); Serial.print(master.pushbackPresAve);
//						Serial.print("	 NAT_PRE: "); Serial.println(naturalPreCharge);
//						master.noInterrupt = false;
						earlyPush = true;

					} else {
						if (hiddenPeak < disp) {
							hiddenPeak = disp; //TADD
//							if (hiddenPeak > currentPeak)
//								currentPeak = hiddenPeak;
						}
						//	Serial.println("disp > thresh, yesBall");
					}
				} // end else  disp > thresh
				  // closes if (disp > threshold)
			} // end if (disp > ENGAGED_MIN)
		} // end case ASM_POWER
			break;
		} // end switch (mode)


		if (returnmode() == ASM_INDIVIDUAL) {
			//Serial.print("  average: "); Serial.print(pbAvg.getAverage()); Serial.print(" precharge "); Serial.print(naturalPreCharge);
//			master.noInterrupt = true;
			int tempavg = master.pushbackPresAve;
//			master.noInterrupt = false;
			if (hiddenPeak > 0 && tempavg - naturalPreCharge < 6) {
				enterState(AS_POSTHIT);
			}
		} else {

			if (cooldownCounter > 0)
				cooldownCounter--;
//			else {
			switch (mode) {
			case ASM_INDIVIDUAL:
//                  enterState(AS_POSTHIT);
				break;
			case ASM_POWER:
     
				if (returnState() == AS_HITTING) {
					if (hasSeenBall) {
//						master.noInterrupt = true;
						int tempavg2 = master.pushbackPresAve;
//						master.noInterrupt = false;
						if (hiddenPeak > 0 && (tempavg2 - naturalPreCharge) < 6  && disp < hiddenPeak) { //&& (tempavg2 - naturalPreCharge) < 6
							//
//					int avePB = pbAvg.getAverage();
//					if(avePB < naturalPreCharge && hasSeenBall){
//							Serial.print("%Diff: "); Serial.print(change);
//							Serial.print("  Seen Ball: "); Serial.print(hasSeenBall);
//							Serial.print("  lastRead: "); Serial.print(lastReading);
//							Serial.print("  NAT_PRE: "); Serial.print(naturalPreCharge);
//							Serial.print("  currPeak: "); Serial.print(currentPeak);
//							Serial.print("  Look Flag: "); Serial.println(lookForBall);

							enterState(AS_POSTHIT);
//							Serial.println(" hasSeenball false ");
							setHasSeenBall(false);
							//						setNaturalPreCharge();
						}
					} else { // end if(hasSeenBall)
						//if pressure drops below currentpeak/2, then they came off before the ball was in, reset the machine
						if(master.pushbackPresAve <= naturalPreCharge + 10 && !offEarlyFlag){
						      offEarlyFlag = true;
                  offEarlyStart = millis();
//                  Serial.print(" off Earlt Start Time: "); Serial.println(offEarlyStart);
						  }
             
            if(master.pushbackPresAve > naturalPreCharge && offEarlyFlag){
//                  Serial.print(" off Earlt Start Time: "); Serial.println(offEarlyStart);        
                  offEarlyFlag = false;
            }

					  if(offEarlyFlag){
  							offEarlyEnd = millis();
                if(offEarlyEnd < offEarlyStart)
                    offEarlyTime = offEarlyEnd - offEarlyStart + MILLIS_MAX;
                else
                    offEarlyTime = offEarlyEnd - offEarlyStart;
  							if(offEarlyTime > 1000){
    							lookForBall = false;
    							stNoBallFailFlag = true;
                  offEarlyFlag = false;
//                  Serial.println("1 called AS_PREHIT");
    							enterState(AS_PREHIT);
  							}
					}

					} // end else no ball, off early
				} // end AS_HITTING
				break;

			case ASM_STRENGTH:
				if (returnState() == AS_HITTING) {
					if (hasSeenBall) {
						//Serial.print(" strengthChargeTimeoutMillis: "); Serial.println(master.strengthChargeTimeoutMillis);
//						if(master.strengthChargeTimeoutMillis == 0 &&  master.successStartTime == 0) {
						if (master.successOverFlag_AS) { // what about them coming off to early
//						if (currentPeak > 0 && pbAvg.getAverage() - naturalPreCharge < 6 && disp < currentPeak && dumpValveFlag == true) {
               if(++successCount > 10){
                  enterState(AS_POSTHIT);
                  master.successOverFlag_AS = false;
    //              Serial.println(" hasSeenball false ");
                  setHasSeenBall(false);                    
                }
//					int avePB = pbAvg.getAverage();
//					if(avePB < naturalPreCharge && hasSeenBall){
//							Serial.print("%Diff: "); Serial.print(change);
//							Serial.print("  Seen Ball: "); Serial.print(hasSeenBall);
//							Serial.print("  lastRead: "); Serial.print(lastReading);
//							Serial.print("  NAT_PRE: "); Serial.print(naturalPreCharge);
//							Serial.print("  currPeak: "); Serial.print(currentPeak);
//							Serial.print("  Look Flag: "); Serial.println(lookForBall);
							enterState(AS_POSTHIT);
							master.successOverFlag_AS = false;
//							Serial.println(" hasSeenball false ");
							setHasSeenBall(false);
							//						setNaturalPreCharge();
						} else {
							//haven't reached success, and ball has been seen and the pressure dropped
							if (disp <= currentPeak / 4) {
//								hiddenPeak = 0;

								stDurFailFlag = true;
								hasSeenBall = false;

								//Serial.println("                     RESET BECAUSE OF PRESSURE DROP ");

								enterState(AS_POSTHIT);
//								Serial.println("                     RESET BECAUSE OF PRESSURE DROP ");

//								delay(5000); //DELAY ask Kevin how long the error message should be on the screen for???
								ui.enterState(UIS_SCRUM_STRENGTH);
								pushback.enterState(PBS_READY1_SINKING);

								//reset other flags when needed
							}
						}
					} else {  // end if(hasSeenBall)
            if(master.pushbackPresAve <= naturalPreCharge + 10 && !offEarlyFlag){
                  offEarlyFlag = true;
                  offEarlyStart = millis();
//                  Serial.print(" off Earlt Start Time: "); Serial.println(offEarlyStart);
              }
             
            if(master.pushbackPresAve > naturalPreCharge && offEarlyFlag){
//                  Serial.print(" off Earlt Start Time: "); Serial.println(offEarlyStart);        
                  offEarlyFlag = false;
            }

            if(offEarlyFlag){
                offEarlyEnd = millis();
                if(offEarlyEnd < offEarlyStart)
                    offEarlyTime = offEarlyEnd - offEarlyStart + MILLIS_MAX;
                else
                    offEarlyTime = offEarlyEnd - offEarlyStart;
                if(offEarlyTime > 1000){
                  lookForBall = false;
                  stNoBallFailFlag = true;
                  offEarlyFlag = false;
//                  Serial.println("2 called AS_PREHIT");
                  enterState(AS_PREHIT);
                  stNoBallFailFlag = true;
                }
						}
					}
				} // end AS_HITTING
				break;
			} // end switch (mode)
			//closes else ie cooldownCounter = 0
		} // end else

	} // end case AS_HITTING
		break;

	case AS_POSTHIT:
    if(++postHitCycle == 150){
        postHitCycle = 0;
//        Serial.println("POST_HIT called AS_PREHIT");
		    enterState(AS_PREHIT);
    }
	} // end switch (state)

	//Serial.print(" hasSeenBall = "); Serial.println(hasSeenBall);
} //end Accustat::heartbeat()

/* ---------------------------------------Accustat::returnmode()---------------------------------------
 *  Called from: beeperHeartbeat() Accustat::heartbeat , MasterSystem::loop , sonarISR
 *  1. Returns the Accustat mode
 */
byte Accustat::returnmode() {
	return mode;
} // end returnmode

/* ---------------------------------------Accustat::returnstate()---------------------------------------
 *  Called from: Accustat::heartbeat , MasterSystem::loop
 *  1. Returns the Accustat state
 */
byte Accustat::returnState() {
	return state;
} // end returnState

/* ---------------------------------------Accustat::getHasSeenBall()---------------------------------------
 *  Called from: sonarISR in main page
 *  1. Returns the hasSeenBall
 */
boolean Accustat::getHasSeenBall() {
	return hasSeenBall;
} // end Accustat::getHasSeenBall()

/* ---------------------------------------Accustat::setHasSeenBall()---------------------------------------
 *  Called from: sonarISR() , MasterSystem::loop , Accustat::heartbeat
 *  1. Sets hasSeenBall
 */
void Accustat::setHasSeenBall(boolean ball) {
	hasSeenBall = ball;
} // end Accustat::setHasSeenBall

/* ---------------------------------------Accustat::setNaturalPreCharge()---------------------------------------
 * Called from: MasterSytem::loop
 * 1. Sets the naturalPreCharge to the live average
 */
void Accustat::setNaturalPreCharge(void) {
	naturalPreCharge = master.pushbackPresAve;
//	Serial.print(" natural precharge: "); Serial.println(naturalPreCharge);

//	Serial.print("NAT_PRE");
//	Serial.println(naturalPreCharge);
} // end Accustat::setNaturalPreCharge(

/* ---------------------------------------Accustat::getNaturalPreCharge()---------------------------------------
 * not used since naturalPreCharge is public
 *
 */
int Accustat::getNaturalPreCharge(void) {
	return naturalPreCharge;
} // end Accustat::getNaturalPreCharge(

