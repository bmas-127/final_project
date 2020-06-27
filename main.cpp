#include "mbed.h"

#include "bbcar.h"

// servo motor
DigitalOut led1(LED1);
PwmOut pin9(D9), pin8(D8);
DigitalInOut pin10(D10);
Ticker servo_ticker;
BBCar car(pin8, pin9, servo_ticker);

// encoder
Serial pc(USBTX, USBRX);
DigitalIn Din_11(D11);
Ticker encoder_ticker;


 int main() {
    int cnt, step;
    parallax_ping  ping1(pin10);
    parallax_encoder encoder( Din_11, encoder_ticker );

    pc.baud(9600);

    car.goStraight(100, 105);

    cnt = 0;
    while(true){
        if((float)ping1 < 20){
            led1 = 0;
            cnt ++;
        }
        else {
            led1 = 1;
        }

        if(cnt == 2) break;

        wait(0.1);
    }

    car.stop();
    wait(1);
    pc.printf("Before start\r\n");
    pc.printf("make a love\r\n");


    car.goStraight(-40, 42);

    encoder.reset();
    while(true){
        step = encoder.get_steps();
        pc.printf("step = %d\r\n", step);
        if(step >= 11) break;
    }

    car.goStraight(100, 105);

 }