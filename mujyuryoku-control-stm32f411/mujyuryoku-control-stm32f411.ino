#include <Arduino.h>
#include <HardwareSerial.h>

#define DATA_SIZE       100000

uint8_t data_list[DATA_SIZE];

#define EN_GPIO_NUM         PA8
#define STEP_GPIO_NUM       PA9
#define DIR_GPIO_NUM        PA10

#define EN_GPIO_NUM2        PA5
#define STEP_GPIO_NUM2      PA6
#define DIR_GPIO_NUM2       PA7

#define SW_GPIO_NUM         PA0  // KEY button on Black Pill

#define LED_GPIO_NUM        PC13 // LED on Black Pill
#define LED2_GPIO_NUM       PC14

#define HALL_IC_GPIO_NUM    PB8
#define HALL_PW_GPIO_NUM    PB9

#define STEP_4
#ifdef STEP_4
#define SHIFT_STEP 2
#define STEP_MICRO_SEC 4
#else
#define SHIFT_STEP 3
#define STEP_MICRO_SEC 8
#endif

#define DIR_UP     digitalWrite(DIR_GPIO_NUM,HIGH)
#define DIR_DOWN   digitalWrite(DIR_GPIO_NUM,LOW)
#define DIR_UP2    digitalWrite(DIR_GPIO_NUM2,HIGH)
#define DIR_DOWN2  digitalWrite(DIR_GPIO_NUM2,LOW)

float pitch_gear = 2.0e-3;  // 2mm
float gear_tooth = 60.0f;
// float step_deg = 1.8f;    // 1.8 deg / step : 200 pulse/rev
// float step_deg = 0.9f;    // 0.9 deg / step : 400 pulse/rev
// float step_deg = 0.45f;    // 0.45 deg / step : 800 pulse/rev 脱調する
float step_deg = 0.225f;    // 0.225 deg / step : 1600 pulse/rev
// float step_deg = 0.1125f;    // 0.1125 deg / step : 3200 pulse/rev これは駄目
float moter_coefficient;

// 1600 pulse/rev とすると、4.9m/sの為には 1パルス 15.3 μ秒

float g = 9.80665f * 1.0f;

int slow_speed = 1;    // Standard 1 - 10
// int slow_speed = 10;    // Slowly

float n00 = 2.2f;     // max G
float n01 = 0.50f;      // max G
// float h_peak = 2.0f;    // Height
//float h_peak = 0.7f;    // Height
// float h_peak = 0.5f;    // Height
//             float h_peak = 1.3f;    // Height 1.3m までが安全圏、1.4 m だと4回に 3回くらい上昇中に止まる ＠モーター2連、30V ４A
float h_peak = 1.4f;    // Height 1.3m までが安全圏、1.4 m だと4回に 3回くらい上昇中に止まる ＠モーター2連、30V ４A
// float h_peak = 2.5f;    // Height 2.5mくらいまでは、空荷で脱調せずにモーター駆動できるようだ
float n1 = 3.0f;      // max G
float h3 = 0.00f;  // 最後の高さ
// float nxx =  9.80665f * 0.05f * 0.0f;

float t0 = 0.0f;
float t10 = 0.0f;
float t11 = 0.0f;
float h0 = 0.0f;
float t2 = 0.0f;
float t_end = 0.0f;

int i_end;
int i_peek;

float h = 0.0f;
float v = 0.0f;
float a = 0.0f;

float t = 0.0f;
int flgGO = 0;
uint64_t tim_start = 0;
float height_m = 0.0f;
float offset0 = 0.0f;
uint64_t tim_count_old = 0;

float t_gap0 = 0.18f;
// float t_gap1 = 0.04f;
// float t_gap1 = 0.02f;
float t_gap1 = 0.03f;
float h_peak_gap;
float v0;
//float h0;
float v0_gap0;
float h0_gap0;
float v1_gap1;
float h1_gap1;

int toggle = 0;
int sw_status = 0;
int hall_ic_status = 0;
int flgHall = 0;

// printf function to support Serial or printf
void local_printf(char *fmt, ...)
{
  va_list argptr;
  va_start(argptr, fmt);
#ifdef STM32F411xE
  SerialUSB.printf(fmt, argptr);
#else
  printf(fmt, argptr);
#endif
  va_end(argptr);
}

void printPara() {
  local_printf("t0 (加速する時間):%lfs\n", t0);
  local_printf("t10 (加速終了から頂点までの時間):%lfs\n", t10);
  local_printf("t11 (頂点から減速開始までの時間):%lfs\n", t11);
  local_printf("Zero G Time （= t10 + 111）:%lfs\n", t_gap0 + t_gap1 + t10 + t11);
  local_printf("合計時間(= t0 + t10 + t11 + t2):%lfs\n", t_end);
  local_printf("加速度 初期値:%lfG (1.0+%lf)G\n", n00, n00 - 1.0f);
  local_printf("加速度 加速終了時の値:%lfG (1.0+%lf)G\n", n00 + n01 * t0, n00 + n01 * t0 - 1.0f);
  local_printf("加速の高さ:%lfm\n", (n00 - 1.0)*g / 2.0 * t0 * t0 + n01 * g / 6.0 * t0 * t0 * t0);
  if (t_gap1 < 0.001f) {
    local_printf("加速終了から頂点の高さ:%lfm\n", t10 * t10 * g / 2.0);
    local_printf("最初から頂点の高さ:%lfm\n", (n00 - 1.0)*g / 2.0 * t0 * t0 + n01 * g / 6.0 * t0 * t0 * t0 + t10 * t10 * g / 2.0);
  } else {
    local_printf("加速終了からギャップ開始までの時間:%f秒\n", t_gap0);
    local_printf("ギャップ時間（一定速度が続く時間）:%f秒\n", t_gap1);
    local_printf("加速終了から頂点の高さ:%fm\n", h_peak_gap - ((n00 - 1.0f)*g / 2.0f * t0 * t0 + n01 * g / 6.0f * t0 * t0 * t0));
    local_printf("最初から頂点の高さ:%0.03fm\n", h_peak_gap);
  }

  local_printf("減速時の加速度:%lfG (1.0+%lf)G\n", n1, n1 - 1.0f);
  local_printf("頂点から減速開始までの高さ:%lfm\n", t11 * t11 * g / 2.0);
  local_printf("減速開始から減速終了までの高さ:%lfm\n", (n1 - 1.0f)*t2 * t2 * g / 2.0);
  //  local_printf("内部加速:%lfG\n", nxx / 9.80665f);
  local_printf("内部加速の高さ:%lfm\n", offset0);
}


void setup() {
  SerialUSB.begin(9600);
  while (!SerialUSB) {
    delay(100);  // wait for USB serial to be ready
  }
  SerialUSB.println("\n無重力制御スタート!! with STM32F411CEU\n");

  pinMode(EN_GPIO_NUM, OUTPUT);
  pinMode(STEP_GPIO_NUM, OUTPUT);
  pinMode(DIR_GPIO_NUM, OUTPUT);
  pinMode(EN_GPIO_NUM2, OUTPUT);
  pinMode(STEP_GPIO_NUM2, OUTPUT);
  pinMode(DIR_GPIO_NUM2, OUTPUT);

  pinMode(SW_GPIO_NUM, INPUT_PULLUP);
  pinMode(HALL_IC_GPIO_NUM, INPUT_PULLUP);
  pinMode(LED_GPIO_NUM, OUTPUT); // LED

  pinMode(HALL_PW_GPIO_NUM, OUTPUT); // LED
  pinMode(LED2_GPIO_NUM, OUTPUT); // LED

  t0 = 0.0;
  while ((n00 - 1.0f)*g / 2.0f * t0 * t0 + n01 * g / 6.0f * t0 * t0 * t0 + g / 2.0f * ((n00 - 1.0)*t0 + n01 / 2.0f * t0 * t0) * ((n00 - 1.0f)*t0 + n01 / 2.0f * t0 * t0) < h_peak) {
    t0 += 0.00001f;
  }
  t10 = (n00 - 1.0) * t0 + n01 / 2.0 * t0 * t0;
  t2 = sqrt(2.0 * (h_peak - h3) / (g * (n1 * n1 - n1)));
  t11 = (n1 - 1.0) *  t2;
  t_end = t0 + t10 + t11 + t2;
  h_peak_gap = h_peak;
  if ((t_gap0 < t10) && (t_gap1 >= 0.001f)) {
    v0 = (n00 - 1.0f) * g * t0 + n01 * g * t0 * t0 / 2.0;
    h0 = (n00 - 1.0f) * g / 2.0f * t0 * t0 + n01 * g / 6.0f * t0 * t0 * t0;
    v0_gap0 = v0 - g * t_gap0;
    h0_gap0 = h0 + v0 * t_gap0 - 0.5f * g * t_gap0 * t_gap0;
    v1_gap1 = v0 - g * t_gap0; // 変化なし
    h1_gap1 = h0_gap0 + v1_gap1 * t_gap1;
    t10 = v1_gap1 / g;
    h_peak_gap = h1_gap1 + v1_gap1 * t10 - 0.5f * g * t10 * t10;
    t2 = sqrt(2.0f * (h_peak_gap - h3) / (g * (n1 * n1 - n1)));
    t11 = (n1 - 1.0f) *  t2;
    t_end = t0 + t_gap0 + t_gap1 + t10 + t11 + t2;
  } else {
    if (t_gap0 >= t10) {
      local_printf("\n\n\nError t_gap0 >= t10\n\n");
    }
    t_gap0 = 0.0f;
  }
  v0 = (n00 - 1.0f) * g * t0 + n01 * g * t0 * t0 / 2.0f;
  h0 = (n00 - 1.0f) * g / 2.0f * t0 * t0 + n01 * g / 6.0f * t0 * t0 * t0;
  v0_gap0 = v0 - g * t_gap0;
  h0_gap0 = h0 + v0 * t_gap0 - 0.5 * g * t_gap0 * t_gap0;
  v1_gap1 = v0 - g * t_gap0; // 変化なし
  h1_gap1 = h0_gap0 + v1_gap1 * t_gap1;
  h = 0.0f;
  v = 0.0f;
  a = 0.0f;

  float tk = ((float)(STEP_MICRO_SEC) / 1000000.0f);
  int i = 0;
  while (i < DATA_SIZE) {
    data_list[i++] = 0;
  }

  t = 0.0f;
  moter_coefficient = 2.0f / pitch_gear / gear_tooth * 360.0f / step_deg;
  toggle = 0;
  i = 0;
  int64_t moter_step = 0;
  //  int old_dir = 1;
  while (t <= t_end) {
    h = 0.0f;
    if (t < t0) {
      h = (n00 - 1.0f) * g / 2.0f * t * t + n01 * g / 6.0f * t * t * t;
    } else if (t < t0 + t_gap0) {
      float dt = t - t0;
      h = h0 + v0 * dt - 0.5f * g * dt * dt;
      /*
        float offset = 0.0f;
        dt = t - t0;
        if (dt < (t10 + t11) / 4.0f) {
        offset = nxx / 2.0 * dt * dt;
        } else if (dt < (t10 + t11) * 3.0f / 4.0f) {
        offset = offset0 - nxx / 2.0 * (dt - (t10 + t11) / 2.0f) * (dt - (t10 + t11) / 2.0f);
        } else {
        offset = nxx / 2.0 * (dt - (t10 + t11)) * (dt - (t10 + t11));
        }
        h -= offset;
      */
    } else if (t < t0 + t_gap0 + t_gap1) {
      h = h0_gap0 + v1_gap1 * (t - t0 - t_gap0);
    } else if (t < t0 + t_gap0 + t_gap1 + t10 + t11) {
      float dt = t0 + t_gap0 + t_gap1 + t10 - t;
      h = h_peak_gap - 0.5f * g * dt * dt;
    } else {
      float dt = t0 + t_gap0 + t_gap1 + t10 + t11 + t2 - t;
      h = 0.5f * (n1 - 1.0f) * g * dt * dt;
    }
    int64_t new_moter_step = (int64_t)(h * moter_coefficient + 0.5f);
    uint8_t data_8 = 0;
    data_8 = 0;
    if (toggle) {
      data_8 = 1;
    }
    if (new_moter_step > moter_step) {
      i_peek = i;
      data_8 |= 2;
      if (toggle) {
        data_8 = 0;
        toggle = 0;
      } else {
        data_8 = 1;
        toggle = 1;
      }
    }
    if (new_moter_step < moter_step) {
      if (toggle) {
        data_8 = 0;
        toggle = 0;
      } else {
        data_8 = 1;
        toggle = 1;
      }
    }
    if (i / 8 < DATA_SIZE) {
      data_8 = data_8 << (i % 8);
      data_list[i / 8] |= data_8;
      i++;
    }
    moter_step = new_moter_step;
    t += tk;
  }
  i_end = i;
  local_printf("i_end:%d\n", i_end);
  local_printf("endt:%fs\n", ((float)i_end * tk));
  local_printf("i_peek:%d\n", i_peek);
  local_printf("peekt:%fs\n", ((float)i_peek * tk));
  digitalWrite(LED_GPIO_NUM, LOW); // LED ON

  digitalWrite(HALL_PW_GPIO_NUM, HIGH); // HALL IC PWR
  hall_ic_status = 0;
  digitalWrite(LED2_GPIO_NUM, HIGH); // LED
  DIR_UP;
  DIR_UP2;
  digitalWrite(STEP_GPIO_NUM, LOW);
  digitalWrite(STEP_GPIO_NUM2, LOW);
  digitalWrite(EN_GPIO_NUM, HIGH);
  digitalWrite(EN_GPIO_NUM2, HIGH);
  t = 0.0f;
  flgGO = 0;
  flgHall = 0;
  tim_start = 0;
  height_m = 0.0f;
  //  offset0 = nxx / 2.0 * ((t10 + t11) / 4.0f) * ((t10 + t11) / 4.0f) * 2.0f;

  moter_coefficient = 2.0f / pitch_gear / gear_tooth * 360.0f / step_deg;
  toggle = 0;
  sw_status = 0;

  printPara();
}


void loop() {
  uint64_t tim_count_new = ((micros() / slow_speed) >> SHIFT_STEP); // 16μ/8μ/4μ秒毎
  if (tim_count_old != tim_count_new) {
    if (tim_count_old & 0x20000) {
      digitalWrite(LED_GPIO_NUM, HIGH); // LED
    } else {
      digitalWrite(LED_GPIO_NUM, LOW); // LED
    }
    if (hall_ic_status == 0) {
      if (digitalRead(HALL_IC_GPIO_NUM) == 0) {
        digitalWrite(LED2_GPIO_NUM, HIGH); // LED
        digitalWrite(HALL_PW_GPIO_NUM, LOW); // HALL IC PWR
        hall_ic_status = 1000;
      } else {
        digitalWrite(LED2_GPIO_NUM, LOW); // LED
      }
    } else {
      hall_ic_status--;
      if (hall_ic_status == 0) {
        digitalWrite(HALL_PW_GPIO_NUM, HIGH); // HALL IC PWR
      }
    }
    if (digitalRead(SW_GPIO_NUM) == 0) {
      if (sw_status) {
        DIR_UP;
        DIR_UP2;
        digitalWrite(STEP_GPIO_NUM, LOW);  // STEP
        digitalWrite(STEP_GPIO_NUM2, LOW);  // STEP
        toggle = 0;
        digitalWrite(EN_GPIO_NUM, LOW);  // EN
        digitalWrite(EN_GPIO_NUM2, LOW);  // EN
        t = 0.0f;
        height_m = 0.0f;
        tim_start = tim_count_old;
        flgGO = 1;
        flgHall = 0;
        sw_status = 0;
      }
    } else {
      sw_status = 1;
    }
    if (tim_count_new % (1000000 / STEP_MICRO_SEC / slow_speed) == 0) {
      local_printf("%d Zero G Time:%lfs h_peak:%fm h_peak_gap:%fm t_gap0:%fs t_gap1:%fs\n", (int)tim_count_new / (1000000 / STEP_MICRO_SEC / slow_speed), t_gap0 + t_gap1 + t10 + t11, h_peak, h_peak_gap, t_gap0, t_gap1);
    }
  }
  tim_count_old = tim_count_new;
  if ( Serial.available() ) {       // 受信データがあるか？
    while ( Serial.available() ) {
      char key = Serial.read();            // 1文字だけ読み込む
    }
    digitalWrite(LED_GPIO_NUM, HIGH); // LED
    //    digitalWrite(LED2_GPIO_NUM, HIGH); // LED
    DIR_UP;
    DIR_UP2;
    digitalWrite(STEP_GPIO_NUM, LOW);  // STEP
    digitalWrite(STEP_GPIO_NUM2, LOW);  // STEP
    digitalWrite(EN_GPIO_NUM, LOW);  // EN
    digitalWrite(EN_GPIO_NUM2, LOW);  // EN
    t = 0.0f;
    height_m = 0.0f;
    tim_start = tim_count_old;
    flgGO = 1;
    flgHall = 0;
  }

  float dt;
  int error_no = 0;
  int i;
  while (flgGO) {
    tim_count_new = ((micros() / slow_speed) >> SHIFT_STEP); // 4μ/8μ秒毎
    if (tim_count_old != tim_count_new) {
      tim_count_old = tim_count_new;
      i = (tim_count_new - tim_start) & 0x00ffffff;
      if ((i < i_peek) || (flgHall && (i < i_end))) {
        if (i <= i_peek) {
          DIR_UP;
          DIR_UP2;
        } else {
          DIR_DOWN;
          DIR_DOWN2;
        }
        if ((data_list[i / 8] >> (i % 8)) & 0x01) {
          digitalWrite(STEP_GPIO_NUM, HIGH);  // STEP
          digitalWrite(STEP_GPIO_NUM2, HIGH);  // STEP
        } else {
          digitalWrite(STEP_GPIO_NUM, LOW);  // STEP
          digitalWrite(STEP_GPIO_NUM2, LOW);  // STEP
        }

        if (digitalRead(HALL_IC_GPIO_NUM) == 0) {
          digitalWrite(LED2_GPIO_NUM, HIGH); // LED
          digitalWrite(HALL_PW_GPIO_NUM, LOW); // HALL IC PWR
          hall_ic_status = 1000;
          flgHall = 1;
        } else {
          //        digitalWrite(LED2_GPIO_NUM, LOW); // LED
        }
      } else {
        flgGO = 0;
        flgHall = 0;
        digitalWrite(LED_GPIO_NUM, HIGH); // LED
        //        digitalWrite(LED2_GPIO_NUM, HIGH); // LED
        DIR_UP;
        DIR_UP2;
        digitalWrite(STEP_GPIO_NUM, LOW);  // STEP
        digitalWrite(STEP_GPIO_NUM2, LOW);  // STEP
        toggle = 0;
        digitalWrite(EN_GPIO_NUM, HIGH);  // EN
        digitalWrite(EN_GPIO_NUM2, HIGH);  // EN
        printPara();
        delay(2000);
        if ( Serial.available() ) {       // 受信データがあるか？
          while ( Serial.available() ) {
            char key = Serial.read();            // 1文字だけ読み込む
          }
        }
      }
      if (tim_count_old & 0x04000) { // 上下運動中の高速点滅
        digitalWrite(LED_GPIO_NUM, HIGH); // LED
        //        digitalWrite(LED2_GPIO_NUM, HIGH); // LED
      } else {
        digitalWrite(LED_GPIO_NUM, LOW); // LED
        //        digitalWrite(LED2_GPIO_NUM, LOW); // LED
      }
    }
  }
}
