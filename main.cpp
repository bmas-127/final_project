#include "mbed.h"
#include "bbcar.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

// servo motor
DigitalOut led1(LED1);
PwmOut pin9(D9), pin8(D8);
DigitalInOut pin10(D10);
Ticker servo_ticker;
BBCar car(pin8, pin9, servo_ticker);

// encoder
DigitalIn Din_11(D11);
Ticker encoder_ticker;

// ping
parallax_ping  ping1(pin10);
parallax_encoder encoder( Din_11, encoder_ticker );

// XBee
void background_Acc();
RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;


void GoStraight(int speed_left, int speed_right, int dist, bool direction);
void TurnDirection(int speed_left, int speed_right, int step);

void xbee_setting();
void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);

int main() {
    xbee_setting();

    pc.baud(9600);

    car.stop();
    wait(1);

    // xbee
    xbee_setting();
    background_Acc();

    // go straight first road
    GoStraight(100, 107, 25, 1);

    // turn left
    TurnDirection(30, 200, 27);

    // go straight to parking lot
    GoStraight(100, 107, 15, 1);

    // parking
    TurnDirection(-20, -207, 23);
    GoStraight(-100, -107, 40, 0);

    // leaving parking lot and taking photo
    GoStraight(100, 107, 25, 1);
    TurnDirection(150, -150, 12);
    car.goStraight(100, 102);
    wait(1);
    TurnDirection(30, 200, 27);



 
}


void GoStraight(int speed_left, int speed_right, int dist, bool direction){
    int cnt = 0;

    car.stop();
    wait(0.5);

    if(direction)
        car.goStraight(100, 255);
    else
        car.goStraight(-100, -255);
    
    
    wait(0.05);
    car.goStraight(speed_left, speed_right);

    while(true){
        if(direction){
            if((float)ping1 < dist){
                led1 = 0;
                cnt ++;
            }
            else {
                led1 = 1;
            }
        }else{
            wait(2);
            cnt = 2;
            car.stop();
        }

        if(cnt == 2) break;

        wait(0.1);
    }
}

void TurnDirection(int speed_left, int speed_right, int num){
    car.stop();
    wait(0.5);

    car.goStraight(speed_left, speed_right);
    

    encoder.reset();
    while(true){
        int step = encoder.get_steps();
        pc.printf("step = %d\r\n", step);
        if(step >= num) break;
    }

    car.stop();
    wait(1);
}

void background_Acc(){
  for(int i = 0; i < 10; i ++){
    xbee.printf("suck my dick");
  }
}

void xbee_setting(){
    char xbee_reply[4];

    // XBee setting
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY 0x209\r\n");
    reply_messange(xbee_reply, "setting MY : 0x209");

    xbee.printf("ATDL 0x109\r\n");
    reply_messange(xbee_reply, "setting DL : 0x109");

    xbee.printf("ATID 0x1\r\n");
    reply_messange(xbee_reply, "setting PAN ID : 0x1");

    xbee.printf("ATWR\r\n");
    reply_messange(xbee_reply, "write config");

    xbee.printf("ATMY\r\n");
    check_addr(xbee_reply, "MY");

    xbee.printf("ATDL\r\n");
    check_addr(xbee_reply, "DL");

    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");
    xbee.getc();

    // start

  pc.printf("start\r\n");

  t.start(callback(&queue, &EventQueue::dispatch_forever));


  // Setup a serial interrupt function to receive data from xbee

  xbee.attach(xbee_rx_interrupt, Serial::RxIrq);
}

void xbee_rx_interrupt(void)

{

  xbee.attach(NULL, Serial::RxIrq); // detach interrupt

  queue.call(&xbee_rx);

}


void xbee_rx(void)

{

  static int i = 0;

  static char buf[100] = {0};

  while(xbee.readable()){

    char c = xbee.getc();

    if(c!='\r' && c!='\n'){

      buf[i] = c;

      i++;

      buf[i] = '\0';

    }else{

      i = 0;

      pc.printf("Get: %s\r\n", buf);

      xbee.printf("%s", buf);

    }

  }

  wait(0.1);

  xbee.attach(xbee_rx_interrupt, Serial::RxIrq); // reattach interrupt

}


void reply_messange(char *xbee_reply, char *messange){

  xbee_reply[0] = xbee.getc();

  xbee_reply[1] = xbee.getc();

  xbee_reply[2] = xbee.getc();

  if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){

   pc.printf("%s\r\n", messange);

   xbee_reply[0] = '\0';

   xbee_reply[1] = '\0';

   xbee_reply[2] = '\0';

  }

}


void check_addr(char *xbee_reply, char *messenger){

  xbee_reply[0] = xbee.getc();

  xbee_reply[1] = xbee.getc();

  xbee_reply[2] = xbee.getc();

  xbee_reply[3] = xbee.getc();

  pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);

  xbee_reply[0] = '\0';

  xbee_reply[1] = '\0';

  xbee_reply[2] = '\0';

  xbee_reply[3] = '\0';

}