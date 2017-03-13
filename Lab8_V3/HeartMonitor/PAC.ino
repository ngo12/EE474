/*
  DETECTS PAC
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375
*/


double RR[7] = {0};  // holds current and prev RR interval values

// main PAC test, uses three tests to determine if PAC detected
int detectPAC() {
  RR[0] = PeriodOfRR;
  if (testPAC1() || testPAC2 || testPAC3()) {
    for (int i = 0; i < 6; i++) {
      RR[i+1] = RR[i];
    }
    return 1;
  } else {
    for (int i = 0; i < 6; i++) {
      RR[i+1] = RR[i];
    }
    return 0;
  }
}

int testPAC1() {
  double condRR = 0;
  for (int i = 2; i < 7; i++) {
    condRR += RR[i];
  }
  condRR *= 0.9;
  condRR /= 5;
  
  if (RR[1] < condRR) {
    return 1;
  } else {
    return 0;
  }
}

int testPAC2() {
  // two part test  
  double condRR1 = 0;
  double condRR2 = 0;
  for (int i = 2; i < 7; i++) {
    condRR1 += RR[i];
  }
  condRR2 = condRR1 / 5;
  condRR1 *= 1.1;
  condRR1 /= 5;

  if (RR[0] > condRR1 && RR[1] <= condRR2) {
    return 1;
  } else {
    return 0;
  }
}

int testPAC3() {
  double condRR = RR[0] / RR[1];
  if (condRR > 1.2) {
    return 1;
  } else {
    return 0;
  }
}


