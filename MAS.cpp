/*
 * MAS.cpp
 *
 *  Created on: May 30, 2016
 *      Author: Prospect
 */
#include "MAS.h"

//---------------------------------------MASTER_CLASS_CONSTRUCTOR---------------------------------------
MasterSystem::MasterSystem() {
  lastReadyState = UIS_SCRUM_INDIVIDUAL;
  strengthChargeTimeoutMillis = 0;
  strengthPosthitTimeoutHeartbeats = 0;

  successStartTime = 0;
  lastMillis = 0;

  lastUIState = UIS_TOWING;
  lastTowSwitch = MAS_LTS_ON;

  outriggerLooseIndex = 0;
  outriggerLooseSonarAve = 0;

  outriggerTightIndex = 0;
  outriggerTightSonarAve = 0;

  pushbackPresIndex = 0;
  pushbackPresAve = 0;

  pushbackIndex = 0;
  pushbackSonarAve = 0;

  debugCounter = 0;

  noInterrupt = false;
  successOverFlag_AS = false;
  successOverFlag_UI = false;
  balancingDone = false;
  arrayFullFlag = false;
  towSwitchFlag = false;
  strengthDisplayFlag = false;
}


/* ------------------------------------------MasterSystem loop------------------------------------------
    This function is called from the main loop
    1. Read PushBackSonar and Air pressure in main push back system
    2. If the sonar value is > CHARGE_DISTANCE_TRIP (390), ui.goStrengthPosthit - which just updates the screen display
    3. If the pressure in the tank is > CHARGE_PRESSURE_TRIP  (400) ,  which goes into ui.goStrengthPosthit which just updates the display
*/
void MasterSystem::loop() {
/*
 * Emergency shutdown removed from interrupt, and put into a faster loop
*/
	if(ui.getState() == UIS_SCRUM_STRENGTH_CHARGE){
		if(accustat.returnmode() == ASM_STRENGTH && accustat.returnState() == AS_HITTING){
        digitalWrite(oPushbackSonarTrigger, HIGH);
        delayMicroseconds(10);
        digitalWrite(oPushbackSonarTrigger, LOW);
        delayMicroseconds(2);
        int sonar = pulseIn(iPushbackSonar, HIGH, 3000)/58;
			if(sonar > 24) {
//					ui.pbTooFar = true;
				master.strengthChargeTimeoutMillis = 0;
				accustat.setHasSeenBall(false);
//				Serial.println("      EMERGENCY SHUTDOWN        ");
				ui.goStrengthPosthit(UISPH_TOO_HIGH, 0);
				delay(5000);
				accustat.enterState(AS_POSTHIT);
				pushback.enterState(PBS_READY1_SINKING);

			}
		}
	} // end shutdown

    master.pushbackSonarAve -= master.pushbackSonar[master.pushbackIndex]/AVE_ARRAY_SIZE;
    int temp =  analogRead(aiPushbackSonar);

    if(temp > 130 || temp < 58){
      if(master.pushbackIndex == 0){
        master.pushbackSonar[master.pushbackIndex] = master.pushbackSonar[AVE_ARRAY_SIZE-1];
      }else{
        master.pushbackSonar[master.pushbackIndex] = master.pushbackSonar[master.pushbackIndex-1];
      }
    }else{
      master.pushbackSonar[master.pushbackIndex] =  temp;
    }
    master.pushbackSonarAve += master.pushbackSonar[master.pushbackIndex]/AVE_ARRAY_SIZE;
    if(++master.pushbackIndex == AVE_ARRAY_SIZE)
      master.pushbackIndex = 0;


    if(accustat.lookForBall == true) {
      if(analogRead(aiLoose_ball_sonar) < BALLIN_TRIP || analogRead(aiTight_ball_sonar) < BALLIN_TRIP){
        accustat.setHasSeenBall(true);
        accustat.lookForBall = false;
        accustat.printBall = true;
//        Serial.println("original check saw ball");      
        if(accustat.returnmode() == ASM_STRENGTH){
          master.UIModeChanged(UIS_SCRUM_STRENGTH_CHARGE);
          ui.enterState(UIS_SCRUM_STRENGTH_CHARGE);
        }
      }
    } // end if(accustat.lookForBall == true)

	if(outriggers.getBalanceMode() == true){
		updateOutriggerLooseArray();
		updateOutriggerTightArray();
	}
 
	if(balancingDone){
		int newReading = analogRead(aiAchievedPin);
		master.pushbackPresAve -= master.pushbackPresArray[master.pushbackPresIndex]/AVG_NUM_READINGS;
		master.pushbackPresArray[master.pushbackPresIndex] = newReading;
		master.pushbackPresAve += master.pushbackPresArray[master.pushbackPresIndex]/AVG_NUM_READINGS;

		if(++master.pushbackPresIndex == AVG_NUM_READINGS) {
			master.pushbackPresIndex = 0;
			arrayFullFlag = true;
		}
		if (arrayFullFlag && accustat.setPreChargeFlag == true) {
			delay(1000);
			for(int i = 0; i < AVG_NUM_READINGS; i++){
					int newReading = analogRead(aiAchievedPin);
					master.pushbackPresAve -= master.pushbackPresArray[i]/AVG_NUM_READINGS;
					master.pushbackPresArray[i] = newReading;
					master.pushbackPresAve += master.pushbackPresArray[i]/AVG_NUM_READINGS;
			}
			accustat.setNaturalPreCharge();
//     Serial.print("NatPre: ");Serial.println(accustat.getNaturalPreCharge());
			accustat.setPreChargeFlag = false;
      digitalWrite(oDisplayPowerPin, HIGH);
		}
//		master.noInterrupts = true;
//		Serial.print(" live: "); Serial.print(analogRead(aiAchievedPin));
//		Serial.print(" natPre: "); Serial.print(accustat.getNaturalPreCharge());
//		Serial.print(" avg: ");
//
//		Serial.println(master.pushbackPresAve);
//		for(int i = 0; i < AVG_NUM_READINGS; i++){
//							Serial.print(i); Serial.print(": "); Serial.print(master.pushbackPresArray[i]); Serial.print(" ");
//						}
//						Serial.println(" ");
//						Serial.println("                                         ");
//
//						//setNaturalPreCharge();
//		master.noInterrupts = false;

	}

  if (strengthChargeTimeoutMillis > 0) {
//	master.noInterrupts = true;
////    volatile int son = pushbackSonarAve;
//    master.noInterrupts = false;
    int pres = analogRead(aiAchievedPin);

//    if (son > CHARGE_DISTANCE_TRIP) { //#define CHARGE_DISTANCE_TRIP  390   // if sonar over this, shutdown
//      // if sonar too high,
//      strengthChargeTimeoutMillis = 0;
//      ui.goStrengthPosthit(UISPH_TOO_HIGH, son - CHARGE_DISTANCE_TRIP);//this just updates the lcd screen display
//      return;
//    }

    // check sonar speed
    //  long sonarMillis = millis();
    //  if (???) {
    //    // if sonar increasing too fast,
    //    strengthChargeTimeoutMillis = 0;
    //    ui.goStrengthPosthit(UISPH_TOO_FAST, 0);
    //    return;
    //  }
    //  lastSonarDistance = son;
    //  lastSonarMillis   = sonarMillis;

//    if (pres > CHARGE_PRESSURE_TRIP) {   // if pressure over this, shutdown (400 means approx. 30 lbs)
//      // if pushback pressure too much,
////    	Serial.println("too much pressure - timeout reset");
//      strengthChargeTimeoutMillis = 0;
//      ui.goStrengthPosthit(UISPH_TOO_MUCH, pres - CHARGE_PRESSURE_TRIP);
//      return;
//    }

    /* Code Added by Trevor and Zach
        The code below, keeps track of the timer for the strength charge push, which after they hold this for a certain
        time, the sled is released, and it can be pushed. The sled push length is handled by anothter timer.
        Rollover is taken into account here.
    */
    // check for Duration timeout
    unsigned long m = millis();
    unsigned long elapsedMillis = 0;
//    Serial.print(" m: "); Serial.print(m); Serial.print("  lastMillis: ");Serial.print(lastMillis);
    if (m < lastMillis) {
        elapsedMillis =(unsigned long)(m + (MILLIS_MAX - lastMillis)); //rollover
//        Serial.println(" ROLLOVER ");
    }
    else
    	elapsedMillis = m - lastMillis;
    lastMillis = m;
    strengthChargeTimeoutMillis -= elapsedMillis;
//    Serial.print("  elapsedMillis: ");Serial.print(elapsedMillis);

    if (strengthChargeTimeoutMillis < 50) {
      strengthChargeTimeoutMillis = 0;
      digitalWrite(oSuccess, HIGH); //allow for the sled to move
//      Serial.println("dumpvalve written High");
      master.successOverFlag_UI = false;
//      Serial.println("_____________SUCCESS ON_____________");
//
//      Serial.print(" AS over  flag: "); Serial.print(master.successOverFlag_AS);
//      Serial.print(" UI over flag: "); Serial.print(master.successOverFlag_UI);
//      Serial.print(" dumpValveFlag: "); Serial.print(accustat.dumpValveFlag);
//      Serial.print(" beeperFlag: "); Serial.print(accustat.beeperFlag);
//
//
//      Serial.print(" AS: "); Serial.print(accustat.returnState());
//      Serial.print(" PB: "); Serial.print(pushback.getState());
//      Serial.print(" SS: "); Serial.print(sleep.getState());
//      Serial.print(" UIS: "); Serial.println(ui.getState());
      successStartTime = millis();

      ui.goStrengthPosthit(UISPH_SUCCESS, 0);  //changes the screen to display success
      accustat.saveHiddenPeak();
    }
  } // end if (strengthChargeTimeoutMillis > 0)
}// end master loop


/* ------------------------------------------MasterSystem::heartbeat()------------------------------------------
    This function is called from heartbeat() in the main page
    // states of 'lastTowingSwitch'
    1. If the towing switch is in towing up, change LCD to display towing mode
    2. If not in towing up, change LCD to display last state
    3. Does some more stuff that I DON'T UNDERSTAND RIGHT NOW
*/
void MasterSystem::heartbeat() {
//Serial.print(" Tight ball: "); Serial.println(analogRead(aiTight_ball_sonar));
  //  Serial.println("Master System Heartbeat Started");
//Serial.print(" iTrailerPowerPin status: "); Serial.println(digitalRead(iTrailerPowerPin));
  // check towing switch
  if (halIsTowScrumSwitchInTowing()) {
		if (lastTowSwitch != MAS_LTS_ON) {
		  // enter Towing mode
		  lastTowSwitch = MAS_LTS_ON;
//		  Serial.println("Tow Switch turned off LED");
		  ui.enterState(UIS_TOWING); //display towing state
		  digitalWrite(oDisplayPowerPin, LOW);
		}
  } else {
    if (lastTowSwitch != MAS_LTS_OFF) {
      // exit Towing mode
      lastTowSwitch = MAS_LTS_OFF;
      ui.enterState(lastReadyState); //display last state
//      towSwitchFlag = true;
      halSetPushbackUpDown(-1);
//      Serial.println("exit tow switch call ui.enterState");
//      digitalWrite(oDisplayPowerPin, HIGH);
    }
  } // end else

  /* **** CHAGNED MADE By Trevor Zach and Kevin
      Currently, the state lastUIState is always == 1, and the swtich statement below is a little weird
  */
  byte uiState = ui.getState(); 		// returns a value 0 -> 5 depending on the state
  byte ssState = sleep.getState(); 		//returns the value of the state 0 = Asleep, 1 = Awake

  if (uiState > 0 && ssState == 1) {
    accustat.enable(true);
    pushback.enable(true);
    if (uiState > 2) {
      initcharge.enable(true);
    }
  } else {
    if (digitalRead(iTrailerPowerPin) == LOW) {
      accustat.enable(false);
      pushback.enable(false);
      initcharge.enable(false);
    } // if the vehicle is hooked up, write above to low
  } // end if
  /* Removed code below
    boolean isLastUIStateNotTowing = lastUIState != UIS_TOWING;
    initcharge.enable              (isLastUIStateNotTowing);
    pushback.enable                (isLastUIStateNotTowing);
    accustat.enable                (isLastUIStateNotTowing);
  */
  digitalWrite(oReservoirLockout, sleep.getState() == SSS_AWAKE ? HIGH : LOW); //switches a boolean to high or low appropriately

  //***** the switch statement is lacking logic because it is switching a variable that has just been
  // define to == 1 ?????????????????????????????

  // Things we do every heartbeat while UI is in each state:
  switch (lastUIState) {
    case UIS_TOWING:
      digitalWrite(oAirSpringLockout, LOW);// if towing, turn pin off
      ui.setVar(UIVN_TOWING_RESPRESS, analogRead(aiReservoirPin));
      break;

    case UIS_SCRUM_STRENGTH_POSTHIT:
      if (successOverFlag_UI) {
    	  //pushback.enterState(PBS_READY1_SINKING);
    	  //Serial.println(" calling PBS_READY1_SINKING early");
 //   	  ui.restartStrengthFlag = true;
    	  ui.enterState(UIS_SCRUM_STRENGTH);
    	  successOverFlag_UI = false;
      }
      break;

      // (other states do nothing)nnn
  } // end switch (lastUIState)
} // end MasterSystem::heartbeat()


/* ------------------------------------------MasterSystem::pushbackIsReady()------------------------------------------
    This function is called from PushbackSystem::heartbeat()
    1. reset the timeout just does this : enterState(AS_PREHIT); in accustat
    // called by PushbackSystem when Ready cycle has finished
*/
void MasterSystem::pushbackIsReady() {
//  accustat.resume(); this makes the accustat enter prehit before the machine is done moving
  strengthChargeTimeoutMillis = 0;
} // end MasterSystem::pushbackIsReady()


/* ------------------------------------------MasterSystem::accustatEnteringPosthit()------------------------------------------
    This function is called from Accustat::enterState
    1. reset the timer
    2. writes the beeper low
    3. UI is the last ready state
*/
void MasterSystem::accustatEnteringPosthit() {
  strengthChargeTimeoutMillis = 0;
  /* Changes made May 20, 2016 by trevor zach and lunk
      We are commenting all writing of oSuccess to low
      This will be done using a timer. So after there is a success, the timer will start.
      This timer will be edited by Kevin in the field to what his preference is.
    digitalWrite(oSuccess, LOW);  // just to make sure
  */
  if (lastReadyState != UIS_SCRUM_INDIVIDUAL){
    ui.enterState(lastReadyState);
//    Serial.print(" lastState :"); Serial.print(lastReadyState); Serial.print("UIState");Serial.println(ui.getState());
    //Serial.println("accustatEnterPostHit caused ghost");
  }
} //end MasterSystem::accustatEnteringPosthit


/* ------------------------------------------MasterSystem::UIModeChanged(byte uis)------------------------------------------
    Called from: UISystem::enterState(byte newState)
    1. The lastUIState is assigned to the byte that is passed to it
    2. Based on the state, initializes the pushback arm, and initcharge when needed
    3. Also assigns ladtReadyState = lastUIState
*/
void MasterSystem::UIModeChanged(byte uis) {
  lastUIState = uis;

  switch (lastUIState) {
    case UIS_SCRUM_INDIVIDUAL:
    case UIS_SCRUM_POWER:
    case UIS_SCRUM_STRENGTH:
      lastReadyState = lastUIState;
      /* Changes made May 20, 2016 by trevor zach and lunk
          We are commenting all writing of oSuccess to low
          This will be done using a timer. So after there is a success, the timer will start.
          This timer will be edited by Kevin in the field to what his preference is.
        digitalWrite(oSuccess, LOW);  // just to make sure
      */
      break;
  } // end switch
  //  Serial.println("uiModeChanged called goReady");
  //  Serial.print("lastUIState: "); Serial.println(lastUIState);
  switch (lastUIState) {
    case UIS_SCRUM_INDIVIDUAL:  pushback.goReady(ASM_INDIVIDUAL, INDIVIDUAL_SINK_RAISE);  	break;
    case UIS_SCRUM_POWER:	    pushback.goReady(ASM_POWER,      POWER_SINK_RAISE);			break;

    case UIS_SCRUM_STRENGTH:
      initcharge.enable(true);
      initcharge.setTargetPercent(10 * ui.getVar(UIVM_STRENGTH_DIFFICULTY));
      pushback.goReady(ASM_STRENGTH, STRENGTH_SINK_RAISE);
      break;

    case UIS_SCRUM_STRENGTH_CHARGE:
      lastMillis = millis();
//      Serial.println("Master in STrengh Charge");
      strengthChargeTimeoutMillis = ui.getVar(UIVM_STRENGTH_DURATION) * 1000L;
//      Serial.println(" strengthChargeTimeoutMillis was set");
      break;

    case UIS_SCRUM_STRENGTH_POSTHIT:
    	if(!accustat.stDurFailFlag)
      strengthPosthitTimeoutHeartbeats = STRENGTH_POSTHIT_HEARTBEATS;

      break;
  } // end switch (lastUIState)
} // end MasterSystem::UIModeChanged(byte uis)


/* ------------------------------------------MasterSystem::UIVarChanged(byte uivn, int val)------------------------------------------
    Called From: UISystem::setVar(byte vn, int val)
    1. Changed the Strength difficulty to the passed value
    2. Only does this when in Strength mode
*/
void MasterSystem::UIVarChanged(byte uivn, int val) {

  switch (uivn) {
    case UIVM_STRENGTH_DIFFICULTY:
      initcharge.setTargetPercent(10 * val);
      break;
  } // end switch
} // end MasterSystem::UIVarChanged(byte uivn, int val)


/* ------------------------------------------MasterSystem::outriggerLooseAve()------------------------------------------
    Called from:  MasterSystem::loop() , MasterSystem::getOutriggerLooseAve()
    Called from MasterSystem::loop() , getOutriggerTightAve()
    1. If the outriggerLooseSonar[] is not full, it will calculate the average until it is full
    2. If outriggerLooseSonar[] is full, it will delete the first cell, shift them down, then calculate the average
    3. outriggerLooseSonarAve is the variable that holds the current average
*/
int MasterSystem::getOutriggerLooseAve() {
	outriggerLooseSonarAve = 0;
    for (int i = 0; i < AVE_ARRAY_SIZE; i++) {
        outriggerLooseSonarAve += outriggerLooseSonar[i];
    } // end for
    return outriggerLooseSonarAve/AVE_ARRAY_SIZE;
} // end MasterSystem::outriggerLooseAve()


/* ------------------------------------------MasterSystem::outriggerTightAve()------------------------------------------
    Called from MasterSystem::loop() , getOutriggerTightAve()
    1. If the outriggerTightSonar[] is not full, it will calculate the average until it is full
    2. If outriggerTightSonar[] is full, it will delete the first cell, shift them down, then calculate the average
    3. outriggerTightSonarAve is the variable that holds the current average
*/
int MasterSystem::getOutriggerTightAve() {
	outriggerTightSonarAve = 0;
    for (int i = 0; i < AVE_ARRAY_SIZE; i++) {
        outriggerTightSonarAve += outriggerTightSonar[i];
    } // end for
    return outriggerTightSonarAve/AVE_ARRAY_SIZE;
} // end MasterSystem::outriggerTightAve()


/* ------------------------------------------MasterSystem::getOutriggerLooseAve()------------------------------------------
    Called from OutriggerSystem::loop() , OutriggerSystem::heartbeat(void)
    1. Calls outriggerLooseAve(); to update the average
    2. Returns the latest calculated average
*/
void MasterSystem::updateOutriggerLooseArray() {
	outriggerLooseSonar[outriggerLooseIndex] = analogRead(aiOutriggerLooseSonar);
	if(outriggerLooseSonar[outriggerLooseIndex] > 190 || outriggerLooseSonar[outriggerLooseIndex] < 58){
				if(outriggerLooseIndex == 0){
					outriggerLooseSonar[outriggerLooseIndex] = outriggerLooseSonar[AVE_ARRAY_SIZE-1];
				}else{
					outriggerLooseSonar[outriggerLooseIndex] = outriggerLooseSonar[outriggerLooseIndex-1];
				}
			}
	outriggerLooseIndex++;
	if(outriggerLooseIndex == AVE_ARRAY_SIZE)
		outriggerLooseIndex = 0;
} // end MasterSystem::getOutriggerLooseAve()


/* ------------------------------------------MasterSystem::getOutriggerTightAve()------------------------------------------------
    Called from OutriggerSystem::loop() , OutriggerSystem::heartbeat(void)
    1. Calls the outriggerTightAve(); to update the average
    2. Returns the latest calculated average
*/
void MasterSystem::updateOutriggerTightArray() {
	outriggerTightSonar[outriggerTightIndex] = analogRead(aiOutriggerTightSonar);
	if(outriggerTightSonar[outriggerTightIndex] > 190 || outriggerTightSonar[outriggerTightIndex] < 58){
				if(outriggerTightIndex == 0){
					outriggerTightSonar[outriggerTightIndex] = outriggerTightSonar[AVE_ARRAY_SIZE-1];
				}else{
					outriggerTightSonar[outriggerTightIndex] = outriggerTightSonar[outriggerTightIndex-1];
				}
	}
	outriggerTightIndex++;
	if(outriggerTightIndex == AVE_ARRAY_SIZE)
		outriggerTightIndex = 0;
} // end MasterSystem::getOutriggerTightAve()


//-------------------------------------------fillOutriggerArray------------------------------------------------------------------
/* Called from OutriggerSystem::setBalanceMode
 * 1. This fills the ourtigger array with sonar values until the array is full
*/
void MasterSystem::fillOutriggerArray(){
	for(int i = 0; i < AVE_ARRAY_SIZE; i++){
		outriggerLooseSonar[i] = analogRead(aiOutriggerLooseSonar);
		outriggerTightSonar[i] = analogRead(aiOutriggerTightSonar);
		if(outriggerLooseSonar[i] > 190 || outriggerLooseSonar[i] < 58){
			if(i == 0){
				outriggerLooseSonar[i] = outriggerLooseSonar[AVE_ARRAY_SIZE-1];
			}else{
				outriggerLooseSonar[i] = outriggerLooseSonar[i-1];
			}
		}
		if(outriggerTightSonar[i] > 190 || outriggerTightSonar[i] < 58){
			if(i == 0){
				outriggerTightSonar[i] = outriggerTightSonar[AVE_ARRAY_SIZE-1];
			}else{
				outriggerTightSonar[i] = outriggerTightSonar[i-1];
			}
		}
	} // end for
} // end MasterSystem::fillOutriggerArray()

/*-------------------------------------------getLastUIState------------------------------------------------------------------
 * Called from:
 * 1. Returns the lastUIState variable
*/
byte MasterSystem::getLastUIState() {
	return lastUIState;
} // end MasterSystem::getLastUIState()



