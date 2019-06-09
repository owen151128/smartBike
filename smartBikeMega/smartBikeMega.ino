#include <SoftwareSerial.h>
#include "DHT.h"
#include "Timer.h" //https://playground.arduino.cc/Code/Timer/

//#include <Time.h>//속도계
//#include <LiquidCrystal.h>//LCD


//입력
#define trig_pin 29
#define echo_pin 30//초음파센서
#define vibration_pin A0//진동센서
#define left_pin 25
#define right_pin 26//좌우 방향지시 스위치_3단 복귀스위치
#define brake_pin 29//브레이크 스위치_리미트스위치
#define flash_pin 24//후레쉬 스위치_토글 스위치
#define horn_pin 22//경적_토글복귀스위치
#define emergency_pin 23//비상등스위치_토글 스위치
#define magnet_pin 27//속도계_자석센서
#define temp_pin 28//온도센서
#define BRX_pin 11
#define BTX_pin 10//블루투스 통신
//#define URX_pin 30
//#define UTX_pin 31//아두이노 우노 통신



//출력
#define speaker_pin 4
#define light_pin 5

#define CLK_pin 6
#define CS_pin 7
#define DIN_pin 8//도트매트릭스_LedControl



Timer t1;
Timer t2;

DHT dht(temp_pin, DHT11);//온습도계


int flashlight = 0;//플래쉬라이트

int guard = 0;//도난방지모드 설정
int vib_data = 0;//진동변수

int horn = 0;//경적
int left = 0;
int right = 0;
int emergency = 0;
int brake = 0;





//속도변수
float radius = 20; // 바퀴당 이동 거리를 확인 하기 위해 자전거 바퀴의 반지름을 입력해 줍니다.(Cm 단위)
float circle = (2 * radius * 3.14) / 100;  // 자전거 바퀴의 둘레를 계산(단위를 m로 바꿔주기 위해 100을 나눕니다.)

float bySpeed = 0; // 자전거의 속도
float ckTime = 0;  // 리드스위치가 
float uckTime = 0; // Unckecked
float cycleTime = 0;  // 리드스위치가 인식이 안됬을 시간 부터 인식됬을 때까지의 시간
float distance = 0; // 자전거의 누적 이동 거리
float lcdDis = 0; // 자전거의 이동 거리를 LCD출력에 맞게 바꿔즌 값.(단위 수정 or 소숫점 제거)

int count = 0;  // 리드스위치의 노이즈를 제거하기 위해 카운트를 넣어줍니다.


float hum = 0.0f;//습도
float temp = 0.0f;//온도

boolean lead = 0;  // 리드 스위치가 닫혔는지 확인하는 변수
boolean beep_i = false; // 경보음 토글변수
boolean check = false; //속도계

boolean sp_off = false; //스피커 꺼짐


String data = "";


//반드시 수정
//LiquidCrystal lcd(12,11,5,4,3,2);   //RS 핀, E핀, 데이터 핀 4개


SoftwareSerial BTSerial(BTX_pin, BRX_pin);//블루투스
//SoftwareSerial UNOSerial(UTX_pin, URX_pin);//블루투스



void ultra();
void vib();
void ON(int output);
void OFF(int output);
void beep();
void show_speed();
void show_temp();
void bluetoothCallback();
void setDefanse(int value);
void sendAndroid(String message);
String receive();
void procedure_BT();
void show_status();


void setup() {
	Serial.begin(9600);
	Serial.println("Hello!");
	BTSerial.begin(9600);//블루투스 통신 보드레이트 설정
  //UNOSerial.begin(9600);//아두이노 우노 통신 보드레이트 설정

  dht.begin();//온습도계




	pinMode(trig_pin, OUTPUT);
	pinMode(echo_pin, OUTPUT);//초음파센서


	//pinMode(vibration_pin, INPUT);//진동센서
	pinMode(left_pin, INPUT_PULLUP);
	pinMode(right_pin, INPUT_PULLUP);
	pinMode(brake_pin, INPUT_PULLUP);
	pinMode(flash_pin, INPUT_PULLUP);
	pinMode(horn_pin, INPUT_PULLUP);
	pinMode(emergency_pin, INPUT_PULLUP);
	pinMode(magnet_pin, INPUT);


  t1.every(100, beep);
  t2.every(500, show_speed);
  t2.every(500, show_temp);
  t2.every(100, procedure_BT);
  t2.every(500, show_status);

	//LCD
	//lcd.begin(16, 2); // LCD 크기 지정, 2줄 16칸
	//lcd.clear();     // 화면 초기화

}




void loop() {



//	// BT에서 받은 데이터가 있으면 데이터를 읽어서 PC로 전달
//	if (BTSerial.available()) {
//		Serial.println(BTSerial.read());
//	}
//	// PC에서 받은 데이터가 있으면 데이터를 읽어서 BT로 전달
//	if (Serial.available()) {
//		BTSerial.write(Serial.read());
//	}

  brake = digitalRead(brake_pin);//브레이크
  if(brake == 1) {
    
  } else {
  
  }

  left = digitalRead(left_pin);//왼쪽깜빡이
  if(left == 0) {
    
  } else {
  
  }

  right = digitalRead(right_pin);//오른쪽깜빡이
  if(right == 0) {
    
  } else {
  
  }

  emergency = digitalRead(emergency_pin);//비상깜빡이
  if(emergency == 0) {
    
  } else {
  
  }



  flashlight = digitalRead(flash_pin);//플레시라이트_else
  if(flashlight == 0) {
    ON(light_pin);
  } else {
  OFF(light_pin);
  }


  horn = digitalRead(horn_pin);//경적_else
  if(horn == 0) {
      
    tone(speaker_pin,400);
    
    sp_off = false;
    
  } else {
    
      if(sp_off==false) {
  noTone(speaker_pin);
  sp_off = !sp_off;
      }
  }


  if(guard == 1) {
  vib_data = analogRead(vibration_pin);//도난경보시스템
  Serial.println(vib_data);
  if(vib_data < 100) {  ////////////////////////////////////////////진동감지 크기설정가능
      while(1) {
      Serial.println("도난발생");
      t1.update();
      if(!(digitalRead(flash_pin) || digitalRead(right_pin) || digitalRead(horn_pin) || digitalRead(emergency_pin))) {
        Serial.println("해제 커맨드 입력");
        break; }
      }
    }
  }





  hum = dht.readHumidity(); // 온습도센서
  temp = dht.readTemperature();
    if (isnan(hum) || isnan(temp) ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
 
  


	/* 참고용 GPS 로그
	Lat/Long: 37.30678, 126.94424
	Date: 5/11/2016  Time: 8:41:30.0
	Altitude (meters): 1000000.00
	Course (degrees): 0.00
	Speed(kmph): 0.09
	*/


  
	//속도계 예제
	check = digitalRead(magnet_pin); // 리드스위치의 상태를 확인합니다.

	if (check == 1 && lead == 0) {  // 리드 스위치가 열릴 때(닫힘 -> 열림)
		ckTime = millis();  // 시간을 확인해서 저장합니다.
		lead = 1;  // lead값을 1로 바꿔줍니다.(리드스위치가 열려있는 상태값 저장)
	}

	else if (check == 0 && lead == 1 && count > 5) {  // 리드 스위치가 닫히고(열림 -> 닫힘), 노이즈 방지 카운트가 5이상일때
		uckTime = millis();  // 시간을 확인해서 저장합니다.

		cycleTime = (uckTime - ckTime) / 1000;
		// 열릴 때 시각과 닫힐 때 시각의 차를 이용하여 바퀴가 한바퀴 돌때 걸린 시간을 계산합니다.
		bySpeed = (circle / cycleTime) * 3.6; // 바퀴가 한바퀴 돌때의 거리와 시간을 가지고 속도를 구해줍니다.(단위는 Km/h입니다.)
		lead = 0;
		count = 0;
		distance += circle;  // 한바퀴 돌았으면 이동거리를 누적 이동거리에 더해줍니다.
	}

	if (check == 1) {  // 리드 스위치가 열려있으면 카운트를 1씩 증가 시켜 줍니다.
		count++;
		if (count > 150) { // 카운트가 150이 넘어가면(자전거가 멈췄을 때) 속도를 0으로 바꿔줍니다.
			bySpeed = 0;
		}
	}

  t2.update();




  





	//LCD 값 출력
	/*lcd.setCursor(0, 0);          //커서를 0,0에 지정
	lcd.print("Speed: ");
	if (bySpeed < 10) {            // 속도가 한자리라면 앞에 0을 붙여 줍니다.
		lcd.print('0');
	}
	lcd.print(bySpeed);
	lcd.print("Km/h");           // 속도를 lcd에 출력합니다.("Speed: xx.xxKm/h");

	lcdDis = distance / 1000;
	lcd.setCursor(0, 1);          // 커서를 0,1에 지정
	lcd.print("Distance: ");
	if (lcdDis >= 100) {           // 이동거리가 100Km가 넘어가면 소숫점을 지워줍니다.
		lcd.print((int)lcdDis);
	}
	else {
		lcd.print(lcdDis);
	}
	lcd.print("Km");             // 이동거리를 lcd에 출력합니다.("Distance: xx.xxKm")

	lcd.print("               ");        //전 글씨 삭제
	Serial.print("Speed : ");
	Serial.print(bySpeed);
	Serial.println(" km/h");             // 시리얼 모니터를 이용하여 속도를 확인합니다.
	*/

}



void ultra() {
	digitalWrite(trig_pin, LOW);
	digitalWrite(echo_pin, LOW);
	delayMicroseconds(2);
	digitalWrite(trig_pin, HIGH);
	delayMicroseconds(10);
	digitalWrite(trig_pin, LOW);
	unsigned long duration = pulseIn(echo_pin, HIGH);
	float distance = duration / 29.0 / 2.0;
}

void vib() {
	vib_data = digitalRead(vibration_pin);
}



void ON(int output) {
  digitalWrite(output, HIGH);
}

void OFF(int output) {
  digitalWrite(output, LOW);
}


void beep() {
  
if(beep_i== true) {
tone(speaker_pin,300);
} else {
tone(speaker_pin,800);
}
beep_i = !beep_i;

}

void show_speed() {
  Serial.print("속도 : ");
  Serial.print(bySpeed);
  Serial.println("km/h");
}

void show_temp() {
  // 온도와 습도값을 시리얼 모니터에 출력해 줍니다.
  Serial.print((int)temp); Serial.print(" *C, ");
  Serial.print((int)hum); Serial.println(" %");
}


void bluetoothCallback()
{
  //Serial.println(data);
  if(data.equals("getTemp"))
  {
//    Serial.println("send to Android \"temp:20\"");
    String result = "temp:";
    result.concat((int)temp);
    sendAndroid(result);

    return;
  }

  if(data.equals("getSpeed"))
  {
//    Serial.println("send to Android \"speed:40\"");
    String result = "speed:";
    result.concat((int)bySpeed);
    sendAndroid(result);

    return;
  }

  if(data.equals("setDefanse:On"))
  {
    setDefanse(1);

    return;
  }

  if(data.equals("setDefanse:Off"))
  {
    setDefanse(0);

    return;
  }

  Serial.println("Message from Android : \"" + data + "\"  is not defined!! deny this message!");
}

void setDefanse(int value)//보안모드
{
  if(value == 1)
  {
    guard = 1;
    Serial.println("Defanse mode has been on(1)");
  } else
  {
    guard = 0;
    Serial.println("Defanse mode has been off(0)");
  }
}

void sendAndroid(String message)
{
  BTSerial.println(message);
}

String receive()
{
  if(BTSerial.available())
  {
    return BTSerial.readStringUntil('\n');
  } else 
  {
    return "";
  }
}




void procedure_BT() {
data = receive(); // 블루투스 통신
  if(!data.equals(""))
  {
    bluetoothCallback();
    data = "";
  }
}


void show_status() {
  if(brake == 1)
    Serial.println("브레이크");
  if(left == 0)
    Serial.println("좌회전");
  if(right == 0)
    Serial.println("우회전");
  if(emergency == 0) 
    Serial.println("비상등");
  if(flashlight == 0)
    Serial.println("후레쉬");
  if(horn == 0)
    Serial.println("경적");
}
