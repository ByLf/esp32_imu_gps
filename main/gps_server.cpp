#include "gps_server.h" 
#include "data_sample.h"

#include <string>
#include <vector>

#define ECHO_UART_PORT_NUM 2
#define BUF_SIZE 1024
#define ECHO_TEST_TXD 17
#define ECHO_TEST_RXD 16

#define ECHO_TEST_RTS UART_PIN_NO_CHANGE
#define ECHO_TEST_CTS UART_PIN_NO_CHANGE

enum class MsgType {
  GSV,
  GLL,
  UNDEFINED
};



struct GSV {
  int numMsg;
  int msgNum;
  int numSv;
  
  void print() {
   printf("GSV || numMsg: %d msgNum: %d numSv: %d\n", numMsg, msgNum, numSv);   
  }
};

// struct Degree {
//   int degree;
//   float minute;
//   
//   void parse(char *data) {
//     
//   }
// };

struct GLL {
    float lat;
    char ns;
    float lon;
    char ew;
    char status;
    
    void print() {
     printf("GLL || lat: %4.8f ns: %c lon: %4.8f ew: %c status: %c\n", lat, ns, lon, ew, status);    
    }
    
    std::string print_str() {
        return "GLL || lat: " + std::to_string(lat) + " ns " + ns + " lon: " + std::to_string(lon) + " ew: " + ew + " status: " + status;
    }
};

std::string last_msg;

class GPS {
    
};

bool firstCompare(int &shift, char *data, char *tmp) {
 bool ok = true;
 shift = 0;
 while(*data != 0 && *tmp != 0) {
  ok = ok && (*data == *tmp);
  data++;
  tmp++;
  shift++;
 }
 ok = ok && ((*data == 0 && *tmp == 0) || (*data != 0 && *tmp == 0));
 return ok;
}

MsgType parseMsgType(int &shift, char *data) {
 shift = 0;
 while(*data != 0) {
     int loc_shift;
//      if(firstCompare(loc_shift, data, "$GPGSV")) {
//       return MsgType::GSV;   
//      }
     if(firstCompare(loc_shift, data, "$GPGLL")) {
      return MsgType::GLL;   
     }
     data++;
     shift++;
 }
 return MsgType::UNDEFINED;
}

int find(char *data, char val) {
    int res = 0;
 while(*data != 0) {
    if(*data == val) {
     return res + 1;   
    }
    data++;
    res++;
 }
 return -1;
}

int alloc_val(char *dst, char *data, int buf_size) {
 int vals = find(data, ',');
 if(vals == -1)
     return -1;
 vals = vals - 1;
 
 memset(dst, 0, buf_size);
 memcpy(dst, data, vals);
 return vals + 1;
}



void GSVParse(GSV &dst, char *data) {
    const int buf_size = 80;
    char buf[buf_size];
    
    int shift = find(data, ',');
    if(shift == -1)
        return;
    
    data = data + shift;
    
    data = data + alloc_val(buf, data, buf_size);
    dst.numMsg = atoi(buf);
    
    data = data + alloc_val(buf, data, buf_size);
    dst.msgNum = atoi(buf);
    
    data = data + alloc_val(buf, data, buf_size);
    dst.numSv = atoi(buf);
}

void GLLParse(GLL &dst, char *data) {
    const int buf_size = 80;
    char buf[buf_size];
    
    int shift = find(data, ',');
    if(shift == -1)
        return;
    
    data = data + shift;
    
    shift = alloc_val(buf, data, buf_size);
    if(shift == -1)
        return;
    data = data + shift;
    dst.lat = atof(buf);
    
    shift = alloc_val(buf, data, buf_size);
    if(shift == -1)
        return;
    data = data + shift;
    dst.ns = buf[0];
    
    shift = alloc_val(buf, data, buf_size);
    if(shift == -1)
        return;
    data = data + shift;
    dst.lon = atoi(buf);
    
    shift = alloc_val(buf, data, buf_size);
    if(shift == -1)
        return;
    data = data + shift;
    dst.ew = buf[0];
    
    shift = alloc_val(buf, data, buf_size);
    if(shift == -1)
        return;
    data = data + shift; //time
//     dst.ew = atoi(buf);
    
    
    shift = alloc_val(buf, data, buf_size);
    if(shift == -1)
        return;
    data = data + shift;
    dst.status = buf[0];
}

static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    QueueHandle_t uart_queue;
    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE, BUF_SIZE, 10, &uart_queue, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    
    char data[BUF_SIZE];

    while (1) {
        memset(data, 0, BUF_SIZE);
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        
        int shift = 0;
        MsgType msg = parseMsgType(shift, data);
//         if(msg == MsgType::GSV) {
//          printf("Find GSV message at %d\n", shift);   
//          GSV gsv;
//          GSVParse(gsv, data + shift);
//          gsv.print();
//          
//         }
        if(msg == MsgType::GLL) {
         printf("Find GLL message at %d\n", shift);
         
         GLL gll;
         GLLParse(gll, data + shift);
         gll.print();
         
         last_sample.gps.lat = gll.lat;
         last_sample.gps.lon = gll.lon;
         last_sample.gps.have_signal = gll.status == 'A';
        }
        printf("%s", data);
        
//         Write data back to the UART
//         uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
    }
}

void gps_server_init() {
    xTaskCreate(echo_task, "uart_echo_task", 2048 * 2, NULL, 10, NULL);
}
