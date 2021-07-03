#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "lwip/opt.h"

#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif

#include "lwip/err.h"
#include "lwip/sys.h"


#define MY_DNS_IP_ADDR 0x08080808 // 8.8.8.8

// WIFI CONFIGURATION
#define ESP_AP_SSID "espnewthre"
#define ESP_AP_PASS "twtracker"

#define EXAMPLE_ESP_WIFI_SSID      "twtest1"
#define EXAMPLE_ESP_WIFI_PASS      "twtest@123"

#define EXAMPLE_ESP_MAXIMUM_RETRY  3


char *payload = {"hello welcome"};



//mqqt
#include "mqtt_client.h"

esp_mqtt_client_handle_t client;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);

esp_mqtt_client_config_t mqtt_cfg = { .uri = "mqtt://demos.trackerwave.com",

		.username = "twdemo", .password = "demo@2018", .event_handle = mqtt_event_handler,

//		.cert_pem = "-----BEGIN CERTIFICATE-----\n"
//						"MIIC+jCCAeKgAwIBAgIQdzsvZg0v6KZD1vZJWhKapTANBgkqhkiG9w0BAQsFADA5\n"
//						"klGCn/6DMMQjMDH4bcZ8iv9TrBeaduOXhVJs1iQjcSkgtHaPAo+32AwDfRQlZA==\n"
//						"-----END CERTIFICATE-----\n",
//		.client_cert_pem = NULL,
//		.client_key_pem = NULL,
	//	.clientkey_password = "1234",

// .username =
//"gwuser", .password = "tw@2018",
// .task_prio = 3,
// .transport = MQTT_TRANSPORT_OVER_TCP,
// .buffer_size = 3 * 1024,
// .task_stack = 6 * 1024,
// .user_context = (void *)your_context,
// .disable_auto_reconnect = false,

		};
void tw_mqtt_start();

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "wifi apsta";

static int s_retry_num = 0;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch(event->event_id) {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	 tw_mqtt_start();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    {
      //if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
	esp_wifi_connect();
	xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	//s_retry_num++;
	ESP_LOGI(TAG,"retry to connect to the AP");
     // }
      ESP_LOGI(TAG,"connect to the AP failed");
      break;
    }
  case SYSTEM_EVENT_AP_STACONNECTED:
    ESP_LOGI(TAG,"station connected");
    break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    ESP_LOGI(TAG,"station disconnected");
    break;
  default:
    break;
  }
  return ESP_OK;
}

void tw_mqtt_start() {


		ESP_LOGE(TAG, "-------Initiating MQTT Instance----------");
		client = esp_mqtt_client_init(&mqtt_cfg);
		esp_mqtt_client_start(client);

}


void mqtt_task(void *pvParameters)
{

	while(1)
	{
	//char * dPayload;

	if (client != NULL) {

	//dPayload = calloc(300 * 10, sizeof(char));


	// ESP_LOGI(TAG, "PUSHING-------");

     esp_mqtt_client_publish(client,"tw/r/client" , payload, 0, 0,
             							0);

		vTaskDelay(2000 / portTICK_PERIOD_MS);

	}
	}

}



static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			//sending_mastermsg_Tocloud(payload);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
           msg_id = esp_mqtt_client_publish(client, "/sweety", "helloworld", 0, 0, 0);
           ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
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


void wifi_init_sta()
{
    ip_addr_t dnsserver;
    //tcpip_adapter_dns_info_t dnsinfo;
	


    s_wifi_event_group = xEventGroupCreate();
	//  ESP_ERROR_CHECK(esp_netif_init());

  //  ESP_ERROR_CHECK(esp_event_loop_create_default());
  // esp_netif_create_default_wifi_sta();
  // esp_netif_create_default_wifi_ap();
	

   tcpip_adapter_init();
	
	
	
	// esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();

  //  esp_netif_dhcpc_stop(my_sta);

   // esp_netif_ip_info_t ip_info;

  // IP4_ADDR(&ip_info.ip, 192, 168, 43, 198);
  //	IP4_ADDR(&ip_info.gw, 192, 168, 43, 1);
 // 	IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
  
  	

 // esp_netif_set_ip_info(my_sta, &ip_info);


	
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* ESP STATION CONFIG */
    wifi_config_t wifi_config = {
	.sta = {
	    .ssid = EXAMPLE_ESP_WIFI_SSID,
	    .password = EXAMPLE_ESP_WIFI_PASS
	},
    };

    /* ESP AP CONFIG */
    wifi_config_t ap_config = {
	.ap = {	
	    .ssid = ESP_AP_SSID,
	    .channel = 0,
	    .authmode = WIFI_AUTH_WPA2_PSK,
	    .password = ESP_AP_PASS,
	    .ssid_hidden = 1,
	    .max_connection = 8,
	    .beacon_interval = 100
	}
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config) );

    // Enable DNS (offer) for dhcp server
    dhcps_offer_t dhcps_dns_value = OFFER_DNS;
    dhcps_set_option_info(6, &dhcps_dns_value, sizeof(dhcps_dns_value));

    // Set custom dns server address for dhcp server
    dnsserver.u_addr.ip4.addr = htonl(MY_DNS_IP_ADDR);
    dnsserver.type = IPADDR_TYPE_V4;
    dhcps_dns_setserver(&dnsserver);

    //tcpip_adapter_get_dns_info(TCPIP_ADAPTER_IF_AP, TCPIP_ADAPTER_DNS_MAIN, &dnsinfo);
    //ESP_LOGI(TAG, "DNS IP:" IPSTR, IP2STR(&dnsinfo.ip.u_addr.ip4));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_apsta finished.");
    ESP_LOGI(TAG, "connect to ap SSID: %s ",
	     EXAMPLE_ESP_WIFI_SSID);
}

void app_main()
{
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
	//ESP_ERROR_CHECK(esp_netif_init());
//tcpip_adapter_init();
  // Setup WIFI
  wifi_init_sta();

#if IP_NAPT
  u32_t napt_netif_ip = 0xC0A80401; // Set to ip address of softAP netif (Default is 192.168.4.1)
  ip_napt_enable(htonl(napt_netif_ip), 1);
  ESP_LOGI(TAG, "NAT is enabled");
#endif

xTaskCreate(mqtt_task, "mqtt_task", 4096, NULL, 5, NULL);

}
