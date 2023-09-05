/*Program that uses one task to blink the lED and one task to control the blinking rate of the LED
When you enter a number the blinking led should now be delayed by that time
*/
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Pins
static const int led_pin = 2;
static int led_delay = 100;
static const int BUF_LEN = 10;
//String
const char msg[] = "Barkadeer brig Arr booty rum. ";

//Task Handles
static TaskHandle_t task_flicker_LED = NULL;
static TaskHandle_t task_LED_Speed = NULL;

//Tasks
void flicker_LED(void *parameters) {

  //Print string to Terminal
  while (1) {
    digitalWrite(2, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(2, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
  }
}

void set_LED_Speed(void *parameter) {
  char str[BUF_LEN] = { '\0' };
  memset(str, 0, BUF_LEN);
  char c;
  int count = 0;
  while (1) {
    while (Serial.available() > 0) {
      c = Serial.read();
      if (c == '\n') {
        led_delay = atoi(str);
        Serial.print("Updated delay time to: ");
        Serial.println(led_delay);
        memset(str, 0, BUF_LEN);
        count = 0;
      } else {

        if (count < BUF_LEN - 1) {
          str[count] = c;
          count++;
        }
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  pinMode(led_pin, OUTPUT);
  Serial.print("Setup and loop task running on core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));

  //(function to be called, name of task, stack size (bytes in arduino), parameter to pass to function, task priority, task handle, which core for task)
  xTaskCreatePinnedToCore(flicker_LED, "Flicker LED", 1024, NULL, 1, &task_flicker_LED, app_cpu);
  xTaskCreatePinnedToCore(set_LED_Speed, "SET LED SPEED", 1024, NULL, 1, &task_LED_Speed, app_cpu);
  vTaskDelete(NULL);
}

void loop() {
}
