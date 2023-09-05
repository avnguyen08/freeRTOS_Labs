//Blinks LED using freeRTOS

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Pins
static const int led_pin = 2;

void toggleLED(void *parameter) {
  while(1) {
    digitalWrite(led_pin, HIGH );
    vTaskDelay(500/ portTICK_PERIOD_MS); //PORT TICK DEFAULT 1MS, VTASK DELAY EXPECTS NUMBER OF TICKS TO DELAY NOT MS
    digitalWrite(led_pin, LOW );
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print("hello");

  }
}
void toggleLEDfaster(void *parameter) {
  while(1) {
    digitalWrite(led_pin, HIGH );
    vTaskDelay(200/ portTICK_PERIOD_MS); //PORT TICK DEFAULT 1MS, VTASK DELAY EXPECTS NUMBER OF TICKS TO DELAY NOT MS
    digitalWrite(led_pin, LOW );
    vTaskDelay(200 / portTICK_PERIOD_MS);
    Serial.print("hello");

  }
}

void setup() {
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  //(function to be called, name of task, stack size (bytes in arduino), parameter to pass to function, task priority, task handle, which core for task)
  xTaskCreatePinnedToCore(toggleLED, "Toggle LED", 1024, NULL, 1, NULL, app_cpu);
   xTaskCreatePinnedToCore(toggleLEDfaster, "Toggle LED faster", 1024, NULL, 1, NULL, app_cpu);
}

void loop() {
  // put your main code here, to run repeatedly:

}
