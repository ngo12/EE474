float numerator[3 + 1] =
{
    0.002040279141687, /* z^{0} */
    0.006120837425060, /* z^{-1} */
    0.006120837425060, /* z^{-2} */
    0.002040279141687, /* z^{-3} */
};


float denominator[3 + 1] =
{
    1, /* z^{0} */
    -2.448746826101290, /* z^{-1} */
    2.039304153327110, /* z^{-2} */
    -0.574235094092326, /* z^{-3} */
};

// butterworth filter
double inSignal[4] = {0, 0, 0, 0};
double outSignal[4] = {0, 0, 0, 0}; 
double LPF() {
inSignal[0] = (double) adcValueCopy; 
outSignal[0] = numerator[0]*inSignal[0] +  numerator[1]*inSignal[1] +  numerator[2]*inSignal[2]  +  numerator[3]*inSignal[3] -  denominator[1]*outSignal[1] - denominator[2]*outSignal[2] - denominator[3]*outSignal[3]; 
for (int i = 0; i < 4 - 1; i++) {
  inSignal[i+1] = inSignal[i]; 
}

for (int i = 0; i < 4 - 1; i++) {
  outSignal[i+1] = outSignal[i];
}
return  outSignal[0];
}
int windowSize = 20;
double MovingAvgArr[100] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
double MovingAvgSum = 0;
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

double lowPassExponential()
{
    return adcValueCopy*LPF_Beta + (1-LPF_Beta)*LPF_Data;  // ensure factor belongs to  [0,1]
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



// butterworth filter
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



