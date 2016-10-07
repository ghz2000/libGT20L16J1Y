#include "libGT20L16J1Y.h"
#include <SPI.h>

#define GT20L16ADDRESS 0x03

#define DEBUGA

#ifdef DEBUG
#define debug(...) {Serial.println(__VA_ARGS__);}
#define debugL(...) {Serial.println( __func__); Serial.println( __LINE__); Serial.println(__VA_ARGS__);}
#else
#define debug
#endif


CGT20L::CGT20L(){
}

void CGT20L::init(int CLK, int MOSI, int MISO, int SS){
  m_CLK  = CLK;
  m_MOSI = MOSI;
  m_MISO = MISO;
  m_SS   = SS;  

  pinMode (m_SS, OUTPUT);
  SPI.begin ();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
}

// 漢字ROMと通信する
void CGT20L::getData(uint32_t Addr, uint8_t returndata[], uint8_t bytes){ 
//  Serial.print("Address = "); Serial.println(Addr, HEX);
  digitalWrite(m_SS, LOW);  //通信開始

  // 漢字ROMにデータを送信
  SPI.transfer(GT20L16ADDRESS);
  SPI.transfer(Addr >> 16  & 0xff);
  SPI.transfer(Addr >> 8   & 0xff);
  SPI.transfer(Addr        & 0xff);  
  
  // 漢字ROMからデータを受信
  for (int i = 0; i < bytes; i++){
    returndata[i] = SPI.transfer(0x00);
  }
  digitalWrite(m_SS, HIGH); //通信終了
}


// 1byteのSJISを取り出す
void CGT20L::getSJIS1byte(uint8_t code, uint8_t tmp[]){
  uint32_t address = 0;

  address = readFontASCII(code);  //ASCIIコードから漢字ROMのアドレスを引く
  getData(address, tmp, 16);        //漢字ROMからバイナリを取得

#ifdef DEBUG
  Serial.println("---start---");
  for(int i=0; i<16; i++){
    Serial.println(tmp[i], BIN);
  }
  Serial.println("---end---");
#endif

}

// 2byteのSJISを取り出す
//showSJIS2byte(SJIS文字コード)
void CGT20L::getSJIS2byte(unsigned short code, uint8_t tmp[]){ 
  uint32_t addr = 0;
  
  if(code == 0x828F){
    memcpy(tmp, matrixdata32_o, sizeof(matrixdata32_o));
    debug("kotti kita0- ");
  }else{
    uint8_t c1, c2;
    convertJIS(code, &c1, &c2);

    // 読み出し
    addr = readFontJIS(c1, c2);
    getData(addr, tmp, 32);
  }

#ifdef DEBUG
  Serial.println("---start---");
  for(int i=0; i<16; i++){
    Serial.println(tmp[i], BIN);
    Serial.println(tmp[i+16], BIN);
  }
  Serial.println("---end---");
#endif DEBUG
}

void CGT20L::setStrings(uint8_t *str){
  m_strings = str;
}

void CGT20L::copyMatrix16(const uint8_t *tmp, int bytes){
   memcpy(matrixdata16, tmp, bytes);
}

void CGT20L::copyMatrix32(const uint8_t *tmp, int bytes){
   memcpy(matrixdata32, tmp, bytes);
}

// 欲しい文字を取り出す
int CGT20L::getMatrixData(uint8_t *data, int num){
  int j=0;
  for(int i=0; m_strings[i]!=NULL; i++){
    j++;
    if(j == num){ // 欲しい文字だったので取り出す
        if ( (m_strings[i] < 0x80) || ((0xA0 < m_strings[i]) && (m_strings[i] <= 0xdF)) ) {
        getSJIS1byte(m_strings[i], data);
#ifdef DEBUG
     copyMatrix16(data, sizeof(matrixdata16)); //表示するため
     showDotsToSerial16();                     //表示する
#endif
        return 16;
      } else {
        uint16_t code =  ((m_strings[i] << 8) + m_strings[++i]);
        uint8_t tmp[32];
        getSJIS2byte(code, data);
#ifdef DEBUG
     copyMatrix32(data, sizeof(matrixdata32)); //表示するため
     showDotsToSerial32();                     //表示する
#endif
        return 32;
      }
    }

    // 欲しい文字じゃなかったら飛ばす
    if ( (m_strings[i] < 0x80) || ((0xA0 < m_strings[i]) && (m_strings[i] <= 0xdF)) ) { } else i++;
  }
  return 0;
}

// SJIS->JISx0208変換をする
void CGT20L::convertJIS(unsigned short code, uint8_t *c1, uint8_t *c2){
    debug("SJIS, 0x"); debug(code, HEX); debug("\t");
    *c1 = ((code & 0xff00) >> 8);
    *c2 = (code & 0xFF);
    if (*c1 >= 0xe0){
      *c1 = *c1 - 0x40;
    }
    if (*c2 >= 0x80){
      *c2 = *c2 - 1;
    }
    if (*c2 >= 0x9e){
      *c1 = (*c1 - 0x70) * 2;
      *c2 = *c2 - 0x7d;
    } else {
      *c1 = ((*c1 - 0x70) * 2) - 1;
      *c2 = *c2 - 0x1f;
    }
}

// 漢字ROM表との位置合わせ
//readFontJIS(JIS上位8bit,JIS下位8bit);
uint32_t CGT20L::readFontJIS(uint8_t c1, uint8_t c2)
{
  // jisx変換後の表示
//  Serial.print("jisx up8 = 0x"); Serial.print(c1, HEX); Serial.print("\t");
//  Serial.print("jisx down8 = 0x"); Serial.print(c2, HEX); Serial.print("\t");
  // jisxの区点を求める
  uint32_t MSB = c1 - 0x20;//区
  uint32_t LSB = c2 - 0x20;//点
  // JISの句点番号で分類
  uint32_t Address = 0;
//  Serial.print("MSB = d"); Serial.print(MSB, DEC); Serial.print("\t");
//  Serial.print("LSB = d"); Serial.print(LSB, DEC); Serial.print("\t");
  // 各種記号・英数字・かな(一部機種依存につき注意,㍍などWindowsと互換性なし)
  if (MSB >= 1 && MSB <= 15 && LSB >= 1 && LSB <= 94){
    Address = ( (MSB - 1) * 94 + (LSB - 1)) * 32;
  }
  // 第一水準
  if (MSB >= 16 && MSB <= 47 && LSB >= 1 && LSB <= 94){
    Address = ( (MSB - 16) * 94 + (LSB - 1)) * 32 + 43584;
  }
  // 第二水準
  if (MSB >= 48 && MSB <= 84 && LSB >= 1 && LSB <= 94){
    Address = ((MSB - 48) * 94 + (LSB - 1)) * 32 + 138464;
  }
  //
  // GT20L16J1Y内部では1区と同等の内容が収録されている
  if (MSB == 85 && LSB >= 0x01 && LSB <= 94){
    Address = ((MSB - 85) * 94 + (LSB - 1)) * 32 + 246944;
  }
  // GT20L16J1Y内部では2区、3区と同等の内容が収録されている
  if (MSB >= 88 && MSB <= 89 && LSB >= 1 && LSB <= 94){
    Address = ((MSB - 88) * 94 + (LSB - 1)) * 32 + 249952;
  }

  return Address;

}//spireadfont


// 漢字ROMとやりとり
//readFontASCII(ASCIIコード);
uint32_t CGT20L::readFontASCII(uint8_t asciiCode){
//  Serial.print("ASCII,0x");Serial.print(asciiCode, HEX);
  uint32_t Address = 0;
  // ASCII文字
  if (asciiCode >= 0x20 && asciiCode <= 0x7F){
    Address = ( asciiCode - 0x20) * 16 + 255968;
  }
  return Address;
}

// シリアルモニタへ16*16のデータを表示する
void CGT20L::showDotsToSerial32(){
  // 上半分
  for (int i = 0; i < 8; i++){
    for (int b = 0; b < 16; b++){
      char byteDigit = (1 << i);
      if (matrixdata32[b] & byteDigit){
        Serial.write("XX");
      } else {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  // 下半分
  for (int i = 0; i < 8; i++){
    for (int b = 16; b < 32 ; b++){
      char byteDigit = (1 << i);
      if (matrixdata32[b] & byteDigit){
        Serial.write("XX");
      } else {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  Serial.println();
}//sendDataToSerial16


// シリアルモニタへ16*8のデータを表示する
void CGT20L::showDotsToSerial16(){
  // 上半分
  for (int i = 0; i < 8; i++){
    for (int b = 0; b < 8; b++){
      char byteDigit = (1 << i);
      if (matrixdata16[b] & byteDigit){
        Serial.write("XX");
      } else {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  // 下半分
  for (int i = 0; i < 8; i++){
    for (int b = 8; b < 16; b++){
      char byteDigit = (1 << i);
      if (matrixdata16[b] & byteDigit){
        Serial.write("XX");
      } else {
        Serial.write("--");
      }
    }
    Serial.println();
  }
  Serial.println();
}//sendDataToSerial32

