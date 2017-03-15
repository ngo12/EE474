/*
  Filtering: Contains all the filters used to obtain 
  a cleaner signal from the raw ECG measurement.
  
  Lab 8
  Course: EE 474

  Names and Student Numbers:
  Ryan Linden: 1571298
  Khalid Alzuhair: 1360167
  Brandon Ngo: 1462375
*/

// 2nd order bandpass, butterworth, fs = 250, 3-18Hz
float numerator[5] = {
     0.1602003508877,                 0,  -0.1602003508877, 0, 0
};
float denominator[5] = {
                   1,   -1.650692627742,   0.6795992982245,0, 0
};

//// 2nd order bandpass, butterworth, fs = 250, 3-22Hz
//float numerator2[5] = {
//     0.1957561306596,                 0,  -0.1957561306596, 0, 0
//};
//
//float denominator2[5] = {
//                   1,   -1.574426496615,   0.6084877386808, 0, 0
//};

//// 4th order LPF, 40Hz
//float numerator2[5] = {
//    0.02287020771629,  0.09148083086516,   0.1372212462977,  0.09148083086516,
//    0.02287020771629
//};
//
//float denominator2[5] = {
//                   1,   -1.411983501197,    1.122766080821,  -0.4080709518802,
//    0.06321169571625
//};

//// 4th order LPF, 50Hz
//float numerator2[5] = {
//    0.04658290663644,   0.1863316265458,   0.2794974398187,   0.1863316265458,
//    0.04658290663644
//};
//
//float denominator2[5] = {
//                   1,  -0.7820951980233,   0.6799785269163,   -0.182675697753,
//    0.03011887504317
//};

// 4th order LPF, 60Hz
float numerator2[5] = {
    0.08267262046299,    0.330690481852,    0.496035722778,    0.330690481852,
    0.08267262046299
};

float denominator2[5] = {
                   1,  -0.1562105841383,   0.4937426525744, -0.03290721805863,
    0.01813707703045
};

// butterworth filter used for filtering 
// ECG measurement data 
double inSignal[5] = {0, 0, 0, 0, 0};
double outSignal[5] = {0, 0, 0, 0, 0}; 
double LPF() {
inSignal[0] = (double) adcValueCopy; 
outSignal[0] = numerator[0]*inSignal[0] +  numerator[1]*inSignal[1] +  numerator[2]*inSignal[2]  +  numerator[3]*inSignal[3] +  numerator[4]*inSignal[4]-  denominator[1]*outSignal[1] - denominator[2]*outSignal[2] - denominator[3]*outSignal[3] - denominator[4]*outSignal[4]; 
for (int i = 0; i < 5 - 1; i++) {
  inSignal[i+1] = inSignal[i]; 
}
for (int i = 0; i < 5 - 1; i++) {
  outSignal[i+1] = outSignal[i];
}
return  outSignal[0];
}

// Band-pass Filter used for filtering 
// ECG measurement data 
double BPF(double val) {
  inBPF[0] = val; 
  outBPF[0] = numerator2[0]*inBPF[0] +  numerator2[1]*inBPF[1] +  numerator2[2]*inBPF[2]  +  numerator2[3]*inBPF[3] +  numerator2[4]*inBPF[4] 
               - denominator2[1]*outBPF[1] - denominator2[2]*outBPF[2] - denominator2[3]*outBPF[3] - denominator2[4]*outBPF[4]; 
  for (int i = 3; i >= 0; i--) {
    inBPF[i+1] = inBPF[i]; 
  }
  
  for (int i = 3; i >= 0; i--) {
    outBPF[i+1] = outBPF[i];
  }
  return  outBPF[0];
}

int windowSize = 20;
double MovingAvgArr[100] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
double MovingAvgSum = 0;
// Moving average filter used for averaging 
// the ECG measurement data 
double movingAvgFilter() {
  MovingAvgSum = 0;
  MovingAvgArr[0] = LPF_Data;
  for (int i = 0; i < windowSize - 1; i++) {
    MovingAvgArr[i + 1] = MovingAvgArr[i]; 
    MovingAvgSum+= MovingAvgArr[i];
  }
  MovingAvgSum+= MovingAvgArr[windowSize - 1];
  return MovingAvgSum/windowSize;
}

// lowPassExponential filter used for averaging 
// the ECG measurement data 
double lowPassExponential(float beta, double prevOutput)
{
  return squaring()*beta + (1-beta)*prevOutput;  // ensure factor belongs to  [0,1]
}

double in2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
double out2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
double LPF2() {
  float den[9 + 1] =
{ 1,
  -6.10773776689883,
  16.9198667599856, 
  -27.8202509896190,
  29.8559875972512,
  -21.6508816009811,
  10.5952260482105,
  -3.37023883882295,
  0.631721422712149,
  -0.0531203744321101
  };


float num[9 + 1] =
{
  1.11769024501717e-06,
  1.00592122051546e-05,
  4.02368488206182e-05,
  9.38859805814425e-05,
  0.000140828970872164,
  0.000140828970872164,
  9.38859805814425e-05,
  4.02368488206182e-05,
  1.00592122051546e-05,
  1.11769024501717e-06
};

  in2[0] = (double) adcValueCopy; 
  out2[0] = num[0]*in2[0] +  num[1]*in2[1] +  num[2]*in2[2]
          + num[3]*in2[3] +  num[4]*in2[4] +  num[5]*in2[5] 
          + num[6]*in2[6] + num[7]*in2[7]  + num[8]*in2[8] + num[9]*in2[9] 
          -  den[0]*out2[0] - den[1]*out2[1] - den[2]*out2[2]
          -  den[3]*out2[3] - den[4]*out2[4] - den[5]*out2[5]
          -  den[6]*out2[6] - den[7]*out2[7] - den[8]*out2[8] - den[9]*out2[9];
for (int i = 0; i < 10 - 1; i++) {
  in2[i+1] = in2[i]; 
}
for (int i = 0; i < 10 - 1; i++) {
  out2[i+1] = out2[i];
}
return  out2[0];
}



// butterworth filter used for filtering 
// ECG measurement data 
double in_diff[4] = {0, 0, 0, 0};
double out_diff[4] = {0, 0, 0, 0}; 
double Diff() {
  float num[3 + 1] =
{
 2, 1, -1, -2
};


float denom[3 + 1] =
{
    1, /* z^{0} */
0, 0, 0
};
//in_diff[0] = (double) BPF(LPF());  // NEW FILTER, NOT CALIBRATED, DONT USE
in_diff[0] = (double) LPF(); 
out_diff[0] = num[0]*in_diff[0] +  num[1]*in_diff[1] +  num[2]*in_diff[2]  +  num[3]*in_diff[3] -  denom[1]*out_diff[1] - denom[2]*out_diff[2] - denom[3]*out_diff[3]; 
for (int i = 0; i < 4 - 1; i++) {
  in_diff[i+1] = in_diff[i]; 
}

for (int i = 0; i < 4 - 1; i++) {
  out_diff[i+1] = out_diff[i];
}
return  out_diff[0];
}

double squaring() {
  return (pow(Diff(),2));
}

const int winSize = 100;
double prevWindowVals[winSize] = {0}; 
double movingWindowInt() {
  double winSum = 0;
  prevWindowVals[0] = squaring();
//  prevWindowVals[0] = LPF();
  for (int i = 0; i < winSize; i++) {
    winSum = winSum + prevWindowVals[i];
  }
  winSum = winSum / winSize;

  for (int i = 0; i < winSize-1; i++) {
    prevWindowVals[i+1] = prevWindowVals[i];
    
//    prevWindowVals[i+1] = prevWindowVals[i];
  }
  return winSum;
}

