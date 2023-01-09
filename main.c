//Hussam Al Basha & Charbel Chedid
// HTTP Client - MQTT Client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/spi_master.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "mqtt_client.h"
#include "esp_system.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

char url[512];
char topic [50];
char data [50];
char city [50];
char country [50];

char all_data[512];
char buff[900];
char response[1024];
char buffer[50];

static const char *TAG = "MQTT_TCP";
static esp_mqtt_client_handle_t client = NULL;

char* wind_dir(int deg){
    deg = deg % 360;
    if (deg>=337.5 || deg<=22.5)  return "N";
    if (deg>=67.5  || deg<=112.5) return "E";
    if (deg>=157.5 || deg<=202.5) return "S";
    if (deg>=247.5 || deg<=292.5) return "W";    
    if (deg>22.5   || deg<67.5)   return "NE";
    if (deg>112.5  || deg<157.5)  return "SE";
    if (deg>202.5  || deg<247.5)  return "SW";
    if (deg>292.5  || deg<337.5)  return "NW";
}

char* get_day(int i){
    if (i==0) return "Sunday";  
    if (i==1) return "Monday";
    if (i==2) return "Tuesday";
    if (i==3) return "Wednesday";
    if (i==4) return "Thursday";
    if (i==5) return "Friday";
    if (i==6) return "Saturday";
    else
        return "null";
}

char* get_month(int i){
    if (i==0) return "January";  
    if (i==1) return "February";
    if (i==2) return "March";
    if (i==3) return "April";
    if (i==4) return "May";
    if (i==5) return "June";
    if (i==6) return "July";
    if (i==7) return "August";  
    if (i==8) return "September";
    if (i==9) return "October";
    if (i==10)return "November";
    if (i==11)return "December";
    else
        return "null";
}

char* timecon(int h, int m, int s){
    char* x="AM";
    if (h>12) { h=h-12; x="PM";}
    else if (h==0) h=12;
    else if (h==12) x="PM";
    sprintf(buffer, "%d:%d:%d %s",h,m,s,(char*)x);
    return buffer;
}

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        memset(buff,0,sizeof(buff));
        sprintf(buff,(char *) evt->data);
        strncat(response, buff, evt->data_len);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void rest_get()
{
    memset(url,0,sizeof(url));

    sprintf(url, "http://api.openweathermap.org/data/2.5/weather?q=%s,%s&APPID=5eea103de7e3be67647f47537973cf95", (char*) city, (char*) country);
    esp_http_client_config_t config_get = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = client_event_get_handler};
    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
}

static void rest_get_plus(){
    rest_get();
    cJSON *root=cJSON_Parse(response);
    cJSON *sys=cJSON_GetObjectItem(root,"sys");
    cJSON *mainn=cJSON_GetObjectItem(root,"main");
    cJSON *weather=cJSON_GetObjectItem(root,"weather");
    cJSON *subweather=cJSON_GetArrayItem(weather,0);
    cJSON *wind=cJSON_GetObjectItem(root,"wind");

    int timezone=cJSON_GetObjectItem(root,"timezone")->valueint;
    int dt=cJSON_GetObjectItem(root,"dt")->valueint;
    int t=timezone+dt;
    time_t epoch = t;
    char buffs[80];
    struct tm ts ;

    ts = * localtime (& epoch );
    strftime ( buffs , sizeof ( buffs) , "%a %Y-%m-%d %H:%M:%S " , & ts );

    int sec=ts.tm_sec;
    int minutes=ts.tm_min;
    int hour=ts.tm_hour;
    int day=ts.tm_wday;
    int date=ts.tm_mday;
    int month=ts.tm_mon;
    int year=ts.tm_year + 1900;
    char *day1=get_day(day);
    char *month1=get_month(month);
    char* time_converted = timecon(hour, minutes, sec);

    char *country=cJSON_GetObjectItem(sys,"country")->valuestring;
    char *loc=cJSON_GetObjectItem(root,"name")->valuestring;

    double temp=cJSON_GetObjectItem(mainn,"temp")->valuedouble;
    int humidity=cJSON_GetObjectItem(mainn,"humidity")->valueint;
    temp = temp - 273.15;
    //weather description
    char *description=cJSON_GetObjectItem(subweather,"description")->valuestring;
    //wind 
    double speed=cJSON_GetObjectItem(wind,"speed")->valuedouble;
    int deg=cJSON_GetObjectItem(wind,"deg")->valueint;
    speed = speed * 3.6;
    char* winddir= wind_dir(deg);
    cJSON * wind_gust=cJSON_GetObjectItem(wind,"gust");

    if(wind_gust==NULL){
     sprintf(all_data, "Weather conditions in %s, %s  <i class=\"fa fa-map-marker\" aria-hidden=\"true\"></i><br><br>%s, %s %d, %d %s  <i class=\"fa fa-calendar\" aria-hidden=\"true\"></i><br><br>Temperature : %.2f C  <i class=\"fa fa-thermometer-empty\" aria-hidden=\"true<\"></i><br><br>Humidity : %d %%  <i class=\"fa fa-tint\" aria-hidden=\"true\"></i><br><br>Conditions : %s  <i class=\"fa fa-cloud\" aria-hidden=\"true\"></i><br><br>Wind : %.3f km/h from the %s  <i class=\"fa fa-paper-plane\" aria-hidden=\"true\"></i><br><br>"
    , (char*)loc, (char*)country,(char*)day1, (char*)month1, date, year, (char*)time_converted, temp, humidity, (char*)description, speed, (char*)winddir); 
    }
    else{
        double wind_gust=cJSON_GetObjectItem(wind,"gust")->valuedouble;
        sprintf(all_data, "Weather conditions in %s, %s  <i class=\"fa fa-map-marker\" aria-hidden=\"true\"></i><br><br>%s, %s %d, %d %s  <i class=\"fa fa-calendar\" aria-hidden=\"true\"></i><br><br>Temperature : %.2f C  <i class=\"fa fa-thermometer-empty\" aria-hidden=\"true<\"></i><br><br>Humidity : %d %%  <i class=\"fa fa-tint\" aria-hidden=\"true\"></i><br><br>Conditions : %s  <i class=\"fa fa-cloud\" aria-hidden=\"true\"></i><br><br>Wind : %.3f km/h from the %s  <i class=\"fa fa-paper-plane\" aria-hidden=\"true\"></i><br><br>Gust: %.3f m/s<br><br>"
    , (char*)loc, (char*)country, (char*)day1, (char*)month1, date, year, (char*)time_converted, temp, humidity, (char*)description, speed, (char*)winddir,wind_gust);
    }

    // printf("Weather conditions in %s, %s \n", (char*)loc, (char*)country);
    // printf("%s, %s %d, %d %s \n", (char*)day1, (char*)month1, date, year, (char*)time_converted);
    // printf("Temperature : %.2f C \n", temp);
    // printf("Humidity : %d %% \n", humidity);
    // printf("Conditions : %s \n", (char*)description);
    // printf("Wind : %f km/h from the %s \n", speed, (char*)winddir);
}

// MQTT functions

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(event->client, "weather", 0);
        esp_mqtt_client_subscribe(event->client, "city", 0);
        esp_mqtt_client_subscribe(event->client, "country", 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        sprintf(topic, "%.*s", event->topic_len, event->topic);

        printf("%s\n", (char*) topic);
        if(strcmp(topic, "city")==0){
            sprintf(city, "%.*s", event->data_len, event->data);
            //printf("%s\n", (char*) city);
        }
        
        if(strcmp(topic, "country")==0){
            sprintf(country, "%.*s", event->data_len, event->data);
            //printf("%s\n", (char*) country);
        }
        
        if(strcmp(topic, "weather")==0){
            rest_get_plus();
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            esp_mqtt_client_publish(event->client, "weather", all_data, 0,1 , 0);
        }
    
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://mqtt.eclipseprojects.io"
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation                   s1.1
    esp_event_loop_create_default();     // event loop                          s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station                        s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //                                         s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "Hussam",
            .password = "12345678"}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

void app_main(void)
{
    nvs_flash_init();
    wifi_connection();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    mqtt_app_start();
    printf("WIFI was initiated ...........\n\n");    
}


