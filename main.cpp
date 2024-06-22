#include "mbed.h"
#include "rtos.h"

// Define pins for sensor, LED, buzzer, and motor driver
#define MQ2_SENSOR_PIN A0
#define LED_PIN LED1
#define BUZZER_PIN D7
#define MOTOR_IN1_PIN D6
#define MOTOR_IN2_PIN D5

// Define threshold for LPG gas detection
#define LPG_THRESHOLD 20



// Initialize DigitalOut objects for LED, buzzer, and motor driver
DigitalOut led(LED_PIN);
DigitalOut buzzer(BUZZER_PIN);
DigitalOut motor_in1(MOTOR_IN1_PIN);
DigitalOut motor_in2(MOTOR_IN2_PIN);

// Initialize AnalogIn object for MQ2 sensor
AnalogIn mq2_sensor(MQ2_SENSOR_PIN);

// Semaphore for sensor value update
Semaphore sem_sensor(1);

// Shared variable for sensor value
volatile float lpg_value = 0;

// Function prototypes
void read_mq2_sensor();
void handle_led();
void handle_buzzer();
void handle_motor();

// RTOS threads
Thread thread_sensor(osPriorityHigh);
Thread thread_motor(osPriorityHigh);
Thread thread_led(osPriorityNormal);
Thread thread_buzzer(osPriorityNormal);

// MQ2 sensor handler
void read_mq2_sensor() {
    while (true) {
        sem_sensor.acquire();
        lpg_value = mq2_sensor.read() * 1000; // Convert to range 0-1000
        printf("***************************************************************************\n");
        printf("Sensor task (Priority: High): LPG value = %f\n\n", lpg_value);
        sem_sensor.release();
        ThisThread::sleep_for(500ms);
    }
}

// LED handler
void handle_led() {
    while (true) {
        sem_sensor.acquire();
        bool lpg_detected = (lpg_value >= LPG_THRESHOLD);
        sem_sensor.release();

        if (lpg_detected) {
            led = 1; // Turn on LED
            printf("LED task (Priority: Normal): LED turned ON\n\n");
        } else {
            led = 0; // Turn off LED
            printf("LED task (Priority: Normal): LED turned OFF\n\n");
        }
        ThisThread::sleep_for(500ms);
    }
}

// Buzzer handler
void handle_buzzer() {
    while (true) {
        sem_sensor.acquire();
        bool lpg_detected = (lpg_value >= LPG_THRESHOLD); //assert if the threshold LPG value is exceeded
        sem_sensor.release();

        if (lpg_detected) {
            buzzer = !buzzer; // Turn on buzzer
            printf("Buzzer task (Priority: Normal): Buzzer turned ON\n\n");
        } else {
            buzzer = 0; // Turn off buzzer
            printf("Buzzer task (Priority: Normal): Buzzer turned OFF\n\n");
        }
        ThisThread::sleep_for(500ms);
    }
}

// Motor (fan) handler
void handle_motor() {
    while (true) {
        sem_sensor.acquire();
        bool lpg_detected = (lpg_value >= LPG_THRESHOLD);
        sem_sensor.release();

        if (lpg_detected) {
            motor_in1 = 1; // Turn on motor
            motor_in2 = 0;
            printf("Motor task (Priority: High): Motor turned ON\n\n");
        } else {
            motor_in1 = 0; // Turn off motor
            motor_in2 = 0;
            printf("Motor task (Priority: High): Motor turned OFF\n\n");
        }
        ThisThread::sleep_for(500ms);
    }
}

int main() {
    printf("Main thread started\n");

    // Start RTOS threads
    thread_sensor.start(read_mq2_sensor);
    thread_motor.start(handle_motor);
    thread_led.start(handle_led);
    thread_buzzer.start(handle_buzzer);

    // Main thread can perform other tasks or just idle
    while (true) {
        ThisThread::sleep_for(1000ms);
    }
}
