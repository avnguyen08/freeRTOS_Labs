/*ESP32 WROOM Hardware Interrupt Challenge
* Program that takes in serial data like a command line
* when "avg" is typed int cmd line. The average of the last 10 adc samples are displayed
*
*
*
*/
//Debugging mode statements. Helps differentiate serial lines from serial lines only used for debugging.
#define DEBUG 0
//if in debugging mode then make each of these debug functions the same as a serial print.
#if DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
//all debug lines are equal to nothing
#else
#define debug(x)
#define debugln(x)
#endif
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif
//Globals
const uint STR_MAX = 20;   //max size of serial input
const uint BUF_SIZE = 10;  //size of eachdouble buffer
float adc_avg = 0;
const int adc_pin = 35;
volatile bool flag_buf = 0;
volatile int buf_idx = 0;
volatile int buffer1[BUF_SIZE] = { 0 };  //1st buffer
volatile int buffer2[BUF_SIZE] = { 0 };  //2nd buffer
static TaskHandle_t task_cmd_line = NULL;
static TaskHandle_t task_compute_avg = NULL;
static hw_timer_t* timer = NULL;
static SemaphoreHandle_t bin_sem;  // Waits for parameter to be written/read
static SemaphoreHandle_t mutex;    // Waits for parameter to be written/read

// Settings
static const uint16_t timer_divider = 8;
static const uint64_t timer_max_count = 1000000;

//Interrupt
void IRAM_ATTR Sample_Timer() {
  BaseType_t task_woken = pdFALSE;
  //write to buffer statements
  if (flag_buf == true) {
    buffer1[buf_idx] = analogRead(adc_pin);
  } else if (flag_buf == false) {
    buffer2[buf_idx] = analogRead(adc_pin);
  }
  buf_idx++;
  //flip buffer flag after buffer is full and reset index back to zero
  if (buf_idx >= BUF_SIZE) {
    flag_buf = !flag_buf;
    buf_idx = 0;
    // debugln("I'm in timer, pos 3");
    //give semaphore? For task b to start reading buffer?
    xSemaphoreGiveFromISR(bin_sem, &task_woken);
    //wakeup/call compute avg function
  }
  // Exit from ISR (ESP-IDF)
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

//Tasks
//Task A: Read data buffer, average data inside, and make it adc_avg new value
void compute_avg(void* parameters) {

  while (1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    //read buffer

    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
      if (flag_buf == true) {
        adc_avg = avg_of_array(buffer2, BUF_SIZE);  // average buffer 2
      } else if (flag_buf == false) {
        adc_avg = avg_of_array(buffer1, BUF_SIZE);  // average buffer 1
      }
    }
    else {
      Serial.println("Error: Unable to obtain mutex");
    }

    xSemaphoreGive(mutex);
  }
}
//Task B: Reads Serial input and echoes it. If Serial input is "avg", Displays
//        the average of last 10 samples.
void cmd_line(void* parameters) {
  char temp = 0;
  char str[STR_MAX] = { '\0' };  //maximum serial input of STR_MAX characters
  int counter = 0;
  while (1) {
    if (Serial.available() > 0) {
      temp = Serial.read();
      if (temp != '\n') {
        str[counter] = temp;
        counter++;
      } else if (temp == '\n') {
        str[counter] = '\0';
        Serial.println(str);

        //CMD LINE
        // "avg": prints the average value
        if (strncmp(str, "avg", 3) == 0) {
          //Critical Section
          //prints average to serial
          Serial.print("Average: ");
          xSemaphoreTake(mutex, 20);
          Serial.println(adc_avg);
          xSemaphoreGive(mutex);
        }
        counter = 0;
        memset(&str, 0, sizeof(str));
      }
      //Prevents over indexing
      if (counter + 1 >= STR_MAX) {
        Serial.println("String too big");
        counter = 0;
        memset(&str, 0, sizeof(str));
      }
    }
    //reset string and counter
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
float avg_of_array(volatile int* arr, int size) {
  float sum = 0;
  for (int i = 0; i < size; ++i) {
    sum = sum + arr[i];
  }
  return sum / size;
}
//function
void setup() {
  Serial.begin(115200);  //Serial initialization
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Finished Initialization");
  // Create binary semaphore
  bin_sem = xSemaphoreCreateBinary();
  if (bin_sem == NULL) {
    Serial.println("Error: failed to create binary semaphore");
  }
  // Create mutex
  mutex = xSemaphoreCreateMutex();
  if (mutex == NULL) {
    Serial.println("Error: failed to create mutex");
  }
  xSemaphoreGive(mutex);

  // Create and start timer (num, divider, countUp)
  timer = timerBegin(0, timer_divider, true);
  // Provide ISR to timer (timer, function, edge)
  timerAttachInterrupt(timer, &Sample_Timer, true);
  // At what count should ISR trigger (timer, count, autoreload)
  timerAlarmWrite(timer, timer_max_count, true);
  // Allow ISR to trigger
  timerAlarmEnable(timer);

  xTaskCreatePinnedToCore(cmd_line, "command line", 1024, NULL, 1, &task_cmd_line, app_cpu);
  xTaskCreatePinnedToCore(compute_avg, "Compute Average", 1024, NULL, 1, &task_compute_avg, app_cpu);
  Serial.println("Finished Task Creation");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  vTaskDelete(NULL);
}

void loop() {
}
