#include <SPI.h>
#define N_samples 2
#define SENSORS_NUMBER 1
#define MAX_RESOLUTION 32768
#define TEST

int16_t encoder_cycles[SENSORS_NUMBER] = {0};
int16_t encoder_data[SENSORS_NUMBER][N_samples] = {0};
float   encoder_angle[SENSORS_NUMBER] = {0};
const int chipSelectors[SENSORS_NUMBER] = {10}; // {5,6,7,8,9,10};

bool SENDING = false;
bool PEEK    = false;

typedef struct{
  float enc_1;
}pkg_1_t;

pkg_1_t pkg_1 = {0};

void setup() {
  
  Serial.begin(115200);
  SPI.begin();

  for ( int aux = 0; aux < SENSORS_NUMBER; aux++ ) {
    pinMode(chipSelectors[aux], OUTPUT);
    delay(10);
    digitalWrite (chipSelectors[aux], LOW);
  }

  delay(100);

}

void printMat(int16_t vec[][N_samples] , int tam){
  for(int i = 0 ; i < tam ; i++){
    Serial.print(vec[i][0]);
    Serial.print("\t");
  }
  Serial.print("\n");
}

void printVec(int16_t vec[] , int tam){
  for(int i = 0 ; i < tam ; i++){
    Serial.print(vec[i]);
    Serial.print("\t");
  }
  Serial.print("\n");
}

void printMat2(int16_t vec[][N_samples]){
   
   Serial.print("\n");
   for(int i = 0 ; i < SENSORS_NUMBER ; i++){
    Serial.print("\n");
    for(int j = 0 ; j < N_samples ; j++){
      Serial.print(vec[i][j]);
      Serial.print("\t");
    }
   }
      
  }
  


void shiftMat(int16_t vec[][N_samples] , int tam){
  for(int i = 0 ; i < tam ; i++){
   for(int j = (N_samples-1) ; j > 0 ; j--){
      vec[i][j] = vec[i][j-1];
   }
  }
}

void readEncoders(){
   for ( int aux = 0; aux < SENSORS_NUMBER; aux++ )
  {
    SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE1) );
    digitalWrite (chipSelectors[aux], LOW);
    encoder_data[aux][0] = SPI.transfer16(0x00);
    digitalWrite (chipSelectors[aux], HIGH);
    SPI.endTransaction();
  }
}

void countRevs(){
  for ( int aux = 0; aux < SENSORS_NUMBER; aux++ ){
    if( encoder_data[aux][0] > MAX_RESOLUTION/4.0 || encoder_data[aux][0] < -MAX_RESOLUTION/4.0  ){
      if (encoder_data[aux][0] > 0 && encoder_data[aux][1] < 0 )  encoder_cycles[aux]--;
      if (encoder_data[aux][0] < 0 && encoder_data[aux][1] > 0 )   encoder_cycles[aux]++;  
    }
  }
}
void calcAngles(){
  for ( int aux = 0; aux < SENSORS_NUMBER; aux++ ){
    encoder_angle[aux] = encoder_cycles[aux]*360 +  encoder_data[aux][0]*180.0f/MAX_RESOLUTION; //encoder_cycles[0]*360 + 
  }
}

void loop() {

  readEncoders();
  countRevs();
  calcAngles();
  shiftMat(encoder_data,SENSORS_NUMBER);

  if(SENDING || PEEK){
    

    #ifdef TEST
      Serial.print("\n");
      Serial.print(encoder_angle[0]);  
      delay(50);
    #else
      pkg_1.enc_1 = encoder_angle[0];
      Serial.write((byte *)&pkg_1,sizeof(pkg_1));
    #endif
       
    PEEK = false;
  }
  
}

void serialEvent(){
  if(Serial.available()>0){

    switch(Serial.read()){
      case 't':
        SENDING = true;      
      break;
      case 'f':
        SENDING = false;      
      break;
      case 'p':
        PEEK = true;      
      break;
    }

  }
}
