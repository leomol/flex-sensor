/**
 * @brief Detect changes in a couple of flex sensors
 * (https://www.adafruit.com/product/182).
 * Application: Measure subjects resting behavior.
 *
 * @file FlexSensor.ino
 * @author Leonardo Molina (leonardomt@gmail.com)
 * @date 2018-08-21
 * @version: 0.1.180824
**/

const uint8_t pin1 = A8;	//< Analog pin for sensor 1.
const uint8_t pin2 = A9;	//< Analog pin for sensor 2.
const uint16_t step = 50;	//< Integration time.

uint32_t next = step;		//  Report due this time.
int16_t delta = 0;			//  Current change.
int16_t last1 = 0;			//  Last value for sensor 1.
int16_t last2 = 0;			//  Last value for sensor 2.
int16_t lastD = 0;			//  Last change reported.

void setup() {
	// Communicate over serial port at the given baudrate.
	Serial.begin(115200);
	
	// Set target bits as output.
	pinMode(A0, OUTPUT);
	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	pinMode(A4, OUTPUT);
	pinMode(A5, OUTPUT);
	pinMode(A6, OUTPUT);
	pinMode(A7, OUTPUT);
}

void loop() {
	// Largest change in a time step.
	int16_t current1 = analogRead(pin1);
	int16_t current2 = analogRead(pin2);
	delta = max(delta, sqrt((current1 - last1) * (current1 - last1) + (current2 - last2) * (current2 - last2)));
	if (millis() >= next) {
		// Serial port: Only print if delta changed.
		if (delta != lastD) {
			Serial.println(delta);
			lastD = delta;
			setPort(min(0xFF, delta));
		}
		
		// Reset values.
		delta = 0;
		last1 = current1;
		last2 = current2;
		// Schedule next change.
		next = millis() + step;
	}
}

#if defined(__AVR__)

	void setPort(uint8_t value) {
		// Port K/F: Write delta in every iteration.
		PORTF = value;
	}

#elif defined(__SAM3X8E__)

	void setPort(uint8_t value) {
		const uint32_t maskW = 0x1C1005C;
		const uint32_t maskR = 0xFE3EFFA3;
		
		//                        A0, A1, A2, A3, A4, A5, A6, A7
		const uint8_t shifts[] = {16, 24, 23, 22,  6,  4,  3,  2};
		// Leave other bits unchanged.
		uint32_t target = PIOA -> PIO_PDSR;
		for (int s = 0; s < 8; s++)
			target = bitWrite(target, shifts[s], bitRead(value, s));
		// Apply changes.
		PIOA -> PIO_CODR = maskW;
		PIOA -> PIO_SODR = target;
	}
	
#endif