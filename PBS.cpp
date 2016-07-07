/*
 * PBS.cpp
 *
 *  Created on: May 30, 2016
 *      Author: Prospect
 */
#include "PBS.h"

//--------------------------------------PUSHBACK_CONSTRUCTOR--------------------------------------
PushbackSystem::PushbackSystem() {
	state = PBS_QUIET;
	readyRaiseTo = 0;
	readySinkTo = 0;
	settlingTimeout = 0;
	enabled = false;
  distance = 0;
  newDistance = false;
}

/* --------------------------------------PushbackSystem::heartbeat()--------------------------------------
    This is called from the heartbeat function in the main program page
*/
void PushbackSystem::heartbeat() {

//	volatile int sonAve = master.pushbackSonarAve;//find value of pushback sonar
//  int son = analogRead(aiPushbackSonar);
//  while(son > 130 || son < 58){
//        son = analogRead(aiPushbackSonar);
//        Serial.println("bad Sonar Value");
//    }
//  int sonarDirect = analogRead(aiPushbackSonar);
//  Serial.print("avrPB: "); Serial.print(sonAve);
//  digitalWrite(oPushbackSonarTrigger, LOW);
//  delayMicroseconds(2);
  digitalWrite(oPushbackSonarTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(oPushbackSonarTrigger, LOW);
  delayMicroseconds(2);
//  while(newDistance = false);
  int son = pulseIn(iPushbackSonar, HIGH, 3000)/58;
  while(son == 0){
      son = pulseIn(iPushbackSonar, HIGH, 10000)/58;
  }
//  newDistance = false;
////  pulseIn(iPushbackSonar, HIGH, 100000)/58;
//  Serial.print("Interrupt start: "); Serial.print(startTime);
//  Serial.print("    Interrupt end: "); Serial.print(endTime);
//  Serial.print("    Interrupt Dist: "); Serial.print(distance);
//  Serial.print("    PulseIn Dist: "); Serial.println(son);

//  master.noInterrupt = false;
  switch (state) {
    case PBS_READY1_SINKING:
      /* Changes made May 20, 2016 by Trevor, Zach and Kevin
          readySinkTo is the lower limit. The machine will sink to this value, or below then start raising.
          the machine will raise until readyRaiseTo is less than or equal to the sonar value
          once this raise is complete, the machine enters a settling state for apprx 1sec, then is quiet
      */
//      digitalWrite(oAirSpringLockout, LOW);
//      delay(200);
//      digitalWrite(oAirSpringLockout, HIGH);  //written high to allow air into the springs
//      delay(50);
//      if(master.towSwitchFlag){
//          if(analogRead(aiOutriggerLooseSonar) < 90){
//                master.towSwitchFlag = false;
//            }
//        }else 
        if (readySinkTo >= son) {
        // start raising
        digitalWrite(oAirSpringLockout, HIGH);  //written high to allow air into the springs
//        Serial.println(" PBS heartbeat changed PBS State: left sinking, now raising ");
        enterState(PBS_READY2_RAISING);
      }
      break;

    case PBS_READY2_RAISING:
//       Serial.print("  readyRaiseTo= ");Serial.print(readyRaiseTo);
       //Serial.print("  son = "); Serial.println(son);
      if (readyRaiseTo <= son) {
        // start settling
        enterState(PBS_READY3_SETTLING);
      }else{enterState(PBS_READY2_RAISING);}
      break;

    case PBS_READY3_SETTLING:
      if (settlingTimeout-- <= 0) {
        // finished settling
        enterState(PBS_QUIET);
        master.pushbackIsReady();
      }
      break;
  } // end switch (state)
} //end pushback heartbeat

/* --------------------------------------PushbackSystem::enable--------------------------------------
    This function is called from MasterSystem::heartbeat()
    1. Enables, or disables the "enabled" variable that belongs to the pushback system class
    2. This is set to true or false

*/
void PushbackSystem::enable(boolean en) {
  if (en) {
    if (!enabled) {
      // enabling
      enabled = true;
    } // end if
  } else {
    if (enabled) {
      // disabling
      enabled = false;
      enterState(PBS_QUIET);
    } // end if (enabled)
  } // end else
} // end PushbackSystem::enable

/* --------------------------------------PushbackSystem::enterState--------------------------------------
 *  this takes a new state, or current state
    Function is called from MAS.cpp
    1. depending on the state, depends on what it does
    2. View indiviudual states for further comments
*/
void PushbackSystem::enterState(byte newState) {
  state = newState;
//    Serial.print ("pbs new st: "); Serial.println(state);
  switch (state) {
    case PBS_QUIET:
      halSetPushbackUpDown(0);
      //outriggers.setBalanceMode(false);
      break;

    case PBS_READY1_SINKING:
    	master.balancingDone = false;
      digitalWrite(oDisplayPowerPin, LOW);
      digitalWrite(oAirSpringLockout, LOW); // when low, suspension is availble to go down
//      Serial.println(" PBS_READY1_SINKING cause AS_Quiet ");
      accustat.pause();
      halSetPushbackUpDown(-1);
      break;

    case PBS_READY2_RAISING:
//      Serial.println("PBS_READY2_RAISE called setBalanceMode");
      halSetPushbackUpDown(1);
//      Serial.println("PBS_Raising_Set_BalanceMode_True");
       //beging to balance the machine
      break;

    case PBS_READY3_SETTLING:
      halSetPushbackUpDown(0);
      //outriggers.setBalanceMode(false);
      settlingTimeout = SETTLING_COUNT;
      outriggers.setBalanceMode(true);
      break;
  } // end switch (state)
}//end enterState

/* --------------------------------------goReady--------------------------------------
 * , is called from MAS.cpp
    1. changes the accustate state to whatever state was passed in
    2. enters new state of PBS_READY1_SINKING which is ^^^
*/
void PushbackSystem::goReady(byte asMode, int sinkTo, int raiseTo) {
  readySinkTo  = sinkTo;
  readyRaiseTo = raiseTo;
  //Serial.print("	readyRaiseto = "); Serial.println(readyRaiseTo);
  accustat.setMode(asMode);
//  Serial.println(" goReady changed PBS STATE");
  enterState(PBS_READY1_SINKING);// this is above^^
//  Serial.println(" enter to early PBS_READY1_SINKING");
}//end goReady

/* --------------------------------------PushbackSystem::getState()--------------------------------------
 *  Called from sonarISR() , OutriggerSystem::loop()
 *  1. Returns the current state from the PushbackSystem
*/
byte PushbackSystem::getState() {
  return state;
} // end PushbackSystem::getState()



