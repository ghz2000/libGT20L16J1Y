/***********
***********/

#ifndef _libGT20L_
#define _libGT20L_

#include <Arduino.h>



class CGT20L{
private:
  uint8_t matrixdata32[32]; //16×16用表示データ
  uint8_t matrixdata16[16]; //16×8用表示データ
  uint8_t m_SS;
  uint8_t m_MISO;
  uint8_t m_MOSI;
  uint8_t m_CLK;
  uint8_t *m_strings;

  // アドレス 0x828F:全角小文字o は大きさと位置が違うため自前で用意
  // アドレス 0x3FCD0: サンセリフ半角小文字oを利用するのも、大きさが1dot大きいので同上
  const unsigned char matrixdata32_o[32] = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11000000,
    B00100000,
    B00010000,
    B00010000,
    B00010000,
    B00100000,
    B11000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000011,
    B00000100,
    B00001000,
    B00001000,
    B00001000,
    B00000100,
    B00000011,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
  };
public:


private:
public:
  CGT20L();
  void init(int CLK, int MOSI, int MISO, int SS);

  void getSJIS1byte(uint8_t code, uint8_t tmp[]);
  void getSJIS2byte(unsigned short code, uint8_t tmp[]);
  void getData(uint32_t Addr, uint8_t returndata[], uint8_t bytes);
  void convertJIS(unsigned short code, uint8_t *c1, uint8_t *c2);
  void spiSendData(uint32_t Addr);
  void copyMatrix16(const uint8_t *tmp, int bytes);
  void copyMatrix32(const uint8_t *tmp, int bytes);
  void setStrings(uint8_t *str);
  int getMatrixData(uint8_t *data, int num);
  
/*1byteのSJISを表示する*/
/*2byteのSJISを表示する*/
/*漢字ROMとやりとり*/
uint32_t readFontJIS(uint8_t c1, uint8_t c2);
/*漢字ROMとやりとり*/
uint32_t readFontASCII(uint8_t asciiCode);
/*シリアルモニタへ16*16のデータを表示する*/
void showDotsToSerial32();
/*シリアルモニタへ16*8のデータを表示する*/
void showDotsToSerial16();

};

#endif

