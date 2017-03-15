/*
  PAC: Detects PAC in during ECG measurement. 
  
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375
*/

// main PAC test, uses three tests to determine if PAC detected
int detectPAC() {
  if ( testPAC2 || testPAC3() ) {
    return 1;
  } else {
    return 0;
  }
}

int testPAC1() {
  double condRR = 0;
  for (int i = 2; i < 8; i++) {
    condRR += RR[i];
  }
  condRR *= 0.9;
  condRR /= 5;
  
  if (heartArray[1] < condRR) {
    return 1;
  } else {
    return 0;
  }
}

int testPAC2() {
  // two part test  
  double condRR1 = 0;
  double condRR2 = 0;
  for (int i = 2; i < 8; i++) {
    condRR1 += RR[i];
  }
  condRR2 = condRR1 / 5;
  condRR1 *= 1.1;
  condRR1 /= 5;

  if (heartArray[0] > condRR1 && heartArray[1] <= condRR2) {
    return 1;
  } else {
    return 0;
  }
}

int testPAC3() {
  double condRR = heartArray[0] / heartArray[1];
  if (condRR > 1.2) {
    return 1;
  } else {
    return 0;
  }
}


