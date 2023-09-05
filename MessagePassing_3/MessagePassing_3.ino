/*Program that takes in serial input and sends it back out to serial port. Using FreeRTOS multitasking.
*/
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// Pins
static const int led_pin = 2;
static int led_delay = 100;
static const int BUF_LEN = 20;
//String
const char msg[] = "Barkadeer brig Arr booty rum. ";
//flag between task
static bool flag = 0;
//Pointer that will point to heap
char *str_heap = nullptr;
//Task Handles
static TaskHandle_t task_Read_Serial = NULL;
static TaskHandle_t task_Print_to_Serial = NULL;

//Tasks
/*initialize variables
store inputs from serial monitor onto buffer array.
once a \n char is hit, call print to serial task.
*/
void Read_Serial(void *parameters) {
  //Reads Serial then stores into char array in heap memory
  char str[BUF_LEN] = { '\0' };
  memset(str, 0, BUF_LEN);
  char c;
  int count = 0;
  while (1) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (count < BUF_LEN - 1) {
        str[count] = c;
        count++;
      }
      if (c == '\n') {
        str[count - 1] = '\0';
        if (flag == 0) {
          str_heap = (char *)(pvPortMalloc((count) * sizeof(char)));  //dont i want count + 1? -------------------Aaron----------------------
          memcpy(str_heap, str, count);                               //length is index + 1
          flag = 1;
          count = 0;
        }
      }
    }
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}


void Print_to_Serial(void *parameter) {
  while (1) {
    if (flag == 1) {
      //print contents in str_heap to serial
      Serial.println(str_heap);
      vPortFree(str_heap); //deallocate str_heap
      str_heap = nullptr; //remove dangling pointer
      flag = 0;  //turn flag to activate read serial.
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



void setup() {
  Serial.begin(115200);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  pinMode(led_pin, OUTPUT);
  Serial.print("Setup and loop task running on core ");
  Serial.print(xPortGetCoreID());
  Serial.print(" with priority ");
  Serial.println(uxTaskPriorityGet(NULL));

  //(function to be called, name of task, stack size (bytes in arduino), parameter to pass to function, task priority, task handle, which core for task)
  xTaskCreatePinnedToCore(Read_Serial, "Reads Serial", 1024, NULL, 1, &task_Read_Serial, app_cpu);
  xTaskCreatePinnedToCore(Print_to_Serial, "Prints to Serial", 1024, NULL, 1, &task_Print_to_Serial, app_cpu);
  vTaskDelete(NULL);
}

void loop() {
}
