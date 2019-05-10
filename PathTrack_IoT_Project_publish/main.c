#include <stdio.h>
#include <apps/shell/tash.h>

#include <tinyara/gpio.h>
#include <tinyara/config.h>

#include <tinyara/gpio.h>
#include <apps/netutils/mqtt_api.h>
#include <apps/netutils/dhcpc.h>

#include <tinyara/analog/adc.h>
#include <tinyara/analog/ioctl.h>

#include <apps/shell/tash.h>


// for NTP
#include <apps/netutils/ntpclient.h>

//from pubmain
#include "wifi.h"
#include "time.h"
#include <tinyara/gpio.h>
#include <apps/netutils/mqtt_api.h>
#include <apps/netutils/dhcpc.h>

#define DEFAULT_CLIENT_ID "123456789"
#define SERVER_ADDR "api.artik.cloud"
//#define SERVER_ADDR "52.86.204.150"
#define SERVER_PORT 8883
//#define SERVER_PORT 1883 // non-secure mode, Not supported in ARTIK Cloud
#define NET_DEVNAME "wl1"
#define RED_ON_BOARD_LED 45

#define THRESHOLD 30
#define ENTRANCE 1
#define PALOR 2
#define ROOM 3
#define TOILET 4

uint32_t cfgcon_out, cfgcon_in;

//from pubmain
char device_id[] = "245c3b2f85474b7f9d2fe2b6c27b93a6";
char device_token[] = "f0a17ad6261145c48a11fe1ca0717357";

static const char mqtt_ca_cert_str[] = "-----BEGIN CERTIFICATE-----\r\n"
      "MIIGrTCCBZWgAwIBAgIQASAP9e8Tbenonqd/EQFJaDANBgkqhkiG9w0BAQsFADBN\r\n"
      "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5E\r\n"
      "aWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTgwMzA4MDAwMDAwWhcN\r\n"
      "MjAwNDA1MTIwMDAwWjBzMQswCQYDVQQGEwJVUzETMBEGA1UECBMKQ2FsaWZvcm5p\r\n"
      "YTERMA8GA1UEBxMIU2FuIEpvc2UxJDAiBgNVBAoTG1NhbXN1bmcgU2VtaWNvbmR1\r\n"
      "Y3RvciwgSW5jLjEWMBQGA1UEAwwNKi5hcnRpay5jbG91ZDCCASIwDQYJKoZIhvcN\r\n"
      "AQEBBQADggEPADCCAQoCggEBANghNaTXWDfYV/JWgBnX4hmhcClPSO0onx5B2url\r\n"
      "YzpvTc3MBaQ+08YBpAKvTqZvPqrJUIM45Q91M301I5e2kz0DMq2zQZOGB0B83V/O\r\n"
      "O4vwETq4PCjAPhMinF4dN6HeJCuqo1CLh8evhfkFiJvpEfQWTxdjzPJ0Zdj/2U8E\r\n"
      "8Ht7zV5pWiDtuejtIDHB5H6fCx4xeQy/E+5l4V6R3BnRKpZsJtlhTh0RFqWhw5DJ\r\n"
      "/WWpGP//1VTZSHyW9SABsPd+jP1YgDraRD4b4lZBU6c8nC5qT3dhdiYoG6xUgTb3\r\n"
      "kfgUhhlOFpe3sBtR32OS8RuFrFeQDGaa3r6pfSy06Kph/eECAwEAAaOCA2EwggNd\r\n"
      "MB8GA1UdIwQYMBaAFA+AYRyCMWHVLyjnjUY4tCzhxtniMB0GA1UdDgQWBBSNBf6r\r\n"
      "7S/j0oV3A0XmEflXErutQDAlBgNVHREEHjAcgg0qLmFydGlrLmNsb3VkggthcnRp\r\n"
      "ay5jbG91ZDAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsG\r\n"
      "AQUFBwMCMGsGA1UdHwRkMGIwL6AtoCuGKWh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNv\r\n"
      "bS9zc2NhLXNoYTItZzYuY3JsMC+gLaArhilodHRwOi8vY3JsNC5kaWdpY2VydC5j\r\n"
      "b20vc3NjYS1zaGEyLWc2LmNybDBMBgNVHSAERTBDMDcGCWCGSAGG/WwBATAqMCgG\r\n"
      "CCsGAQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5jb20vQ1BTMAgGBmeBDAEC\r\n"
      "AjB8BggrBgEFBQcBAQRwMG4wJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2lj\r\n"
      "ZXJ0LmNvbTBGBggrBgEFBQcwAoY6aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29t\r\n"
      "L0RpZ2lDZXJ0U0hBMlNlY3VyZVNlcnZlckNBLmNydDAJBgNVHRMEAjAAMIIBfwYK\r\n"
      "KwYBBAHWeQIEAgSCAW8EggFrAWkAdgCkuQmQtBhYFIe7E6LMZ3AKPDWYBPkb37jj\r\n"
      "d80OyA3cEAAAAWIHFb1dAAAEAwBHMEUCIQCQ0UjVVJSQDRB3oxzI5aD1Hs5GhbXj\r\n"
      "I6Cqt3/tkXT1WQIgNVWRgbJ72Ik9gp5QoNxhCZ+h//or0uL7PHnv3cP5L9UAdgBv\r\n"
      "U3asMfAxGdiZAKRRFf93FRwR2QLBACkGjbIImjfZEwAAAWIHFb73AAAEAwBHMEUC\r\n"
      "IQDxCxJCsZjuqbQvuwipgdUf1l6qXdiekM5zn33i1+KYxgIgKDMJEuKHzhkweT2S\r\n"
      "Y4dWBuzSdOAzZfoDrIGdsFvkxi0AdwC72d+8H4pxtZOUI5eqkntHOFeVCqtS6BqQ\r\n"
      "lmQ2jh7RhQAAAWIHFb1YAAAEAwBIMEYCIQCNDYdxWmqUGGwNzXlJ1/NXxzwqPYIB\r\n"
      "eSJDuR1xfWtSsQIhAJsygf2rqPS+O7qQAzggCQ2V/3JDRUhuxNDPqwooo47uMA0G\r\n"
      "CSqGSIb3DQEBCwUAA4IBAQBvRGWibvHFrRUWsArJ9lmS5MMZFbXXQPXbflgv3nSG\r\n"
      "ShmhBC3o+k97J0Wgp/wH7uDf01RrRMAVNm458g1Mr4AMAXq3zzxNNTwjGYw/USuG\r\n"
      "UprrKqc9onugtAUX8DGvlZr8SWO3FhPlyamWQ69jutx/X4nfHyZr41bX9WQ/ay0F\r\n"
      "GQJ1tRTrX1eUPO+ucXeG8vTbt09bRNnoY+i97dzrwHakXySfHohNsIbwmrsS4SQv\r\n"
      "7eG9g5+5vsc2B9ugGcELIYKrzDWNPshir37KSpcwLUCmDJkTQp8+KhJUKgbTALTa\r\n"
      "nxuDyNwZIwW66vv1t0Zi4vKU8hfUsAN2N3wcsb6pY/RA\r\n"
      "-----END CERTIFICATE-----\r\n"
      "-----BEGIN CERTIFICATE-----\r\n"
      "MIIElDCCA3ygAwIBAgIQAf2j627KdciIQ4tyS8+8kTANBgkqhkiG9w0BAQsFADBh\r\n"
      "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
      "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
      "QTAeFw0xMzAzMDgxMjAwMDBaFw0yMzAzMDgxMjAwMDBaME0xCzAJBgNVBAYTAlVT\r\n"
      "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxJzAlBgNVBAMTHkRpZ2lDZXJ0IFNIQTIg\r\n"
      "U2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\r\n"
      "ANyuWJBNwcQwFZA1W248ghX1LFy949v/cUP6ZCWA1O4Yok3wZtAKc24RmDYXZK83\r\n"
      "nf36QYSvx6+M/hpzTc8zl5CilodTgyu5pnVILR1WN3vaMTIa16yrBvSqXUu3R0bd\r\n"
      "KpPDkC55gIDvEwRqFDu1m5K+wgdlTvza/P96rtxcflUxDOg5B6TXvi/TC2rSsd9f\r\n"
      "/ld0Uzs1gN2ujkSYs58O09rg1/RrKatEp0tYhG2SS4HD2nOLEpdIkARFdRrdNzGX\r\n"
      "kujNVA075ME/OV4uuPNcfhCOhkEAjUVmR7ChZc6gqikJTvOX6+guqw9ypzAO+sf0\r\n"
      "/RR3w6RbKFfCs/mC/bdFWJsCAwEAAaOCAVowggFWMBIGA1UdEwEB/wQIMAYBAf8C\r\n"
      "AQAwDgYDVR0PAQH/BAQDAgGGMDQGCCsGAQUFBwEBBCgwJjAkBggrBgEFBQcwAYYY\r\n"
      "aHR0cDovL29jc3AuZGlnaWNlcnQuY29tMHsGA1UdHwR0MHIwN6A1oDOGMWh0dHA6\r\n"
      "Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RDQS5jcmwwN6A1\r\n"
      "oDOGMWh0dHA6Ly9jcmw0LmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RD\r\n"
      "QS5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEWHGh0dHBzOi8v\r\n"
      "d3d3LmRpZ2ljZXJ0LmNvbS9DUFMwHQYDVR0OBBYEFA+AYRyCMWHVLyjnjUY4tCzh\r\n"
      "xtniMB8GA1UdIwQYMBaAFAPeUDVW0Uy7ZvCj4hsbw5eyPdFVMA0GCSqGSIb3DQEB\r\n"
      "CwUAA4IBAQAjPt9L0jFCpbZ+QlwaRMxp0Wi0XUvgBCFsS+JtzLHgl4+mUwnNqipl\r\n"
      "5TlPHoOlblyYoiQm5vuh7ZPHLgLGTUq/sELfeNqzqPlt/yGFUzZgTHbO7Djc1lGA\r\n"
      "8MXW5dRNJ2Srm8c+cftIl7gzbckTB+6WohsYFfZcTEDts8Ls/3HB40f/1LkAtDdC\r\n"
      "2iDJ6m6K7hQGrn2iWZiIqBtvLfTyyRRfJs8sjX7tN8Cp1Tm5gr8ZDOo0rwAhaPit\r\n"
      "c+LJMto4JQtV05od8GiG7S5BNO98pVAdvzr508EIDObtHopYJeS4d60tbvVS3bR0\r\n"
      "j6tJLp07kzQoH3jOlOrHvdPJbRzeXDLz\r\n"
      "-----END CERTIFICATE-----\r\n";

// mqtt client handle
mqtt_client_t* pClientHandle = NULL;

// mqtt client parameters
mqtt_client_config_t clientConfig;

//typedef struct _mqtt_tls_param_t {
//   const unsigned char *ca_cert;   /* CA certificate, common between client and MQTT Broker */
//   const unsigned char *cert;   /* Client certificate */
//   const unsigned char *key;   /* Client private key */
//   int ca_cert_len;         /* the length of CA certificate  */
//   int cert_len;            /* the length of Client certificate */
//   int key_len;            /* the length of key */
//} mqtt_tls_param_t;

mqtt_tls_param_t clientTls;

struct ntpc_server_conn_s g_server_conn[2];

const unsigned char *get_ca_cert(void) {
   return (const unsigned char*) mqtt_ca_cert_str;
}

// mqtt client on connect callback
void onConnect(void* client, int result) {
   printf("mqtt client connected to the server\n");
}

// mqtt client on disconnect callback
void onDisconnect(void* client, int result) {
   printf("mqtt client disconnected from the server\n");
}

// mqtt client on publish callback
void onPublish(void* client, int result) {
   printf("mqtt client Published message\n");
}

static int gpio_read(int port) {
   char buf[4];
   char devpath[16];
   snprintf(devpath, 16, "/dev/gpio%d", port);
   int fd = open(devpath, O_RDWR);

   ioctl(fd, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_IN);
   if (read(fd, buf, sizeof(buf)) < 0) {
      printf("read error\n");
      return -1;
   }
   close(fd);

   return buf[0] == '1';
}

void gpio_write(int port, int value) {
   char str[4];
   static char devpath[16];
   snprintf(devpath, 16, "/dev/gpio%d", port);
   int fd = open(devpath, O_RDWR);

   ioctl(fd, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);
   write(fd, str, snprintf(str, 4, "%d", value != 0) + 1);

   close(fd);
}

// Utility function to configure mqtt client
void initializeConfigUtil(void) {
   uint8_t macId[IFHWADDRLEN];
   int result = netlib_getmacaddr("wl1", macId);
   if (result < 0) {
      printf(
            "Get MAC Address failed. Assigning \
                Client ID as 123456789");
      clientConfig.client_id =
      DEFAULT_CLIENT_ID; // MAC id Artik 053
   } else {
      printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", ((uint8_t *) macId)[0],
            ((uint8_t *) macId)[1], ((uint8_t *) macId)[2],
            ((uint8_t *) macId)[3], ((uint8_t *) macId)[4],
            ((uint8_t *) macId)[5]);
      char buf[12];
      sprintf(buf, "%02x%02x%02x%02x%02x%02x", ((uint8_t *) macId)[0],
            ((uint8_t *) macId)[1], ((uint8_t *) macId)[2],
            ((uint8_t *) macId)[3], ((uint8_t *) macId)[4],
            ((uint8_t *) macId)[5]);
      clientConfig.client_id = buf; // MAC id Artik 053
      printf("Registering mqtt client with id = %s\n", buf);
   }

   clientConfig.user_name = device_id;
   clientConfig.password = device_token;
   clientConfig.debug = true;
   clientConfig.on_connect = (void*) onConnect;
   clientConfig.on_disconnect = (void*) onDisconnect;
   //clientConfig.on_message = (void*) onMessage;
   clientConfig.on_publish = (void*) onPublish;

   clientConfig.protocol_version = MQTT_PROTOCOL_VERSION_311;
   clientConfig.clean_session = true;

   clientTls.ca_cert = get_ca_cert();
   clientTls.ca_cert_len = sizeof(mqtt_ca_cert_str);
   clientTls.cert = NULL;
   clientTls.cert_len = 0;
   clientTls.key = NULL;
   clientTls.key_len = 0;

   clientConfig.tls = &clientTls;
}

static void ntp_link_error(void) {
   printf("ntp_link_error() callback is called.\n");
}

//for SmartHome
struct SmartHome {
   int outterPin; //바깥쪽 FC 센서 pin number
   int innerPin; //안쪽 FC 센서 pin number
   int outter; //바깥쪽 FC 센서 pin read
   int inner; //안쪽 FC 센서 pin read
   int count; //방 안에 들어간 사람의 수
   int timer; //파워세이빙 모드 작동을 위해 count가 0인 시간을 체크
};

struct SmartHome SH[5] = { { 0, 0, 0, 0, 0, 0 }, { 31, 37, 0, 0, 0, 0 }, //ENTRANCE
      { 40, 38, 0, 0, 0, 0 },   //PALOR
      { 39, 41, 0, 0, 0, 0 },   //ROOM
      { 58, 59, 0, 0, 0, 0 }   //TOILET
};
int home_count = 0;
int safety_mode = 1;
int switchPin = 44;
int code = 0;

void message_publishing(int ret, int code, char *strTopicMsg) {
   dispClock();
   printf(
         "[CODE %d / SAFETY %d]  [HOME : %d] [ENTRANCE : %d, LIVING ROOM : %d, BAD ROOM : %d, BATH ROOM : %d]\n",
         code, safety_mode, home_count, SH[ENTRANCE].count, SH[PALOR].count,
         SH[ROOM].count, SH[TOILET].count);
   char buf[40];
   sprintf(buf, "{\"code\" : %d,\"safety_mode\" : %d}", code, safety_mode);
   mqtt_msg_t message;
   message.payload = (char*) buf;
   message.payload_len = strlen(buf);
   message.topic = strTopicMsg;
   message.qos = 0;
   message.retain = 0;

   ret = mqtt_publish(pClientHandle, message.topic, (char*) message.payload,
         message.payload_len, message.qos, message.retain);
   if (ret < 0) {
      printf("Error publishing \n");
   }
}

//방 들의 타이머를 재주고, timer 초과되면 false return, 초과되기 전에 감지되면 true return
bool TimerCheck(int direction, int roomNumber) {
   printf("room : %d direction : %d waiting for signal\n", roomNumber,
         direction);
   while (SH[roomNumber].timer < THRESHOLD) {
      SH[roomNumber].timer++;
      if (direction) {   //들어오는 방향
         if (!gpio_read(SH[roomNumber].innerPin)) {
            SH[roomNumber].timer = 0;
            return true ;
         }
      } else {   //나가는 방향
         if (!gpio_read(SH[roomNumber].outterPin)) {
            SH[roomNumber].timer = 0;
            return true ;
         }
      }
      up_mdelay(100);
   }
   SH[roomNumber].timer = 0;
   printf("%d room timer overflow!(1:ENTRANCE, 2:PALOR, 3:ROOM, 4:TOILET)\n",
         roomNumber);
   return false ;
}
//entrance의 센서를 읽어서 count를 control, direction은 방향을 뜻하며, 밖에서 안으로 들어오면 1, 나가면 0
bool entranceSensor(int direction) {
   if (TimerCheck(direction, ENTRANCE)) {
      if (direction) {   //들어오는 방향
         home_count++;
         SH[ENTRANCE].count++;
      } else {   //나가는 방향
         home_count--;
         SH[ENTRANCE].count--;
      }
      return true ;
   } else {
      return false ;
   }
}
//entrance에서 행하는 일
void entranceFunction(int direction, int ret, char *strTopicMsg) {
   if (direction) {   //들어오는 방향
      //집에 처음 온 사람
      if (home_count == 1) {
         code = 1;   //거실 PSmode 해제, welcome 시스템 가동
         printf("[home] Welcome!\n");
      }
      //집에 사람이 있는데 밖에서 현관으로 들어옴
      else {
         code = 2;
      }
      message_publishing(ret, code, strTopicMsg);
      up_mdelay(1000);
   } else {   //나가는 방향
      //집의 마지막 사람이 나감
      if (home_count == 0) {
         code = 3;
         safety_mode = 1;
         printf("[home] super PS mode activated\n");
         message_publishing(ret, code, strTopicMsg);
         up_mdelay(1000);
      }
      //현관에서 밖으로 나갔는데 현관에 아무도 없지만 집에 사람이 있음
      else if (SH[ENTRANCE].count == 0) {
         code = 4;
         message_publishing(ret, code, strTopicMsg);
         up_mdelay(1000);
      }
      //message_publishing(ret, code, strTopicMsg);
      //up_mdelay(1000);
   }
}
//palor의 센서를 읽어서 count를 control, direction은 방향을 뜻하며, 밖에서 안으로 들어오면 1, 나가면 0
bool palorSensor(int direction) {
   if (TimerCheck(direction, PALOR)) {
      if (direction) {   //들어오는 방향
         SH[ENTRANCE].count--;
         SH[PALOR].count++;
      } else {   //나가는 방향
         SH[ENTRANCE].count++;
         SH[PALOR].count--;
      }
      return true ;
   } else {
      return false ;
   }
}
//palor에서 행하는 일
void palorFunction(int direction, int ret, char *strTopicMsg) {
   if (direction) {
      //현관에서 거실로 이동했을 때 현관에 아무도 없음.
      if (SH[ENTRANCE].count == 0) {
         code = 5;
      }
      //현관에서 거실로 이동했을 때 현관에 사람이 있음.
      else {
         code = 6;
      }
      message_publishing(ret, code, strTopicMsg);
      up_mdelay(1000);
   } else {
      //거실에서 현관으로 이동했을 때 거실에 아무도 없음
      if (SH[PALOR].count == 0) {
         code = 7;   //인포메이션LCD(사람구별?)
      }
      //거실에서 현관으로 이동했을 때 거실에 사람이 있음
      else {
         code = 8;   //인포메이션LCD (사람구별?)
      }
      message_publishing(ret, code, strTopicMsg);
      up_mdelay(1000);
   }
}
//room의 센서를 읽어서 count를 control, direction은 방향을 뜻하며, 밖에서 안으로 들어오면 1, 나가면 0
bool roomSensor(int direction) {
   if (TimerCheck(direction, ROOM)) {
      if (direction) {   //들어오는 방향
         SH[PALOR].count--;
         SH[ROOM].count++;
      } else {   //나가는 방향
         SH[PALOR].count++;
         SH[ROOM].count--;
      }
      return true ;
   } else {
      return false ;
   }
}
//room에서 행하는 일
void roomFunction(int direction, int ret, char *strTopicMsg) {
   if (direction) {
      //거실에서 방으로 이동했을 때 거실에 아무도 없음
      if (SH[PALOR].count == 0) {
         code = 9;
      }
      //거실에서 방으로 이동했을 때 거실에 사람이 있음
      else {
         code = 10;
      }
      message_publishing(ret, code, strTopicMsg);
      up_mdelay(1000);
   } else {
      //방에서 거실로 이동했을 때 방에 아무도 없음
      if (SH[ROOM].count == 0) {
         code = 11;
      }
      //방에서 거실로 이동했을 때 방에 사람이 있음
      else {
         code = 12;
      }
      message_publishing(ret, code, strTopicMsg);
      up_mdelay(1000);
   }
}
//toilet의 센서를 읽어서 count를 control, direction은 방향을 뜻하며, 밖에서 안으로 들어오면 1, 나가면 0
bool toiletSensor(int direction) {
   if (TimerCheck(direction, TOILET)) {
      if (direction) {   //들어오는 방향
         SH[PALOR].count--;
         SH[TOILET].count++;
      } else {   //나가는 방향
         SH[PALOR].count++;
         SH[TOILET].count--;
      }
      return true ;
   } else {
      return false ;
   }
}
//toilet에서 행하는 일
void toiletFunction(int direction, int ret, char *strTopicMsg) {
   if (direction) {
      //거실에서 화장실로 이동했을 때 거실에 아무도 없음
      if (SH[PALOR].count == 0) {
         code = 13;   //if(거실TV)then(거실 TV off, 화장실 TV 연동)
      }
      //거실에서 화장실로 이동했을 때 거실에 사람이 있음
      else {
         code = 14;   //if(거실TV)then(거실 TV off, 화장실 TV 연동)
      }
      message_publishing(ret, code, strTopicMsg);
      up_mdelay(1000);
   } else {
      //화장실에서 거실로 이동했을 때 화장실에 아무도 없음
      if (SH[TOILET].count == 0) {
         code = 15; //if(화장실 TV) then(거실 TV연동)
      }
      //화장실에서 거실로 이동했을 때 화장실에 사람이 있음
      else {
         code = 16;
      }
      message_publishing(ret, code, strTopicMsg);
      up_mdelay(1000);
   }
}

//도둑이 집안에서 움직인 경로를 표시해주는 모드
void pathTrack(int ret, char *strTopicMsg) {

   while (gpio_read(42)) {
      SH[ROOM].inner = !gpio_read(SH[ROOM].innerPin); //FCPin에 감지되면  1, Clear 이면 0
      SH[ROOM].outter = !gpio_read(SH[ROOM].outterPin);
      SH[PALOR].inner = !gpio_read(SH[PALOR].innerPin);
      SH[PALOR].outter = !gpio_read(SH[PALOR].outterPin);
      SH[TOILET].inner = !gpio_read(SH[TOILET].innerPin);
      SH[TOILET].outter = !gpio_read(SH[TOILET].outterPin);
      SH[ENTRANCE].inner = !gpio_read(SH[ENTRANCE].innerPin);
      SH[ENTRANCE].outter = !gpio_read(SH[ENTRANCE].outterPin);

      if (SH[ENTRANCE].inner && SH[ENTRANCE].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
         up_mdelay(1000);
         continue;
      }
      if (SH[PALOR].inner && SH[PALOR].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
         up_mdelay(1000);
         continue;
      }
      if (SH[ROOM].inner && SH[ROOM].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
         up_mdelay(1000);
         continue;
      }
      if (SH[TOILET].inner && SH[TOILET].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
         up_mdelay(1000);
         continue;
      }

      if (SH[ENTRANCE].outter) {
         int direction = 1;
         if (entranceSensor(direction)) {
            code = -1;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      if (SH[ENTRANCE].inner) {
         int direction = 0;
         if (entranceSensor(direction)) {
            code = -8;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      if (SH[PALOR].outter) {
         int direction = 1;
         if (palorSensor(direction)) {
            code = -2;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      if (SH[PALOR].inner) {
         int direction = 0;
         if (palorSensor(direction)) {
            code = -7;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      if (SH[ROOM].outter) {
         int direction = 1;
         if (roomSensor(direction)) {
            code = -3;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      if (SH[ROOM].inner) {
         int direction = 0;
         if (roomSensor(direction)) {
            code = -4;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      if (SH[TOILET].outter) {
         int direction = 1;
         if (toiletSensor(direction)) {
            code = -5;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      if (SH[TOILET].inner) {
         int direction = 0;
         if (toiletSensor(direction)) {
            code = -6;
            message_publishing(ret, code, strTopicMsg);
            up_mdelay(1000);
         }
      }
      up_mdelay(100);
   }
   //도둑 침입 상황 종료시
   //코드 0 집 초기화
   code = 0;
   message_publishing(ret, code, strTopicMsg);
}

#ifdef CONFIG_BUILT_KERNEL
int main(int argc, FAR char *argv[])
#else
int hello_main(int argc, FAR char *argv[])
#endif
{
   //for mqtt connect
   bool wifiConnected = false;
   gpio_write(RED_ON_BOARD_LED, 1); // Turn on on board Red LED to indicate no WiFi connection is established

   char *strTopicMsg = (char*) malloc(sizeof(char) * 256);
   char *strTopicAct = (char*) malloc(sizeof(char) * 256);
   sprintf(strTopicMsg, "/v1.1/messages/%s", device_id);
   sprintf(strTopicAct, "/v1.1/actions/%s", device_token);

   memset(&clientConfig, 0, sizeof(clientConfig));
   memset(&clientTls, 0, sizeof(clientTls));

   // for NTP Client
   memset(&g_server_conn, 0, sizeof(g_server_conn));
   g_server_conn[0].hostname = "0.asia.pool.ntp.org";
   g_server_conn[0].port = 123;
   g_server_conn[1].hostname = "1.asia.pool.ntp.org";
   g_server_conn[1].port = 123;

#ifdef CONFIG_CTRL_IFACE_FIFO
   int ret;

   while (!wifiConnected) {
      ret = mkfifo(CONFIG_WPA_CTRL_FIFO_DEV_REQ,
      CONFIG_WPA_CTRL_FIFO_MK_MODE);
      if (ret != 0 && ret != -EEXIST) {
         printf("mkfifo error for %s: %s", CONFIG_WPA_CTRL_FIFO_DEV_REQ,
               strerror(errno));
      }
      ret = mkfifo(CONFIG_WPA_CTRL_FIFO_DEV_CFM,
      CONFIG_WPA_CTRL_FIFO_MK_MODE);
      if (ret != 0 && ret != -EEXIST) {
         printf("mkfifo error for %s: %s", CONFIG_WPA_CTRL_FIFO_DEV_CFM,
               strerror(errno));
      }

      ret = mkfifo(CONFIG_WPA_MONITOR_FIFO_DEV, CONFIG_WPA_CTRL_FIFO_MK_MODE);
      if (ret != 0 && ret != -EEXIST) {
         printf("mkfifo error for %s: %s", CONFIG_WPA_MONITOR_FIFO_DEV,
               strerror(errno));
      }
#endif

      if (start_wifi_interface() == SLSI_STATUS_ERROR) {
         printf("Connect Wi-Fi failed. Try Again.\n");
      } else {
         wifiConnected = true;
         gpio_write(RED_ON_BOARD_LED, 0); // Turn off Red LED to indicate WiFi connection is established
      }
   }

   printf("Connect to Wi-Fi success\n");

   bool mqttConnected = false;
   bool ipObtained = false;
   printf("Get IP address\n");

   struct dhcpc_state state;
   void *dhcp_handle;

   while (!ipObtained) {
      dhcp_handle = dhcpc_open(NET_DEVNAME);
      ret = dhcpc_request(dhcp_handle, &state);
      dhcpc_close(dhcp_handle);

      if (ret != OK) {
         printf("Failed to get IP address\n");
         printf("Try again\n");
         sleep(1);
      } else {
         ipObtained = true;
      }
   }
   netlib_set_ipv4addr(NET_DEVNAME, &state.ipaddr);
   netlib_set_ipv4netmask(NET_DEVNAME, &state.netmask);
   netlib_set_dripv4addr(NET_DEVNAME, &state.default_router);

   printf("IP address  %s\n", inet_ntoa(state.ipaddr));

   up_mdelay(2000);

   int ret_ntp = ntpc_start(g_server_conn, 2, 1000, ntp_link_error);
   printf("ret: %d\n", ret_ntp);

   // Connect to the WiFi network for Internet connectivity
   printf("mqtt client tutorial\n");

   // Initialize mqtt client
   initializeConfigUtil();

   pClientHandle = mqtt_init_client(&clientConfig);
   if (pClientHandle == NULL) {
      printf("mqtt client handle initialization fail\n");
      return 0;
   }

   while (mqttConnected == false ) {
      sleep(2);
      // Connect mqtt client to server
      int result = mqtt_connect(pClientHandle, SERVER_ADDR, SERVER_PORT, 60);

      if (result == 0) {
         mqttConnected = true;
         printf("mqtt client connected to server\n");
         break;
      } else {
         continue;
      }
   }

   bool mqttSubscribe = false;

   up_mdelay(2000);
   message_publishing(ret, 0, strTopicMsg);   //Sub 보드 초기화 코드 (0) 퍼블리싱
   printf("\n\n\n***************smart home system*****************\n\n\n\n");
   while (1) {

      SH[ROOM].inner = !gpio_read(SH[ROOM].innerPin); //FCPin에 감지되면  1, Clear 이면 0
      SH[ROOM].outter = !gpio_read(SH[ROOM].outterPin);
      SH[PALOR].inner = !gpio_read(SH[PALOR].innerPin);
      SH[PALOR].outter = !gpio_read(SH[PALOR].outterPin);
      SH[TOILET].inner = !gpio_read(SH[TOILET].innerPin);
      SH[TOILET].outter = !gpio_read(SH[TOILET].outterPin);
      SH[ENTRANCE].inner = !gpio_read(SH[ENTRANCE].innerPin);
      SH[ENTRANCE].outter = !gpio_read(SH[ENTRANCE].outterPin);

      if (!gpio_read(switchPin)) { //아틱보드의 44번 스위치를 누르면 safety mode 해제
         safety_mode = 0;
      }

      if (safety_mode == 0) {   //safety mode 작동 해제

         if (SH[ENTRANCE].inner && SH[ENTRANCE].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
            printf("[entrance] error : double detected\n");
            up_mdelay(1000);
            continue;
         }
         if (SH[PALOR].inner && SH[PALOR].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
            printf("[palor] error : double detected\n");
            up_mdelay(1000);
            continue;
         }
         if (SH[ROOM].inner && SH[ROOM].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
            printf("[room] error : double detected\n");
            up_mdelay(1000);
            continue;
         }
         if (SH[TOILET].inner && SH[TOILET].outter) { //만약 두 개의 센서에 동시에 감지된다면 - 오류이므로 무시해야함
            printf("[toilet] error : double detected\n");
            up_mdelay(1000);
            continue;
         }

         //++Haram entrance
         if (SH[ENTRANCE].outter) {   //밖에서 집으로 들어옴
            int direction = 1;
            if (entranceSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               entranceFunction(direction, ret, strTopicMsg);
            }
         }
         if (SH[ENTRANCE].inner) {   //집에서 나감
            int direction = 0;
            if (entranceSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               entranceFunction(direction, ret, strTopicMsg);
            }
         }

         //++Haram palor
         if (SH[PALOR].outter) {   //현관에서 거실로 들어옴
            int direction = 1;
            if (palorSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               palorFunction(direction, ret, strTopicMsg);
            }
         }
         if (SH[PALOR].inner) {   //거실에서 현관으로 나감
            int direction = 0;
            if (palorSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               palorFunction(direction, ret, strTopicMsg);
            }
         }

         //++Haram room
         if (SH[ROOM].outter) {   //거실에서 방으로 들어옴
            int direction = 1;
            if (roomSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               roomFunction(direction, ret, strTopicMsg);
            }
         }
         if (SH[ROOM].inner) {  //방에서 거실로 나감, if(취침시간) then(유도등)
            int direction = 0;
            if (roomSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               roomFunction(direction, ret, strTopicMsg);
            }
         }

         //++Haram toilet
         if (SH[TOILET].outter) {   //거실에서 화장실로 들어옴
            int direction = 1;
            if (toiletSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               toiletFunction(direction, ret, strTopicMsg);
            }
         }
         if (SH[TOILET].inner) {   //화장실에서 거실로 나감
            int direction = 0;
            if (toiletSensor(direction)) { //다음 센서를 체크하고 센서에 걸리면 함수 실행
               toiletFunction(direction, ret, strTopicMsg);
            }
         }

         if (home_count == 0) {   //집에 아무도 없음
            //safety mode 설정
            //super PS mode 설정
            printf("[home] super PS mode activated\n");
         }
      }

      else {   //safety mode가 작동 중
         printf("safety mode alert\n");
         // 도둑으로 간주, path track mode on
         if (SH[ROOM].inner || SH[ROOM].outter || SH[PALOR].inner
               || SH[PALOR].outter || SH[TOILET].inner || SH[TOILET].outter
               || SH[ENTRANCE].inner || SH[ENTRANCE].outter) {
            printf("Thief!!!!!  path tracking strart\n\n");
            up_mdelay(1000);
            pathTrack(ret, strTopicMsg);   //집에 아무도 없음
            printf("Thief gone\n\n");
            home_count = SH[1].count = SH[2].count = SH[3].count =
                  SH[4].count = 0;
            up_mdelay(1000);
         }

      }
      up_mdelay(100); //전체 loop의 딜레이
   }
}
