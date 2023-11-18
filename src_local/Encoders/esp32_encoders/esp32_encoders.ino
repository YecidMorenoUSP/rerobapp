#include "/home/yecid/rerobapp/src_local/local_libs/Utils_Encoders/abs_encoder.h"

#define SerialOUT Serial
#define SZ_SEND_BUFF 10
Abs_encoder* eR;
Abs_encoder* eL;

static float angles[2] = {0,0};

char pRead;
char send_buff[SZ_SEND_BUFF];

void setup() {
  // put your setup code here, to run once:

  SerialOUT.begin(115200);
  SerialOUT.flush();
  SerialOUT.begin(921600);

  spi_setup();

  eR = new Abs_encoder(GPIO_NUM_27, SPI_CLOCK);
  eR->begin();
  eR->read_angle();
  eR->setZero();

  eL = new Abs_encoder(GPIO_NUM_15, SPI_CLOCK);
  eL->begin();
  eL->read_angle();
  eL->setZero();

  SerialOUT.onReceive(SerialOUT_callback);

  memset(send_buff, '@', SZ_SEND_BUFF);
  send_buff[9] = '\n';
}



void SerialOUT_callback(void) {
  pRead = SerialOUT.read();
  if (pRead == 'p') {

    memcpy(send_buff+1,angles, 8);
    
    SerialOUT.write(send_buff,SZ_SEND_BUFF);
  }else if (pRead == 'r') {
    eR->read_angle();
    eR->setZero();
    eL->read_angle();
    eL->setZero();
  }
}


void loop() {

  eR->read_angle();
  eL->read_angle();

  if(eR->getAngle() !=  0 || true ){
      angles[0] = eR->getAngle();
      angles[1] = eL->getAngle();
  }

  delay(1);
}
