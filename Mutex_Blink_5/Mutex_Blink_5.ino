/*
Program that uses a mutex to increment a variable correctly between two seperate tasks.
Goal: Prevent setuip from exiting using a mutex to prevent setup from exiting before the 
*/

// #include <semphr.h> //for vanilla free RTOS

#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif
//Mutex initialization
SemaphoreHandle_t mutex;

//Task Handles
static TaskHandle_t MyTaskA = NULL;

//Task A:Blink LED
void Task_A(void *parameters) {
  int blk_time = 100;
  while (true) {
    if (xSemaphoreTake(mutex, 0) == pdTRUE) {
      blk_time = *(int *)parameters;
    }
      digitalWrite(2, HIGH);
      vTaskDelay(blk_time / portTICK_PERIOD_MS);
      digitalWrite(2, LOW);
      vTaskDelay(blk_time / portTICK_PERIOD_MS);
      xSemaphoreGive(mutex);
  }
}


void setup() {

  int blink_delay = 100;
  int idx = 0;
  pinMode(2, OUTPUT);
  char mystr[20];
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("ESP32 FreeRTOS Lab\nInitialized Serial Port");
  Serial.println("Type the delay you want for the Blink LED");
  //Create Mutex
  mutex = xSemaphoreCreateMutex();
  if (xSemaphoreTake(mutex, 0) == pdTRUE) {
  } else {
    Serial.println("Error: Could not take mutex");
  }

  while (Serial.available() <= 0) {
    //do nothing but wait for user input
  }
  while (Serial.available() > 0) {
    mystr[idx] = Serial.read();
    idx++;
  }
  blink_delay = atoi(mystr);
  xSemaphoreGive(mutex);
  Serial.print("Blink delay is: ");
  Serial.println(blink_delay);
  //Create Tasks
  xTaskCreatePinnedToCore(Task_A, "Task_A", 1024, (void *)&blink_delay, 1, &MyTaskA, app_cpu);
  vTaskDelete(NULL);
}


void loop() {
}
