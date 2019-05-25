#include <SoftwareSerial.h>

#include <Time.h>//속도계
//#include <LiquidCrystal.h>//LCD


//입력
#define trig_pin 20
#define echo_pin 21//초음파센서
#define vibration_pin 10//진동센서
#define left_pin 11
#define right_pin 12//좌우 방향지시 스위치_3단 복귀스위치
#define brake_pin 13//브레이크 스위치_리미트스위치
#define flash_pin 14//후레쉬 스위치_토글 스위치
#define horn_pin 15//경적_토글복귀스위치
#define emergency_pin 16//비상등스위치_토글 스위치
#define magnet_pin 17//속도계_자석센서
#define BRX_pin 19
#define BTX_pin 18//블루투스 통신
#define URX_pin 30
#define UTX_pin 31//아두이노 우노 통신



//출력
#define speaker_pin 22
#define light_pin 23

#define CLK_pin 24
#define CS_pin 25
#define DIN_pin 26//도트매트릭스_LedControl






int flashlight = 0;//플래쉬라이트

int guard = 0;//도난방지모드 설정
int vib_data = 0;//진동변수

int horn = 0;//경적





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
boolean temp = 0;  // 리드 스위치가 닫혔는지 확인하는 변수


//반드시 수정
//LiquidCrystal lcd(12,11,5,4,3,2);   //RS 핀, E핀, 데이터 핀 4개


SoftwareSerial BTSerial(BTX_pin, BRX_pin);//블루투스
SoftwareSerial UNOSerial(UTX_pin, URX_pin);//블루투스



void ultra();
void vib();
void ON(int output);
void OFF(int output);



void setup() {
	Serial.begin(9600);


	Serial.println("Hello!");
	BTSerial.begin(9600);//블루투스 통신 보드레이트 설정
  UNOSerial.begin(9600);//아두이노 우노 통신 보드레이트 설정




	pinMode(trig_pin, OUTPUT);
	pinMode(echo_pin, OUTPUT);//초음파센서


	pinMode(vibration_pin, INPUT);//진동센서
	pinMode(left_pin, INPUT);
	pinMode(right_pin, INPUT);
	pinMode(brake_pin, INPUT);
	pinMode(flash_pin, INPUT);
	pinMode(horn_pin, INPUT);
	pinMode(emergency_pin, INPUT);
	pinMode(magnet_pin, INPUT);





	//LCD
	//lcd.begin(16, 2); // LCD 크기 지정, 2줄 16칸
	//lcd.clear();     // 화면 초기화

}




void loop() {



	// BT에서 받은 데이터가 있으면 데이터를 읽어서 PC로 전달
	if (BTSerial.available()) {
		Serial.println(BTSerial.read());
	}
	// PC에서 받은 데이터가 있으면 데이터를 읽어서 BT로 전달
	if (Serial.available()) {
		BTSerial.write(Serial.read());
	}








  flashlight = digitalRead(flash_pin);//플레시라이트_else
  if(flashlight == 1) {
    ON(light_pin);
  } else {
  OFF(light_pin);
  }


  horn = digitalRead(horn_pin);//경적_else
  if(horn == 1) {
    tone(speaker_pin,400);
  } else {
  noTone(speaker_pin);
  }
  



  if(guard == 1) {
  vib_data = digitalRead(vibration_pin);//도난경보시스템_미구현
  if(vib_data == 1) {
    for (int j=0; j<10; j++) {
      for(int i=0; i<5; i++) {
      tone(speaker_pin,300);
      delay(10);//딜레이 사용
      tone(speaker_pin,750);
      delay(10);//딜레이 사용
      }}
    } else {
  noTone(speaker_pin);
  }
  }





	/* 참고용 GPS 로그
	Lat/Long: 37.30678, 126.94424
	Date: 5/11/2016  Time: 8:41:30.0
	Altitude (meters): 1000000.00
	Course (degrees): 0.00
	Speed(kmph): 0.09
	*/



	//속도계 예제
	boolean check = digitalRead(magnet_pin); // 리드스위치의 상태를 확인합니다.

	if (check == 1 && temp == 0) {  // 리드 스위치가 열릴 때(닫힘 -> 열림)
		ckTime = millis();  // 시간을 확인해서 저장합니다.
		temp = 1;  // temp값을 1로 바꿔줍니다.(리드스위치가 열려있는 상태값 저장)
	}

	else if (check == 0 && temp == 1 && count > 5) {  // 리드 스위치가 닫히고(열림 -> 닫힘), 노이즈 방지 카운트가 5이상일때
		uckTime = millis();  // 시간을 확인해서 저장합니다.

		cycleTime = (uckTime - ckTime) / 1000;
		// 열릴 때 시각과 닫힐 때 시각의 차를 이용하여 바퀴가 한바퀴 돌때 걸린 시간을 계산합니다.
		bySpeed = (circle / cycleTime) * 3.6; // 바퀴가 한바퀴 돌때의 거리와 시간을 가지고 속도를 구해줍니다.(단위는 Km/h입니다.)
		temp = 0;
		count = 0;
		distance += circle;  // 한바퀴 돌았으면 이동거리를 누적 이동거리에 더해줍니다.
	}

	if (check == 1) {  // 리드 스위치가 열려있으면 카운트를 1씩 증가 시켜 줍니다.
		count++;
		if (count > 150) { // 카운트가 150이 넘어가면(자전거가 멈췄을 때) 속도를 0으로 바꿔줍니다.
			bySpeed = 0;
		}
	}

//	if(digitalRead(A1) == 0){  // 버튼을 누르면 누적거리를 0으로 초기화 합니다.
//	  distance = 0;
//	}


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
