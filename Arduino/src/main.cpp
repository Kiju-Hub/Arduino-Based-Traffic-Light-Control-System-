#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

// 핀 설정
const int redLedPin = 9;  // 빨간 LED 핀
const int yellowLedPin = 10;  // 노란 LED 핀
const int blueLedPin = 11;  // 파란 LED 핀
const int button1Pin = 5;  // 버튼 1 핀
const int button2Pin = 6;  // 버튼 2 핀
const int button3Pin = 7;  // 버튼 3 핀
const int potPin = A0;  // 가변저항

bool redState = false;  // 빨간 LED 상태
bool yellowState = false;  // 노란 LED 상태
bool blueState = false;  // 파란 LED 상태

//TaskScheduler 객체생성
Scheduler runner;

//Task 함수
void trafficLightTaskCallback();  // 신호등 Task 콜백 함수
void blueBlinkTaskCallback();  // 파란불 깜빡이기 Task 콜백 함수
void blinkTaskCallback();  // 깜빡이기 Task 콜백 함수
void adjustBrightnessTaskCallback();  // 밝기 조절 Task 콜백 함수
void sendSerialData();  // 시리얼 데이터 전송 함수

// 인터럽트 핸들러 함수 선언
void toggleBlinkMode();  // 깜빡이기 모드 토글 함수
void toggleRedOnlyMode();  // 빨간불만 켜기 모드 토글 함수
void toggleAllLedOff();  // 모든 LED 끄기 모드 토글 함수

// Task 객체 생성
Task trafficLightTask(500, TASK_FOREVER, &trafficLightTaskCallback, &runner, false);  // 신호등 Task
Task blueBlinkTask(167, 6, &blueBlinkTaskCallback, &runner, false);  // 파란불 깜빡이기 Task
Task blinkTask(500, TASK_FOREVER, &blinkTaskCallback, &runner, false);  // 깜빡이기 Task
Task adjustBrightnessTask(10, TASK_FOREVER, &adjustBrightnessTaskCallback, &runner, true);  // 밝기 조절 Task
Task serialTask(100, TASK_FOREVER, &sendSerialData, &runner, true);  // 시리얼 데이터 전송 Task

//상태 변수
volatile bool blinkMode = false;  // 깜빡이기 모드 상태
volatile bool redOnlyMode = false;  // 빨간불만 켜기 모드 상태
volatile bool allLedOff = false;  // 모든 LED 끄기 모드 상태
bool blinkState = true;  // 깜빡이기 상태
bool blueBlinkStarted = false;  // 파란불 깜빡이기 시작 상태
int brightness = 255;  // LED 밝기
String mode = "Normal";  // 신호등 모드

// 점등 시간 각각, 2초,0.5초, 3초, 0.5초(빨 > 노 > 파 > 노)
unsigned long redDuration = 2000;  // 빨간불 지속 시간
unsigned long yellowDuration = 500;  // 노란불 지속 시간
unsigned long blueDuration = 3000;  // 파란불 지속 시간
const unsigned long extraYellowDuration = 500;  // 추가 노란불 지속 시간
unsigned long previousMillis = 0;  // 이전 시간 저장 변수

void setup() {
    Serial.begin(9600);  // 시리얼 통신 시작 9600설정.

    pinMode(redLedPin, OUTPUT);  // 빨간 LED 핀 출력 모드 설정
    pinMode(yellowLedPin, OUTPUT);  // 노란 LED 핀 출력 모드 설정
    pinMode(blueLedPin, OUTPUT);  // 파란 LED 핀 출력 모드 설정
    pinMode(potPin, INPUT);  // 가변저항 핀 입력 모드 설정

    pinMode(button1Pin, INPUT_PULLUP);  // 버튼 1 핀 입력 풀업 모드 설정
    pinMode(button2Pin, INPUT_PULLUP);  // 버튼 2 핀 입력 풀업 모드 설정
    pinMode(button3Pin, INPUT_PULLUP);  // 버튼 3 핀 입력 풀업 모드 설정

    attachPCINT(digitalPinToPCINT(button1Pin), toggleBlinkMode, FALLING);  // 버튼 1 인터럽트 설정
    attachPCINT(digitalPinToPCINT(button2Pin), toggleRedOnlyMode, FALLING);  // 버튼 2 인터럽트 설정
    attachPCINT(digitalPinToPCINT(button3Pin), toggleAllLedOff, FALLING);  // 버튼 3 인터럽트 설정

    previousMillis = millis();  // 현재 시간 저장
    trafficLightTask.enable();  // 신호등 Task 활성화
    adjustBrightnessTask.enable();  // 밝기 조절 Task 활성화
    serialTask.enable();  // 시리얼 데이터 전송 Task 활성화
}

void loop() {
    runner.execute();  // TaskScheduler 실행
}

// 가변저항 값을 읽어 LED 밝기 조절
void adjustBrightnessTaskCallback() {
    int potValue = analogRead(potPin);  // 가변저항 값 읽기
    brightness = map(potValue, 0, 1023, 5, 255);  // 가변저항 값을 LED 밝기로 변환
}

void trafficLightTaskCallback() {
    if (blinkMode || redOnlyMode || allLedOff) return;  // 깜빡이기 모드, 빨간불만 켜기 모드, 모든 LED 끄기 모드일 때는 실행하지 않음

    unsigned long elapsedTime = millis() - previousMillis;  // 경과 시간

    if (elapsedTime < redDuration) {  
        analogWrite(redLedPin, brightness);  // 빨간LED 켜기
        analogWrite(yellowLedPin, 0);  // 노란LED 끄기
        analogWrite(blueLedPin, 0);  // 파란LED 끄기

        redState = true;  // 빨간LED 상태 업데이트
        yellowState = false;  // 노란LED 상태 업데이트
        blueState = false;  // 파란LED 상태 업데이트
        blueBlinkStarted = false;  // 파란불 깜빡 상태 업데이트
    } 
    else if (elapsedTime < redDuration + yellowDuration) {  
        analogWrite(redLedPin, 0);  // 빨간 LED 끄기
        analogWrite(yellowLedPin, brightness);  // 노란 LED 켜기
        analogWrite(blueLedPin, 0);  // 파란 LED 끄기

        redState = false;  // 빨간 LED 상태 
        yellowState = true;  // 노란 LED 상태 
        blueState = false;  // 파란 LED 상태 
        blueBlinkStarted = false;  // 파란불 깜빡이기 시작 상태 업데이트
    } 
    else if (elapsedTime < redDuration + yellowDuration + blueDuration - 1000) {  
        analogWrite(blueLedPin, brightness);  // 파란 LED 켜기
        analogWrite(redLedPin, 0);  // 빨간 LED 끄기
        analogWrite(yellowLedPin, 0);  // 노란 LED 끄기

        redState = false;  // 빨간 LED 상태 업데이트
        yellowState = false;  // 노란 LED 상태 업데이트
        blueState = true;  // 파란 LED 상태 업데이트
        blueBlinkStarted = false;  // 파란불 깜빡이기 시작 상태 업데이트
    }
    else if (elapsedTime < redDuration + yellowDuration + blueDuration) {  
        if (!blueBlinkStarted) {
            blueBlinkStarted = true;  // 파란불 깜빡이기 시작 상태 업데이트
            blueBlinkTask.restart();  // 파란불 깜빡이기 Task 재시작
        }
    }
    else if (elapsedTime < redDuration + yellowDuration + blueDuration + extraYellowDuration) {  
        analogWrite(redLedPin, 0);  // 빨간 LED끄기
        analogWrite(yellowLedPin, brightness);  // 노란 LED켜기
        analogWrite(blueLedPin, 0);  // 파란 LED끄기

        redState = false;  //빨간LED 상태 업데이트
        yellowState = true;  //노란LED 상태 업데이트
        blueState = false;  //파란LED 상태 업데이트
        blueBlinkStarted = false;  //파란불 깜빡이기 시작 상태 업데이트
    } 
    else {
        previousMillis = millis();  //사이클 재시작
    }

    //신호등 상태 변경 시 시리얼 데이터 전송
    sendSerialData();
}

//파란불 깜빡이기 Task
void blueBlinkTaskCallback() {
    blinkState = !blinkState;  // 깜빡이기 상태 토글
    analogWrite(blueLedPin, blinkState ? brightness : 0);  // 파란 LED 깜빡이기
    Serial.println(blinkState ? "Blue LED ON" : "Blue LED OFF");  // 시리얼 출력

    if (blueBlinkTask.isLastIteration()) {
        analogWrite(blueLedPin, brightness);  // 파란 LED 켜기
    }
}

//깜빡이기 Task (버튼 1 - 깜빡이기 모드)
void blinkTaskCallback() {
    static bool ledState = false;  // LED상태
    if (blinkMode) {
        ledState = !ledState;  // LED토글
        analogWrite(redLedPin, ledState ? brightness : 0);  // 빨간 LED 깜빡이기
        analogWrite(yellowLedPin, ledState ? brightness : 0);  // 노란 LED 깜빡이기
        analogWrite(blueLedPin, ledState ? brightness : 0);  // 파란 LED 깜빡이기
    }
}

void sendSerialData() {
    String currentLight = "Off";  // 현재 켜진 신호등 상태

    int redValue = redState ? 1 : 0;  // 빨간 LED 상태 값
    int yellowValue = yellowState ? 1 : 0;  // 노란 LED 상태 값
    int blueValue = blueState ? 1 : 0;  // 파란 LED 상태 값

    if (mode == "All Blink") {
        currentLight = "All Blinking";  // 모든 LED 깜빡이기 상태
    } else if (mode == "All Off") {
        currentLight = "Off";  // 모든 LED 꺼짐상태
    } else if (mode == "Red Only") {
        currentLight = "Red";  // 빨간불만켜기 상태
    } else if (redState) {
        currentLight = "Red";  // 빨간불 켜짐 상태
    } else if (yellowState) {
        currentLight = "Yellow";  //노란불 켜짐 상태
    } else if (blueBlinkStarted) {
        currentLight = "Blinking";  //파란불 깜빡이기 상태
    } else if (blueState) {
        currentLight = "Blue";  //파란불 켜짐 상태
    }

    //JSON 형식으로 데이터 전송 (밝기 포함)
    Serial.print("{\"Light\":\"");
    Serial.print(currentLight);
    Serial.print("\",\"Red\":");
    Serial.print(redValue);
    Serial.print(",\"Yellow\":");
    Serial.print(yellowValue);
    Serial.print(",\"Blue\":");
    Serial.print(blueValue);
    Serial.print(",\"Mode\":\"");
    Serial.print(mode);
    Serial.print("\",\"Brightness\":");
    Serial.print(brightness);
    Serial.println("}");

    delay(20);  //
}

//버튼 인터럽트 핸들러에서 데이터 전송 추가
void toggleBlinkMode() {
    blinkMode = !blinkMode;  // 깜빡이기 모드 토글
    mode = blinkMode ? "Blink" : "Normal";  // 모드 업데이트
    Serial.println(blinkMode ? "Blink Mode ON" : "Blink Mode OFF");  // 시리얼 출력
    sendSerialData();  // 데이터 즉시 전송
    if (blinkMode) {
        trafficLightTask.disable();  //신호등 Task 비활성화
        blinkTask.enable();  //깜빡이기 Task 활성화
    } else {
        blinkTask.disable();  //깜빡이기 Task 비활성화
        previousMillis = millis();  //현재 시간 저장
        trafficLightTask.enable();  //신호등 Task 활성화
    }
}

void toggleRedOnlyMode() {
    redOnlyMode = !redOnlyMode;  // 빨간불만 켜기 모드 토글
    mode = redOnlyMode ? "Red Only" : "Normal";  // 모드 업데이트
    Serial.println(redOnlyMode ? "Red Only Mode ON" : "Red Only Mode OFF");  // 시리얼 출력
    sendSerialData();  // 데이터 즉시 전송
    if (redOnlyMode) {
        trafficLightTask.disable();  // 신호등 Task 비활성화
        blinkTask.disable();  // 깜빡이기 Task 비활성화
        analogWrite(redLedPin, brightness);  // 빨간 LED 켜기
        analogWrite(yellowLedPin, 0);  // 노란 LED 끄기
        analogWrite(blueLedPin, 0);  // 파란 LED 끄기
    } else {
        previousMillis = millis();  // 현재 시간 저장
        trafficLightTask.enable();  // 신호등 Task 활성화
    }
}

void toggleAllLedOff() {
    allLedOff = !allLedOff;  // 모든 LED 끄기 모드 토글
    mode = allLedOff ? "All Off" : "Normal";  // 모드 업데이트
    Serial.println(allLedOff ? "All LEDs Off" : "Traffic Light Mode ON");  // 시리얼 출력
    sendSerialData();  // 데이터 즉시 전송
    if (allLedOff) {
        trafficLightTask.disable();  // 신호등 Task 비활성화
        blinkTask.disable();  // 깜빡이기 Task 비활성화
        analogWrite(redLedPin, 0);  // 빨간 LED 끄기
        analogWrite(yellowLedPin, 0);  // 노란 LED 끄기
        analogWrite(blueLedPin, 0);  // 파란 LED 끄기
    } else {
        analogWrite(redLedPin, brightness);  // 빨간 LED 켜기
        analogWrite(yellowLedPin, 0);  // 노란 LED 끄기
        analogWrite(blueLedPin, 0);  // 파란 LED 끄기
        previousMillis = millis();  // 현재 시간 저장
        trafficLightTask.enable();  // 신호등 Task 활성화
    }
}