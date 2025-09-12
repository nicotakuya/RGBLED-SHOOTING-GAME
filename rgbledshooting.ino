// SHOOTING GAME for Arduino pro mini(ATmega328 5V/16MHz)
// by takuya matsubara

// Adafruit 16x32 RGB LED matrix

#include <avr/pgmspace.h>
#include <MsTimer2.h>
#include "sfont.h"      // small font

#define COL_WHITE 0b111111
#define COL_RED 0b110000

#define PINR1  2
#define PING1  3
#define PINB1  4
#define PINR2  5
#define PING2  6
#define PINB2  7
#define PINA  14
#define PINB  15
#define PINC  16
#define PINLAT  17
#define PINCLK 8    
#define PINOE  9

#define DEPTH 2

volatile unsigned char vram[32][16];
volatile unsigned char vram_bak[32][16];
volatile int linenum = 0;
volatile int depthcnt = 0;

const int buttonup    = 10;    // 端子番号 joystick 上
const int buttondown  = 13;    // 端子番号 joystick 下
const int buttonleft  = 12;    // 端子番号 joystick 左
const int buttonright = 11;    // 端子番号 joystick 右
const int buttona     = 18;    // 端子番号 button A
const int buttonb     = 19;    // 端子番号 button B

#define CHARAMAX 12 //最大同時表示数
#define NONE -99

unsigned int cnt = 0;
char tx[CHARAMAX];    // キャラクタX座標
char ty[CHARAMAX];    // キャラクタY座標
char tc[CHARAMAX];    // キャラクタ番号
char tt[CHARAMAX];    // キャラクタ時間
unsigned char bgcnt;
int mapcnt;
char seq=0;
unsigned char gameover;

unsigned long vrambg[16]; // VRAM BGバッファ
#define CHRTYPEMAX 6  // 敵の種類
#define CHRMY      0  // 自機
#define CHRTYPE1   1  // 敵1
#define CHRTYPE2   2  // 敵2
#define CHRTYPE3   3  // 敵3
#define CHRTYPE4   4  // 敵4
#define CHRTYPE5   5  // 敵5
#define CHRTYPE6   6  // 敵6
#define CHRTYPEEXP 7  // 爆発
#define CHRLAZER   11 // LAZER

//背景データ
PROGMEM const unsigned int bgmap[] = {
  0b1000000000000000,
  0b1100000000000000,
  0b1110000000000000,
  0b1111000000000000,
  0b1111100000000000,
  0b1111100000000000,
  0b1111100000000000,
  0b1111000000000000,
  0b1110000000000000,
  0b1110000000000000,
  0b1110000000000000,
  0b1100000000000001,
  0b1000000000000001,
  0b1000000000000001,
  0b1100000000000001,
  0b1110000000000001,
  0b1100000000000000,
  0b1100000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b1000000000000000,
  0b1000000000000001,
  0b1100000000000011,
  0b1100000000000011,
  0b1000000000000011,
  0b1000000000000001,
  0b1000000000000000,
  0b1000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b1000000000000000,
  0b1000000000000000,
  0b1000000000000000,
  0b1100000000000000,
  0b1000000000000001,
  0b1100000000000011,
  0b1100000000000001,
  0b1110000000000000,
  0b1100000000000000,
  0b1000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b1000000000000000,
  0b1100000000000111,
  0b1000000000001111,
  0b0000000000111111,
  0b0000000001111111,
  0b0000000001111111,
  0b0000000000111111,
  0b0000000000011111,
  0b0000000000000111,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000011,
  0b0000000000000011,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b1100000000000000,
  0b1100000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000000000000000,
  0b0000011110000000,
  0b0000011110000000,
  0b0000011110000000,
};

//キャラクタデータ
PROGMEM const unsigned char spchr[12][3][3] = {
  {
    {5,0,0},  /*自キャラ*/
    {5,1,5},
    {5,0,0}
  },{
    {0,2,0},  /*敵1*/
    {3,0,3},
    {0,2,0}
  },{
    {0,0,0},  /*敵2*/
    {2,5,5},
    {0,0,0}
  },{
    {0,3,3},  /*敵3*/
    {3,2,0},
    {0,3,3}
  },{
    {6,4,6},  /*敵4*/
    {6,0,6},
    {6,0,6}
  },{
    {0,5,0},  /*敵5*/
    {0,5,0},
    {6,5,6}
  },{
    {3,3,3},  /*敵6*/
    {0,7,0},
    {3,3,3}
  },{
    {0,2,0},  /*7爆発1*/
    {2,6,2},
    {0,2,0}
  },{
    {2,2,2},  /*8爆発2*/
    {2,2,2},
    {2,2,2}
  },{
    {2,2,2},  /*9爆発3*/
    {2,0,2},
    {2,2,2}
  },{
    {2,0,2},  /*10爆発4*/
    {0,0,0},
    {2,0,2}
  },{
    {0,0,0},  /*11弾*/
    {7,7,6},
    {0,0,0}
  }
};

unsigned int ctable[8];    //カラーテーブル

//---------------------------------------------------------------------
void rgbled_init(void){
  pinMode(PINR1, OUTPUT);
  pinMode(PING1, OUTPUT);
  pinMode(PINB1, OUTPUT);
  pinMode(PINR2, OUTPUT);
  pinMode(PING2, OUTPUT);
  pinMode(PINB2, OUTPUT);
  pinMode(PINA, OUTPUT);
  pinMode(PINB, OUTPUT);
  pinMode(PINC, OUTPUT);
  pinMode(PINCLK, OUTPUT);
  pinMode(PINLAT, OUTPUT);
  pinMode(PINOE, OUTPUT);

  digitalWrite(PINOE, HIGH);  //DISABLE
  digitalWrite(PINCLK, HIGH);
}

//---------------------------------------------------------------------
void rgbled_driver(void){
  int threshold;
  unsigned char c,r,g,b;
  int x;
  digitalWrite(PINOE, HIGH);  //DISABLE

  digitalWrite(PINA, linenum & 1);  //line select
  digitalWrite(PINB, (linenum >> 1) & 1); //line select
  digitalWrite(PINC, (linenum >> 2) & 1); //line select

  digitalWrite(PINCLK, HIGH);
  digitalWrite(PINLAT, HIGH); //LATCH ON

  threshold = depthcnt;
//  threshold = 0;

  for(x=0;x<32;x++){
    c = vram[x][linenum];
    b = c & 0x3;
    g = (c>>2)& 0x3;
    r = (c>>4)& 0x3;
    digitalWrite(PINR1,(r > threshold));
    digitalWrite(PING1,(g > threshold));
    digitalWrite(PINB1,(b > threshold));

    c = vram[x][linenum+8];
    b = c & 0x3;
    g = (c>>2)& 0x3;
    r = (c>>4)& 0x3;
    digitalWrite(PINR2,(r > threshold));
    digitalWrite(PING2,(g > threshold));
    digitalWrite(PINB2,(b > threshold));
    digitalWrite(PINCLK, LOW);
    digitalWrite(PINCLK, HIGH);
  }

  digitalWrite(PINLAT, LOW);  //LATCH OFF
  digitalWrite(PINOE, LOW);  //ENABLE

  linenum = (linenum+1) % 8;
  if(linenum==0)depthcnt=(depthcnt+1) % DEPTH;
}

//---------------------------------------------------------------------
// 背景初期化
void bginit(void){
  int i;

  bgcnt=0;
  mapcnt=0;
  for(i=0; i<16; i++){
    vrambg[i] = 0;
  }
}

//---------------------------------------------------------------------
// キャラクタ初期化
void charainit(void){
  int i;

  for(i=0; i<CHARAMAX; i++){
    tx[i] = NONE;
    ty[i] = NONE;
    tc[i] = 0;
    tt[i] = 0;
  }
}

//---------------------------------------------------------------------
// キャラクタ新規配置
// 引数 x座標,y座標,種類
void charamake(char x,char y,char c){
  int i;

  for(i=0;i<CHARAMAX;i++){
    if(tx[i]==NONE){
      tx[i] = x;
      ty[i] = y;
      tc[i] = c;
      tt[i] = 0;
      break;
    }
  }
}


//---------------------------------------------------------------------
// 初期設定
void setup() {
  int i,r,g,b;

  Serial.begin(115200);  //baudrate
  while(!Serial);
  
  rgbled_init();

  pinMode(buttonup, INPUT_PULLUP);
  pinMode(buttondown, INPUT_PULLUP);
  pinMode(buttonleft, INPUT_PULLUP);
  pinMode(buttonright, INPUT_PULLUP);
  pinMode(buttona, INPUT_PULLUP);
  pinMode(buttonb, INPUT_PULLUP);
  for(i=0;i<8;i++){
    r = ((i>>1) & 1)*3;
    g = ((i>>2) & 1)*3;
    b = (i & 1)*3;
    ctable[i] = (r<<4) + (g<<2) + b;    //カラーテーブル
  }

  MsTimer2::set(2,rgbled_driver);
  MsTimer2::start();
}

//---------------------------------------------------------------------
void vram_pset(int x,int y,unsigned char c)
{
  if(x<0)return;
  if(x>31)return;
  if(y<0)return;
  if(y>15)return;
  
  vram_bak[x][y] =c;  
}

//---------------------------------------------------------------------
void vram_clear(void){
  int x,y;
  for(y=0; y<16; y++){
    for(x=0; x<32; x++){
      vram_bak[x][y] = 0;      
    }
  }
}

//---------------------------------------------------------------------
void vram_update(void){
  int x,y;
  for(y=0;y<16;y++){
    for(x=0;x<32;x++){
      vram[x][y] = vram_bak[x][y];      
    }
  }
}

//---------------------------------------------------------------------
// 1キャラクタをVRAM転送
// 引数 x,y,chキャラクターコード
void vram_putch(char textx, char texty, unsigned char ch,unsigned char color)
{
  unsigned char i,j;
  unsigned char bitdata;
  PGM_P p;

  if(ch < 0x20)return;
  p = (PGM_P)smallfont;
  p += ((int)(ch - 0x20) * 3);

  for(i=0 ;i<6 ;i++) {
    bitdata = pgm_read_byte(p);
    if((i % 2)==0){
      bitdata >>= 4;
    }else{
      p++;
    }
    bitdata &= 0xf;
    for(j=0; j<4; j++){
      if(bitdata & (0b1000>>j)){
        vram_pset(textx+j, texty+i, color);
      }
    }
  }
}

//---------------------------------------------------------------------
// print string
void vram_putstr(char textx, char texty,unsigned char *p,unsigned char color)
{
  while(*p != 0){
    vram_putch( textx,texty,*p++,color);
    textx += 4;
  }
}

//---------------------------------------------------------------------
// print number
void vram_putnum(char textx, char texty,char num)
{
  vram_putch( textx,  texty,'0'+(num / 10),COL_WHITE);
  textx += 4;
  vram_putch( textx,  texty,'0'+(num % 10),COL_WHITE);
  textx += 4;
}

//---------------------------------------------------------------------
// 16進数表示
void vram_puthex(char textx, char texty,unsigned char x)
{
  unsigned char ch;
  char shift=4;

  while(shift >= 0){
    ch = (x >> shift) & 0xf;
    if(ch < 10){
      ch += '0';
    }else{
      ch += ('A'-10);
    }
    vram_putch( textx,  texty,ch,COL_WHITE);
    textx +=4;
    shift -= 4;
  }
}

//--------------------------------------------------------------------
// 絶対値
char fnc_abs(char a)
{
  if(a < 0){
    return(-a);
  }else{
    return(a);
  }
}

//---------------------------------------------------------------------
// 3x3pixelのキャラクターを表示
//引数 X座標, Y座標 ,パターンnum ,color
void vram_spput(char x,char y,unsigned char chrnum)
{
  PGM_P p;
  int i,j;
  unsigned char color;

  if((x < -1)||(x > 32)||(y < -1)||(y > 16)) return;
  p = (PGM_P) &spchr[chrnum][0][0];
  x -= 1;
  y -= 1;
  for(j=0; j<3; j++){
    for(i=0; i<3; i++){
      color = pgm_read_byte(p);
      vram_pset(x+i, y, ctable[color]);    //ドット描画
      p++;
    }
    y++;
  }
}

//---------------------------------------------------------------------
// 背景処理
void bgctrl(void){
  #define SCROLLSPEED 15
  int i,j,c;
  unsigned int temp;

  for(i=0; i<16; i++){
    for(j=0; j<32; j++){
      c=0;
      if(vrambg[i] & (0x80000000 >> j)) c=ctable[4];
      vram_pset(j, i, c);    //ドット描画
    }
  }

  bgcnt = (bgcnt+1) % SCROLLSPEED;
  if(bgcnt != 0)return;

  for(i=0; i<16; i++){
    vrambg[i] <<= 1;    //背景左スクロール
  }

  mapcnt++;
  if(mapcnt > (sizeof(bgmap) / 2)){
    mapcnt=0;
  }

  temp = pgm_read_word((PGM_P)(bgmap+mapcnt));
  for(i=0; i<16; i++){
    if(temp & 0x1){
      vrambg[i] |= 1;    //背景継ぎ足し
    }
    temp >>= 1;
  }

  if(random(10) > 5){    //敵出現
    charamake(32,random(13)+1,random(CHRTYPEMAX)+1);
  }
}

//---------------------------------------------------------------------
// 当たり判定
char sphitenemy(char x,char y){
  int i;

  for(i=0; i<CHARAMAX; i++){
    if(tx[i]==NONE)continue;
    if(tc[i]<1 || tc[i]>6)continue;
    if(fnc_abs(x - tx[i]) < 2){
      if(fnc_abs(y - ty[i]) < 2){
        return(i);  //衝突
      }
    }
  }
  return(-1);
}

//---------------------------------------------------------------------
// キャラクタ処理
void charactrl(void){
  char x,y;
  char color;
  char hit;
  int i;

  for(i=0; i<CHARAMAX; i++){
    if(tx[i] == NONE) continue;
    x = tx[i];
    y = ty[i];

    switch(tc[i]) {
    case CHRMY:        //自機
      if (digitalRead(buttonup) == LOW) y--;
      if (digitalRead(buttondown) == LOW) y++;
      if (digitalRead(buttonleft) == LOW) x--;
      if (digitalRead(buttonright) == LOW) x++;
      if (x<1) x=1;
      if (y<1) y=1;
      if (x>30) x=30;
      if (y>14) y=14;
      if(tt[i]>0){  //disable shot
        tt[i]--;
      }else{
        if (digitalRead(buttona) == LOW){
          charamake(x,y,CHRLAZER);  //弾発射
          tt[i] = 15;
        }
      }
      hit = sphitenemy(x,y);    //当たり判定
      if(vrambg[y] & (0x80000000>>x))hit = 1;
      if(hit>=0){
        if(gameover==0)gameover=1;
        tc[i]=CHRTYPEEXP;
        charamake(x-2,y-2,CHRTYPEEXP); //爆発
        charamake(x-2,y+2,CHRTYPEEXP); //爆発
        charamake(x+2,y-2,CHRTYPEEXP); //爆発
        charamake(x+2,y+2,CHRTYPEEXP); //爆発
      }
      break;

    case CHRTYPE1:    //敵1
      if(tt[i]==0){
        x--;
        if(x<3) tt[i]++;
      }else{
        x++;
      }
      break;

    case CHRTYPE2:    //敵2
      if(tt[i] < 15){
        x--;
      }else{
        x-=2;
      }
      break;

    case CHRTYPE3:    //敵3
      tt[i] = (tt[i]+1)% 16;
      if(tt[i] < 4){
        x--;
      }else if(tt[i] < 8){
        y++;
      }else if(tt[i] < 12){
        x--;
      }else{
        y--;
      }
      break;

    case CHRTYPE4:    //敵4
      tt[i]++;
      if(tt[i] >= 8){
        tt[i] = 0;
      }
      if(random(2)) x--;
      if(tt[i] < 4){
        y++;
      }else{
        y--;
      }

      if(((tt[i] % 30)==0)&&(random(10) > 7)){
        charamake(x,y,CHRTYPE2);
      }
      break;

    case CHRTYPE5:    //敵5
      x--;    
      if(tt[i] == 0){
        if((x < 8)||(random(70)==0))
          tt[i] = 1;
      }else{
        y--;
      }
      break;

    case CHRTYPE6:    //敵6
      x--;
      if(tt[i] > 0){
        y--;
        tt[i]--;
      }else if(tt[i] < 0){
        y++;
        tt[i]++;
      }else{
        if(random(5)==0) tt[i] = 3;
        if(random(5)==0) tt[i] = -3;
      }
      break;

    case CHRTYPEEXP:    //爆発1
    case CHRTYPEEXP+1:  //爆発2
    case CHRTYPEEXP+2:  //爆発3
    case CHRTYPEEXP+3:  //爆発4
      tt[i]++;
      if(tt[i] & 1)tc[i]++;
      if(tc[i] >= (CHRTYPEEXP + 4)){
        x = NONE;
        y = NONE;
      }
      break;

    case CHRLAZER:  //弾
      x++;
      hit = sphitenemy(x,y);    //当たり判定
      if(hit>=0){
        charamake(x,y-2,CHRTYPEEXP);
        charamake(x-2,y,CHRTYPEEXP);
        charamake(x+2,y,CHRTYPEEXP);
        charamake(x,y+2,CHRTYPEEXP);
        x = NONE;
        y = NONE;
        tc[hit] = CHRTYPEEXP;
      }
      break;

    default:
      break;
    } //switch
 
    if((x < -1)|(x > 32)|(y < -1)|(y > 16)){
      x = NONE;
      y = NONE;
    }
    vram_spput(x,y, tc[i]);
    tx[i] = x;
    ty[i] = y;
  } //for
}
//---------------------------------------------------------------------
// VRAMから色情報取得
int ptx=0;    //シリアル出力ポインタ(0-511)
char disable = 3;

unsigned char sendvram(void) {
  unsigned char color,ret;

  if(disable){
    delay(1000);
    disable--;
    return;
  }

  ret=0;
  color = vram[ptx % 32][ptx / 32];
  if(color & 0b110000) ret |= (1<<1); //R
  if(color & 0b001100) ret |= (1<<2); //G
  if(color & 0b000011) ret |= (1<<0); //B

  Serial.write(ret); //VRAM情報送信
  ptx = (ptx+1) % (32*16);
}
//---------------------------------------------------------------------
// メインループ
#define GAMESPEED 300

void loop() {
  sendvram();

  cnt=(cnt+1) % GAMESPEED;
  if (cnt > 0){
    return;       //ゲーム速度調整
  }

  if(seq==0){
    //タイトル画面
    vram_clear();
    vram_putstr(0,1,(unsigned char *)"SHOOTING",COL_RED);
    vram_putstr(4,9,(unsigned char *)"PUSH A",COL_WHITE);

    if (digitalRead(buttona) == LOW){
      charainit();        //キャラクタ初期化
      charamake(7,7,0);   //自機配置
      bginit();
      gameover = 0; // game start
      seq++;
    }
  }else{
    vram_clear();
    bgctrl();       //背景処理
    charactrl();    //キャラクタ処理
    if(gameover){   //ゲームオーバー中
      gameover++;
      if(gameover>100)seq=0;
    }
  }
  vram_update();
}

