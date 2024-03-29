/**************************************************************
WinFilter version 0.8
http://www.winfilter.20m.com
akundert@hotmail.com

Filter type: Low Pass
Filter model: Chebyshev
Filter order: 4
Sampling Frequency: 10 KHz
Cut Frequency: 0.010000 KHz
Pass band Ripple: 1.000000 dB
Coefficents Quantization: float

Z domain Zeros
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000
z = -1.000000 + j 0.000000

Z domain Poles
z = 0.997427 + j -0.002483
z = 0.997427 + j 0.002483
z = 0.999560 + j -0.006324
z = 0.999560 + j 0.006324
***************************************************************/
#define NCoef 4
float iir(float NewSample) {
    float ACoef[NCoef+1] = {
        0.00000000005929847418,
        0.00000000023719389671,
        0.00000000035579084507,
        0.00000000023719389671,
        0.00000000005929847418
    };

    float BCoef[NCoef+1] = {
        1.00000000000000000000,
        -3.99397387738456230000,
        5.98197913518426280000,
        -3.98203645370192310000,
        0.99403119633051884000
    };

    static float y[NCoef+1]; //output samples
    static float x[NCoef+1]; //input samples
    int n;

    //shift the old samples
    for(n=NCoef; n>0; n--) {
       x[n] = x[n-1];
       y[n] = y[n-1];
    }

    //Calculate the new output
    x[0] = NewSample;
    y[0] = ACoef[0] * x[0];
    for(n=1; n<=NCoef; n++)
        y[0] += ACoef[n] * x[n] - BCoef[n] * y[n];
    
    return y[0];
}
