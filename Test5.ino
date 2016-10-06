/* 内蔵の文字を表示する
 *
 * memo:9/30 00:00
 * memo:10/7 18:30
 * memo:2016/07/26 07:30
 */

#include <SPI.h>
#include "libGT20L16J1Y.h"
#include "SJIS.h"

#define CLK 13
#define MISO 12
#define MOSI 11
#define SS 8


CGT20L GT20L;

void setup (void){

  GT20L.init(CLK, MOSI, MISO, 8);
  
  Serial.begin(115200);
  Serial.println("input word which you want to show");
  Serial.print("port:");
  Serial.print(SS);
  Serial.println("has set as chip select pin");

  setData();
}


void loop (void){

  uint8_t *sendData = nekomata;

  GT20L.setStrings(nekomata);
  
  for(int i=0; sendData[i]!=NULL ; i++){
    if ( (sendData[i] < 0x80) || ((0xA0 < sendData[i]) && (sendData[i] <= 0xdF)) ) {
      uint8_t tmp[16];
      GT20L.getSJIS1byte(sendData[i], tmp);
      GT20L.copyMatrix16(tmp, sizeof(tmp));
      GT20L.showDotsToSerial16();
    } else {
      uint16_t data =  ((sendData[i] << 8) + sendData[++i]);
      uint8_t tmp[32];
      GT20L.getSJIS2byte(data, tmp);
      GT20L.copyMatrix32(tmp, sizeof(tmp));
      GT20L.showDotsToSerial32();
    }
  }

  Serial.println("これですね");
  uint8_t tmp[32];
  GT20L.getMatrixData(tmp, 2);

  

  delay(1000);
  
  if (Serial.available() > 0){
    /*Arduinoのシリアルモニタから送られてくる日本語がSJISであることに注意*/
    delayMicroseconds(200);//最低でも2文字受信したい
    uint8_t msbdata = Serial.read();//1バイト目 //10/07
    /*SJISの1バイトコードか否か*/
    if ( (msbdata < 0x80) || ((0xA0 < msbdata) && (msbdata <= 0xdF)) ) {
      uint8_t tmp[16];
      GT20L.getSJIS1byte(msbdata, tmp);
    } else {
      uint8_t lsbdata = Serial.read();//2バイト目 //10/07
      uint16_t data =  ((msbdata << 8) + lsbdata); //2
      uint8_t tmp[32];
      GT20L.getSJIS2byte(data, tmp);
    }
  }
}


