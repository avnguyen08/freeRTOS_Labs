/*Program that takes in serial and produces it to serial output. Also takes in special serial sequence to
activate delay feature. Mimicing a command line interface.
sending serial "Delay 400"  will delay blink LED by 400 ms
Queue 1: Takes in delay commands
Queue 2: Takes in serial to be outputted to serial port
*/
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const uint8_t MSG_QUE_LEN = 20;
// Pins
static const int led_pin = 2;
static int led_delay = 100;
static const int BUF_LEN = 20;
//String
const char msg[] = "Barkadeer brig Arr booty rum. ";
//Pointer that will point to heap
char *str_heap = nullptr;
//Task Handles
static TaskHandle_t task_Read_Serial = NULL;
static TaskHandle_t task_blink_LED = NULL;
//Queue Handle
static QueueHandle_t serial_queue = xQueueCreate(MSG_QUE_LEN, BUF_LEN * sizeof(char));  //holds strings to be displayed to output
static QueueHandle_t delay_queue = xQueueCreate(MSG_QUE_LEN, sizeof(int));              //holds delay to be used for blink timer
//Tasks
/*initialize variables
store inputs from serial monitor onto buffer array.
*/

//Task A: Reads serial input and prints it. Prints Queue 2 string. If "delay ###" then pass it to queue 1
void Read_Serial(void *parameters) {
  //Reads Serial then stores into char array in heap memory
  char str[BUF_LEN] = { '\0' };
  memset(str, 0, BUF_LEN);
  char c;
  char arg_delay[7] = "delay ";  //delay string comparision. add one to size for null character
  char serial_out[BUF_LEN];
  int count = 0;            //index
  int delay_ind = 6;        //delay index
  char str_delay[BUF_LEN];  //string holding delay number
  int led_delay = 100;      //led delay number
  while (1) {
    if (Serial.available() > 0) {
      c = Serial.read();
      if (count < BUF_LEN - 1) {
        str[count] = c;
        count++;
      }
      if (c == '\n') {
        str[count - 1] = '\0';  //once a \n char is hit, replace the \n with \0
        //if string contains "delay ###" in first indexes, send ### to queue 1
        if (strncmp(str, arg_delay, 6) == 0) {
          delay_ind = strlen(arg_delay);
          //while loop to store string numbers into led_delay
          while (delay_ind < strlen(str)) {
            str_delay[delay_ind - 6] = str[delay_ind];
            delay_ind++;
          }
          delay_ind = 0;
          led_delay = atoi(str_delay);
          memset(&str_delay, 0, sizeof(str_delay));
          Serial.println(led_delay);
          xQueueSend(delay_queue, &led_delay, 0);
          Serial.println("sent queue");
        }

        str_heap = (char *)(pvPortMalloc((count) * sizeof(char)));  //create heap
        memcpy(str_heap, str, count);                               //copy stack string to heap string
        count = 0;                                                  //reset index back to the start

        //print contents in str_heap to serial
        Serial.println(str_heap);
        vPortFree(str_heap);  //deallocate str_heap
        str_heap = nullptr;   //remove dangling pointer
      }
    }
    //if string in queue 2, pop it into serial_out and send it to serial port
    if (xQueueReceive(serial_queue, serial_out, 0) == pdTRUE) {
      Serial.println(serial_out);
    }
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
}
//Task B: Pop queue 1 and update blink led delay. Every 100 led blinks, send Blinked to queue 2.
void delay_task(void *parameter) {

  int rec_delay = 100;
  int blink_count = 0;
  char str_blink[] = "Blinked";
  while (1) {
    xQueueReceive(delay_queue, &rec_delay, 0);  //updating delay time if update available
                                                //blinks led
    digitalWrite(led_pin, HIGH);
    vTaskDelay(rec_delay / portTICK_PERIOD_MS);  //PORT TICK DEFAULT 1MS, VTASK DELAY EXPECTS NUMBER OF TICKS TO DELAY NOT MS
    digitalWrite(led_pin, LOW);
    blink_count++;
    vTaskDelay(rec_delay / portTICK_PERIOD_MS);

    if (blink_count % 100 == 0) {
      xQueueSend(serial_queue, &str_blink, 0);
    }
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
  xTaskCreatePinnedToCore(delay_task, "blinks LED", 1024, NULL, 1, &task_blink_LED, app_cpu);
  vTaskDelete(NULL);
}


void loop() {
}
