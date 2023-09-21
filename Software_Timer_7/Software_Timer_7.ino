/**
 * FreeRTOS Software Timer Challenge
 * 
 * Challenge: Simulate a dim screen after 5 seconds feature with a software timer. have the led on be the screen
 * and the led off being the dimmed screen
 * 
 */


// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif
// Pins
static const int led_pin = 2;

// Globals
static TimerHandle_t timer_serial = NULL;

//*****************************************************************************
// Tasks

// Read Serial: Reads Serial port, if activity, turn on led, if no activity for 5 seconds, turn off led.
void activity_check(void *parameters) {
  //Check if data in serial port
  digitalWrite(led_pin, HIGH);
  xTimerStart(timer_serial, portMAX_DELAY);
  while (1) {
    char c;
    if (Serial.available() > 0) {
      c = Serial.read();
      digitalWrite(led_pin, HIGH);
      xTimerStart(timer_serial, portMAX_DELAY);
    }
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

void turn_off_LED(void *parameters) {
  digitalWrite(led_pin, LOW);
}

//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {
  //Configure led pin as output
  pinMode(led_pin, OUTPUT);
  // Configure Serial
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Starting Serial Port");
  //Timer for serial port
  timer_serial = xTimerCreate("Serial Timer", 5000 / portTICK_PERIOD_MS, pdFALSE, (void *)0, turn_off_LED);
  if (timer_serial == NULL) {
    Serial.println("Error: Timer serial creation failed");
  }

  xTaskCreatePinnedToCore(activity_check,
                          "Serial Activity",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
  // Delete self task
  vTaskDelete(NULL);
}

void loop() {

  // Do nothing but allow yielding to lower-priority tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}