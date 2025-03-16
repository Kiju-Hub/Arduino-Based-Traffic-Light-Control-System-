# Arduino-Based-Traffic-Light-Control-System-
# -🚦아두이노 활용 신호등 제어 시스템
> 개요 : 신호등 동작 시스템은 Arduino와 p5.js를 이용하여 신호등의 동작을 구성하고 신호등의 상태를 실시간으로 그래픽 인터페이스로 보여줍니다.


## 🔴 시연 동영상 
> https://www.youtube.com/watch?v=c_itagSuSwg
> 시연 동영상입니다. 


## 🟠 신호등 동작 기능
1️⃣ NORMAL : 아래 사이클은 기본동작으로 무한히 반복 동작합니다.  

![image](https://github.com/user-attachments/assets/613ce40e-50a9-410a-bd08-2e34f2919dc5)

2️⃣ 버튼을 활용한 모드 전환 (인터럽트 기반)
- button1Pin (5번 핀): 깜빡이기 모드 (Blink Mode)  
- button2Pin (6번 핀): 적색 신호 전용 모드 (Red Only Mode)  
- button3Pin (7번 핀): 모든 LED OFF 모드 (All Off Mode)

3️⃣ LED 밝기 조절 (가변저항 활용)  
- potPin (A0)에 연결된 가변저항을 이용하여 LED 밝기 조절 가능
- analogRead(potPin) 값을 읽어 LED의 PWM 값을 조정
- 5~255 범위에서 밝기 설정 가능.  
  
4️⃣ 웹 인터페이스를 통해 신호등의 밝기 인디케이터, 현재 신호등 상태, 신호등 모드 인디케이터 등의 기능을 구현


## 🟡 회로구성
![2](https://github.com/user-attachments/assets/177c8f42-d43b-4ecd-9044-d64e908f558c)  


1️⃣ LED 회로 
- Red LED PIN -> 9번
- Yellow LED PIN -> 10번  
- Blue LED PIN -> 11번  
- 각각의 LED는 220Ω 저항을 통해 GND와 연결
- analogWrite을 통하여 PWM을 이용한 밝기 조절 가능


2️⃣ 버튼 회로 (인터럽트)  
- 5번 핀 (Button1) → 깜빡이기 모드 (Blink Mode)  
- 6번 핀 (Button2) → 적색 신호 전용 모드 (Red Only Mode)  
- 7번 핀 (Button3) → 모든 LED OFF 모드 (All Off Mode)  
- 각 버튼은 pull-up저항(내부의 INPUT_PULLUP설정)으로 기본값은 HIGH > 버튼을 누르면 LOW(FALLING 인터럽트 감지)

  
3️⃣ 가변저항 (LED의 밝기 조절)
- 가변저항을 A0핀에 연결 후 LED의 밝기를 조절  
- 회로 연결  
  - 한쪽 끝 : 5V  
  - 반대쪽 끝 : GND  
  - 가운데 : A0(Analog input)  



## 🟢 코드 설명  
 1️⃣  아두이노 코드 요약  
- TaskScheduler 기반 비동기 신호등 제어  

trafficLightTask: 적색(🔴) → 황색(🟡) → 청색(🔵) 신호등 자동 점등 및 깜빡임 제어  
blueBlinkTask: 청색 신호 깜빡임 구현  
blinkTask: 모든 LED 깜빡이기 모드  
adjustBrightnessTask: 가변저항을 이용해 LED 밝기 조절 (PWM)  
serialTask: 100ms마다 p5.js와 Serial JSON 통신으로 신호등 상태 전송  


- 인터럽트를 이용한 버튼 이벤트 처리  
toggleBlinkMode(): Button1 (5번 핀) → 깜빡이기 모드 ON/OFF  
toggleRedOnlyMode(): Button2 (6번 핀) → 적색 신호 전용 모드  
toggleAllLedOff(): Button3 (7번 핀) → 모든 LED OFF 모드  
attachPCINT()을 활용한 PinChangeInterrupt (FALLING 트리거) 방식으로 버튼 감지


- Serial 통신을 활용한 실시간 상태 전송 (JSON 형식)  
sendSerialData(): 신호등 색상, 밝기, 현재 모드를 JSON 데이터로 변환 후 시리얼 전송


- TaskScheduler 사용하여 실행 흐름 최적화  
loop() 내부에서 runner.execute(); 호출하여 모든 Task를 비동기적으로 실행  
delay() 없이 Task 별 실행 주기를 관리하여 효율적 동작  


2️⃣  p5.js 코드 요약 (Serial 통신 + UI를 통한 데이터 인터페이스 가시화)  
- Serial 통신 연결 및 데이터 수신  
connectToArduino(): 시리얼 포트 연결 (Baud rate: 9600) 후 데이터 수신  
readLoop(): Arduino에서 JSON 데이터 수신하여 serialEvent()로 전달  
serialEvent(data): JSON 데이터를 파싱하여 모드, 밝기, 현재 신호등 상태 업데이트


- UI 생성 및 LED 밝기 가시화  
setup(): 캔버스(600x500) 생성, LED 밝기(가변저항) 값을 표시하는 슬라이더 추가  
draw(): 현재 신호등 모드 및 LED 상태를 시각적으로 표현


- 신호등 시각화 (모드별 표현)  
Normal Mode: 수신된 JSON 데이터 기반으로 적색(🔴) → 황색(🟡) → 청색(🔵) 순서대로 표현  
Blink Mode: 모든 LED가 깜빡이는 애니메이션 효과 적용  
Red Only Mode: 적색 LED만 표시  
All Off Mode: 모든 LED OFF  
All Blink Mode: 모든 LED 동시 깜빡임  



## 🔵 핵심 요약  
> 📌 아두이노와 p5.js를 Serial 통신으로 연결하여 신호등 동작을 자동화하고, 버튼 입력 및 가변저항을 통해 모드를 변경하며, 실시간으로 UI에 가시화하는 시스템 구현

