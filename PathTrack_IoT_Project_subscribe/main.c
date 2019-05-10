#include "wifi.h"
#include <tinyara/gpio.h>
#include <apps/netutils/mqtt_api.h>
#include <apps/netutils/dhcpc.h>

#include <tinyara/analog/adc.h>
#include <tinyara/analog/ioctl.h>

#include <apps/shell/tash.h>

// for NTP
#include <apps/netutils/ntpclient.h>

// for JSON
#include <apps/netutils/cJSON.h>

//for siren
#include <fcntl.h>
#include <tinyara/pwm.h>

#define DEFAULT_CLIENT_ID "123456789"
#define SERVER_ADDR "api.artik.cloud"
//#define SERVER_ADDR "52.86.204.150"
#define SERVER_PORT 8883
//#define SERVER_PORT 1883 // non-secure mode, Not supported in ARTIK Cloud
#define RED "RED"
#define BLUE "BLUE"
#define GREEN "GREEN"
#define RED_LED 45 // on-board LED
#define GREEN_LED 60
#define BLUE_LED 49 // on-board LED
#define RED_ON_BOARD_LED 45
#define NET_DEVNAME "wl1"
//for siren
#define BBI 1047
#define BBO 784

#define ON 1
#define OFF 0

#define LEAD 7

char device_id[] = "afc78e8a26864877a37b82a36a762742";
char device_token[] = "39d849f28ff44baeb9842c365395cd49";

char *strTopicMsg;
char *strTopicAct;

//for project
//entrance
int entrance_ledPin = 39;
//palor
int palor_ledPin = 41;
int microwave_ledPin = 30;
int refrigerator_ledPin = 40;
//room
int room_ledPin = 59;
int moniter_ledPin = 37;
int dryer_ledPin = 31;
int airconditioner_ledPin = 38;
//toilet
int toilet_ledPin = 58;

int gas_ledPin = 32;
//buzzer
int bbibbo_buzzer[2] = { BBI, BBO };

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

//int blinkerValue = 0;
int currentLED = 0;

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

// Write the value of given gpio port.
void gpio_write(int port, int value) {
   char str[4];
   static char devpath[16];
   snprintf(devpath, 16, "/dev/gpio%d", port);
   int fd = open(devpath, O_RDWR);

   ioctl(fd, GPIOIOC_SET_DIRECTION, GPIO_DIRECTION_OUT);
   write(fd, str, snprintf(str, 4, "%d", value != 0) + 1);

   close(fd);
}
//++Haram
void superPSmode(int OnOffswitch) {
   if (OnOffswitch) {
      gpio_write(gas_ledPin, 0);
      printf("Super PS mode activated\n");
   } else {
      gpio_write(gas_ledPin, 1);
      printf("Super PS mode deactivated\n");
      printf("Welcome!!!\n");
   }
}
void palorPSmode(int OnOffswitch) {
   if (OnOffswitch) {
      gpio_write(microwave_ledPin, 0); //���ڷ����� ������� off -> palor ps mode on
      printf("[palor] PS mode activated\n");
   } else {
      gpio_write(microwave_ledPin, 1); //���ڷ����� ������� on
      printf("[palor] PS mode deactivated\n");
   }
}
void roomPSmode(int OnOffswitch) {
   if (OnOffswitch) {
      gpio_write(dryer_ledPin, 0); //dryer off
      gpio_write(moniter_ledPin, 0); //moniter off
      printf("[room] PS mode activated\n");
   } else {
      gpio_write(dryer_ledPin, 1); //dryer on
      gpio_write(moniter_ledPin, 1); //moniter on
      printf("[room] PS mode deactivated\n");
   }
}
// mqtt client on message callback
// header: <apps/netutils/cJSON.h>
void onMessage(void *client, mqtt_msg_t *msg) {
   int i;
   cJSON *jsonMsg = NULL;
   char *strActName = NULL;
   char *strParameter = NULL;
   char *payload = strdup(msg->payload);
   //for siren buzzer
   int fd = 0;
   struct pwm_info_s pwm_info;

   jsonMsg = cJSON_Parse((const char*) payload);
   cJSON *data = cJSON_GetObjectItem(jsonMsg, "actions");

   if (data == NULL) {
      printf("data is null\n");
      return;
   }

   cJSON *action = cJSON_GetArrayItem(data, 0);
   cJSON *actName = cJSON_GetObjectItem(action, "name");
   cJSON *actParameter = cJSON_GetObjectItem(action, "parameters");

   strActName = cJSON_Print(actName);

   cJSON_Delete(jsonMsg);
   free(strActName);
   free(strParameter);
   free(payload);

   if (strncmp(strActName, "\"CODE99\"", 8) == 0) {
      printf("Disaster detected!\n");
      fd = open("/dev/pwm1", O_RDWR);
      pwm_info.frequency = bbibbo_buzzer[0];
      pwm_info.duty = (8 * 65536) / 100;
      ioctl(fd, PWMIOC_SETCHARACTERISTICS,
            (unsigned long) ((uintptr_t) &pwm_info));
      ioctl(fd, PWMIOC_START, 0);

      gpio_write(gas_ledPin, 0); //��������
      gpio_write(entrance_ledPin, 0);
      gpio_write(palor_ledPin, 0);
      gpio_write(room_ledPin, 0);
      gpio_write(toilet_ledPin, 0); //��� �� �� ����
      gpio_write(refrigerator_ledPin, 0);
      gpio_write(microwave_ledPin, 0); //�Ž� ������ǰ����
      gpio_write(dryer_ledPin, 0);
      gpio_write(moniter_ledPin, 0);
      gpio_write(airconditioner_ledPin, 0); //ħ�� ������ǰ����
   } else if (strncmp(strActName, "\"Earthquake\"", 12) == 0) {
      printf("Earthquake detected\n");
      fd = open("/dev/pwm1", O_RDWR);
      pwm_info.frequency = bbibbo_buzzer[0];
      pwm_info.duty = (8 * 65536) / 100;
      ioctl(fd, PWMIOC_SETCHARACTERISTICS,
            (unsigned long) ((uintptr_t) &pwm_info));
      ioctl(fd, PWMIOC_START, 0);
      gpio_write(gas_ledPin, 0); //��������
      gpio_write(entrance_ledPin, 0);
      gpio_write(palor_ledPin, 0);
      gpio_write(room_ledPin, 0);
      gpio_write(toilet_ledPin, 0); //��� �� �� ����
      gpio_write(refrigerator_ledPin, 0);
      gpio_write(microwave_ledPin, 0); //�Ž� ������ǰ����
      gpio_write(dryer_ledPin, 0);
      gpio_write(moniter_ledPin, 0);
      gpio_write(airconditioner_ledPin, 0); //ħ�� ������ǰ����
   }

   //ENTRANCE
   else if (strncmp(strActName, "\"CODE1\"", 7) == 0) { //���� ó�� �� ���
      printf("[CODE 1]\n");
      gpio_write(entrance_ledPin, 1); //���� �� on
      superPSmode(0); //Super PS mode ����
   } else if (strncmp(strActName, "\"CODE2\"", 7) == 0) { //���� �̹� ����� �ִµ� �ۿ��� �������� ����
      printf("[CODE 2]\n");
      gpio_write(entrance_ledPin, 1); //���� �� on
   } else if (strncmp(strActName, "\"CODE3\"", 7) == 0) { //���� ������ ����� ����
      printf("[CODE 3]\n");
      gpio_write(entrance_ledPin, 0); //���� �� off
      superPSmode(1); //Super PS mode ����
   } else if (strncmp(strActName, "\"CODE4\"", 7) == 0) { //�������� ������ �����µ� ������ �ƹ��� ������ ���� ����� ����
      printf("[CODE 4]\n");
      gpio_write(entrance_ledPin, 0); //���� �� off
   }

   //PALOR
   else if (strncmp(strActName, "\"CODE5\"", 7) == 0) { //�������� �ŽǷ� �̵����� �� ������ �ƹ��� ����.
      printf("[CODE 5]\n");
      gpio_write(entrance_ledPin, 0); //���� �� off
      gpio_write(palor_ledPin, 1); //�Ž� �� on
      palorPSmode(0); //palor PS mode ����
   } else if (strncmp(strActName, "\"CODE6\"", 7) == 0) { //�������� �ŽǷ� �̵����� �� ������ ����� ����.
      printf("[CODE 6]\n");
      gpio_write(palor_ledPin, 1); //�Ž� �� on
      palorPSmode(0); //palor PS mode ����
   } else if (strncmp(strActName, "\"CODE7\"", 7) == 0) { //�Žǿ��� �������� �̵����� �� �Žǿ� �ƹ��� ����
      printf("[CODE 7]\n");
      gpio_write(palor_ledPin, 0); //�Ž� �� off
      palorPSmode(1); //palor PS mode ����
      gpio_write(entrance_ledPin, 1); //���� �� on
   } else if (strncmp(strActName, "\"CODE8\"", 7) == 0) { //�Žǿ��� �������� �̵����� �� �Žǿ� ����� ����
      printf("[CODE 8]\n");
      gpio_write(entrance_ledPin, 1); //���� �� on
   }

   //ROOM
   else if (strncmp(strActName, "\"CODE9\"", 7) == 0) { //�Žǿ��� ������ �̵����� �� �Žǿ� �ƹ��� ����
      printf("[CODE 9]\n");
      gpio_write(palor_ledPin, 0); //�Ž� �� off
      palorPSmode(1); //palor PS mode ����
      gpio_write(room_ledPin, 1); //�� �� on
      roomPSmode(0); //room PS mode ����
   } else if (strncmp(strActName, "\"CODE10\"", 7) == 0) { //�Žǿ��� ������ �̵����� �� �Žǿ� ����� ����
      printf("[CODE 10]\n");
      gpio_write(room_ledPin, 1); //�� �� on
      roomPSmode(0); //room PS mode ����
   } else if (strncmp(strActName, "\"CODE11\"", 7) == 0) { //�濡�� �ŽǷ� �̵����� �� �濡 �ƹ��� ����
      printf("[CODE 11]\n");
      gpio_write(room_ledPin, 0); //�� �� off
      roomPSmode(1); //room PS mode ����
      gpio_write(palor_ledPin, 1); //�Ž� �� on
      palorPSmode(0); //palor PS mode ����
   } else if (strncmp(strActName, "\"CODE12\"", 7) == 0) { //�濡�� �ŽǷ� �̵����� �� �濡 ����� ����
      printf("[CODE 12]\n");
      gpio_write(palor_ledPin, 1); //�Ž� �� on
      palorPSmode(0); //palor PS mode ����
   }

   //TOILET
   else if (strncmp(strActName, "\"CODE13\"", 7) == 0) { //�Žǿ��� ȭ��Ƿ� �̵����� �� �Žǿ� �ƹ��� ����
      printf("[CODE 13]\n");
      gpio_write(palor_ledPin, 0); //�Ž� �� off
      palorPSmode(1); //palor PS mode ����
      gpio_write(toilet_ledPin, 1); //ȭ��� �� on
   } else if (strncmp(strActName, "\"CODE14\"", 7) == 0) { //�Žǿ��� ȭ��Ƿ� �̵����� �� �Žǿ� ����� ����
      printf("[CODE 14]\n");
      gpio_write(toilet_ledPin, 1); //ȭ��� �� on
   } else if (strncmp(strActName, "\"CODE15\"", 7) == 0) { //ȭ��ǿ��� �ŽǷ� �̵����� �� ȭ��ǿ� �ƹ��� ����
      printf("[CODE 15]\n");
      gpio_write(palor_ledPin, 1); //�Ž� �� on
      palorPSmode(0); //palor PS mode ����
      gpio_write(toilet_ledPin, 0); //ȭ��� �� off
   } else if (strncmp(strActName, "\"CODE16\"", 7) == 0) { //ȭ��ǿ��� �ŽǷ� �̵����� �� ȭ��ǿ� ����� ����
      printf("[CODE 16]\n");
      gpio_write(palor_ledPin, 1); //�Ž� �� on
      palorPSmode(0); //palor PS mode ����
   }

   //siren
   else if (strncmp(strActName, "\"SIREN\"", 7) == 0) { //������ ����
      printf("SIREN ACTIVATED\n");
      fd = open("/dev/pwm1", O_RDWR);
//      if (i) {
      pwm_info.frequency = bbibbo_buzzer[1];
      pwm_info.duty = (8 * 65536) / 100;
      ioctl(fd, PWMIOC_SETCHARACTERISTICS,
            (unsigned long) ((uintptr_t) &pwm_info));
      ioctl(fd, PWMIOC_START, 0);

//         i = 0;
//      } else {
//         pwm_info.frequency = bbibbo_buzzer[0];
//         pwm_info.duty = (8 * 65536) / 100;
//         ioctl(fd, PWMIOC_SETCHARACTERISTICS,
//               (unsigned long) ((uintptr_t) &pwm_info));
//         ioctl(fd, PWMIOC_START, 0);
//
//         i = 1;
//      }
   } else if (strncmp(strActName, "\"CODE0\"", 7) == 0) {
      fd = open("/dev/pwm1", O_RDWR);
//      if (i) {
      pwm_info.frequency = bbibbo_buzzer[1];
      pwm_info.duty = (8 * 65536) / 100;
      ioctl(fd, PWMIOC_SETCHARACTERISTICS,
            (unsigned long) ((uintptr_t) &pwm_info));
      ioctl(fd, PWMIOC_START, 0);

//         i = 0;
//      } else {
//         pwm_info.frequency = bbibbo_buzzer[0];
//         pwm_info.duty = (8 * 65536) / 100;
//         ioctl(fd, PWMIOC_SETCHARACTERISTICS,
//               (unsigned long) ((uintptr_t) &pwm_info));
//         ioctl(fd, PWMIOC_START, 0);
//
//         i = 1;
//      }
      up_mdelay(10);
      ioctl(fd, PWMIOC_STOP, 0);
      close(fd);
      printf("[CODE 0]\n");
      gpio_write(refrigerator_ledPin, 1);
      gpio_write(airconditioner_ledPin, 1);
   } else {
      printf("Unrecognized action.\n");
   }
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
   clientConfig.on_message = (void*) onMessage;
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

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int hello_main(int argc, char *argv[])
#endif
{
   bool wifiConnected = false;
   gpio_write(RED_ON_BOARD_LED, 1); // Turn on on board Red LED to indicate no WiFi connection is established

   strTopicMsg = (char*) malloc(sizeof(char) * 256);
   strTopicAct = (char*) malloc(sizeof(char) * 256);
   sprintf(strTopicMsg, "/v1.1/messages/%s", device_id);
   sprintf(strTopicAct, "/v1.1/actions/%s", device_id);

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

// Subscribe to topic of interest
   while (mqttSubscribe == false ) {
      sleep(2);
      int result = mqtt_subscribe(pClientHandle, strTopicAct, 0); //topic - color, QOS - 0
      if (result < 0) {
         printf("mqtt client subscribe to topic failed\n");
         continue;
      }
      mqttSubscribe = true;
      printf("mqtt client Subscribed to the topic successfully\n");
   }

   while (1) {
      up_mdelay(100);
   }
}
