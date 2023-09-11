/*
Program that uses a mutex to increment a variable correctly between two seperate tasks.
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
static TaskHandle_t MyTaskB = NULL;

//Task A
void Task_A(void *parameters) {

}
//Task B
void Task_B(void *parameters) {

}


void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/ portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("Initialized Serial Port");

  //Create Mutex
  mutex = xSemaphoreCreateMutex();
  //Create Tasks
    xTaskCreatePinnedToCore(Task_A, "Task_A", 1024, NULL, 1, &MyTaskA, app_cpu);
    xTaskCreatePinnedToCore(Task_B, "Task_B", 1024, NULL, 1, &MyTaskB, app_cpu);
}


void loop() {
}
