let port;               // 시리얼 포트 연결 객체
let reader;             // 시리얼 데이터 읽기 객체
let inputDone;          // 입력 스트림 처리 완료 여부
let outputDone;         // 출력 스트림 처리 완료 여부
let inputStream;        // 입력 스트림
let outputStream;       // 출력 스트림
let writer;             // 시리얼 데이터 전송 객체

let potBrightness = 0;  // 가변저항 값 (LED 밝기 조절용)
let mode = "Normal";    // 현재 신호 모드
let currentLight = "Off"; // 현재 켜져 있는 신호등 상태
let isConnected = false; // 시리얼 연결 상태

// 가변저항 값을 표시할 슬라이더 (각 LED별)
let redPotSlider, yellowPotSlider, bluePotSlider;

// 아두이노와 시리얼 포트를 연결하는 함수
async function connectToArduino() {
    try {
        // 시리얼 포트 선택 창을 띄워 사용자에게 포트 선택 요청
        port = await navigator.serial.requestPort();
        
        // 시리얼 포트를 9600 baud 속도로 오픈
        await port.open({ baudRate: 9600 });

        // 입력 스트림 설정 (데이터를 문자열로 변환)
        const textDecoder = new TextDecoderStream();
        inputDone = port.readable.pipeTo(textDecoder.writable);
        inputStream = textDecoder.readable.pipeThrough(new TransformStream(new LineBreakTransformer()));
        reader = inputStream.getReader();

        // 출력 스트림 설정 (데이터를 아두이노로 전송)
        const textEncoder = new TextEncoderStream();
        outputDone = textEncoder.readable.pipeTo(port.writable);
        outputStream = textEncoder.writable;
        writer = outputStream.getWriter();

        // 연결 성공 여부 업데이트
        isConnected = true;
        document.getElementById("status").innerText = "Status: Connected";

        // 시리얼 데이터를 지속적으로 읽는 함수 실행
        readLoop();
    } catch (error) {
        console.error("Error opening the serial port:", error);
    }
}

// 시리얼 데이터를 계속해서 읽어오는 함수
async function readLoop() {
    while (true) {
        try {
            const { value, done } = await reader.read();
            if (done) {
                reader.releaseLock();
                break;
            }
            serialEvent(value); // 읽어온 데이터를 처리하는 함수 호출
        } catch (error) {
            console.error("Error reading from the serial port:", error);
            break;
        }
    }
}

// p5.js에서 UI 요소를 생성하는 setup 함수
function setup() {
    createCanvas(600, 500); // 600x500 크기의 캔버스 생성

    // 각 LED 밝기를 나타내는 슬라이더 (읽기 전용)
    createP("Red LED Light").position(20, 270);
    redPotSlider = createSlider(0, 255, 0);
    redPotSlider.position(20, 310);
    redPotSlider.attribute('disabled', ''); // 슬라이더를 읽기 전용으로 설정

    createP("Yellow LED Light").position(20, 340);
    yellowPotSlider = createSlider(0, 255, 0);
    yellowPotSlider.position(20, 380);
    yellowPotSlider.attribute('disabled', ''); 

    createP("Blue LED Light").position(20, 410);
    bluePotSlider = createSlider(0, 255, 0);
    bluePotSlider.position(20, 450);
    bluePotSlider.attribute('disabled', '');  
}

// 시리얼 데이터를 JSON으로 변환하고 UI에 반영하는 함수
function serialEvent(data) {
    let trimmedData = data.trim(); // 문자열 양쪽 공백 제거
    if (trimmedData.length > 0) {
        try {
            let json = JSON.parse(trimmedData); // JSON 형식으로 변환

            console.log("Received JSON:", json); 

            potBrightness = json.Brightness;  // LED 밝기 값 업데이트
            mode = json.Mode;                 // 현재 모드 업데이트
            currentLight = json.Light;        // 현재 신호등 상태 업데이트

            // 가변저항 슬라이더 값을 받아 업데이트
            redPotSlider.value(potBrightness);
            yellowPotSlider.value(potBrightness);
            bluePotSlider.value(potBrightness);

            redraw(); // 화면을 다시 그려서 변경 사항 반영
        } catch (e) {
            console.error("JSON Parsing Error:", e, "Received Data:", data);
        }
    }
}

// p5.js에서 주기적으로 화면을 그리는 draw 함수
function draw() {
    background(220); // 배경색 설정

    // 현재 신호 모드 표시
    fill(0);
    textSize(20);
    text("Mode: " + mode, 20, 40);

    // LED 상태를 저장할 변수 (기본적으로 꺼진 상태)
    let redCircleColor = color(100);
    let yellowCircleColor = color(100);
    let blueCircleColor = color(100);

    // 현재 모드에 따라 LED 색상을 변경
    if (mode === "All Off") { 
        redCircleColor = color(100);
        yellowCircleColor = color(100);
        blueCircleColor = color(100);
    } else if (mode === "Red Only") {
        redCircleColor = color(255, 0, 0, potBrightness);
    } else if (mode === "Blink") {
        redCircleColor = frameCount % 30 < 15 ? color(255, 0, 0, potBrightness) : color(100);
        yellowCircleColor = frameCount % 30 < 15 ? color(255, 255, 0, potBrightness) : color(100);
        blueCircleColor = frameCount % 30 < 15 ? color(0, 0, 255, potBrightness) : color(100);
    } else if (mode === "All Blink") {
        let blinkColor = frameCount % 30 < 15 ? 255 : 100;
        redCircleColor = color(255, 0, 0, blinkColor);
        yellowCircleColor = color(255, 255, 0, blinkColor);
        blueCircleColor = color(0, 0, 255, blinkColor);
    } else if (mode === "Normal") {
        if (currentLight === "Red") {
            redCircleColor = color(255, 0, 0, potBrightness);
        } else if (currentLight === "Yellow") {
            yellowCircleColor = color(255, 255, 0, potBrightness);
        } else if (currentLight === "Blinking") { 
            blueCircleColor = frameCount % 30 < 15 ? color(0, 0, 255, potBrightness) : color(100);
        } else if (currentLight === "Blue") {
            blueCircleColor = color(0, 0, 255, potBrightness);
        }
    }

    // 신호등 원을 화면에 그림
    fill(redCircleColor);
    ellipse(width / 4, height / 2 - 50, 80, 80);

    fill(yellowCircleColor);
    ellipse(width / 2, height / 2 - 50, 80, 80);

    fill(blueCircleColor);
    ellipse((width / 4) * 3, height / 2 - 50, 80, 80);
}

// 시리얼 데이터에서 줄바꿈을 변환하는 클래스
class LineBreakTransformer {
    constructor() {
        this.container = '';
    }

    transform(chunk, controller) {
        this.container += chunk;
        const lines = this.container.split('\n');
        this.container = lines.pop();
        lines.forEach(line => controller.enqueue(line));
    }

    flush(controller) {
        controller.enqueue(this.container);
    }
}

// HTML 버튼을 추가하여 아두이노 연결을 실행
document.addEventListener("DOMContentLoaded", function() {
    document.getElementById("connectButton").addEventListener("click", () => {
        connectToArduino();
    });
});
