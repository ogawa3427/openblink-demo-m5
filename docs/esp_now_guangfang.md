ESP-NOW
[中文]

Overview
ESP-NOW is a kind of connectionless Wi-Fi communication protocol that is defined by Espressif. In ESP-NOW, application data is encapsulated in a vendor-specific action frame and then transmitted from one Wi-Fi device to another without connection.

CTR with CBC-MAC Protocol (CCMP) is used to protect the action frame for security. ESP-NOW is widely used in smart light, remote controlling, sensor, etc.

Frame Format
ESP-NOW uses a vendor-specific action frame to transmit ESP-NOW data. The default ESP-NOW bit rate is 1 Mbps.

Currently, ESP-NOW supports two versions: v1.0 and v2.0. The maximum packet length supported by v2.0 devices is 1470 (ESP_NOW_MAX_DATA_LEN_V2) bytes, while the maximum packet length supported by v1.0 devices is 250 (ESP_NOW_MAX_DATA_LEN) bytes.

The v2.0 devices are capable of receiving packets from both v2.0 and v1.0 devices. In contrast, v1.0 devices can only receive packets from other v1.0 devices.

However, v1.0 devices can receive v2.0 packets if the packet length is less than or equal to 250 (ESP_NOW_MAX_IE_DATA_LEN).For packets exceeding this length, the v1.0 devices will either truncate the data to the first 250 (ESP_NOW_MAX_IE_DATA_LEN) bytes or discard the packet entirely.

For detailed behavior, please refer to the documentation corresponding to the specific IDF version.

The format of the vendor-specific action frame is as follows:

------------------------------------------------------------------------------------------------------------
| MAC Header | Category Code | Organization Identifier | Random Values | Vendor Specific Content |   FCS   |
------------------------------------------------------------------------------------------------------------
  24 bytes         1 byte              3 bytes               4 bytes             7-x bytes        4 bytes
Category Code: The Category Code field is set to the value (127) indicating the vendor-specific category.

Organization Identifier: The Organization Identifier contains a unique identifier (0x18fe34), which is the first three bytes of MAC address applied by Espressif.

Random Value: The Random Value filed is used to prevents relay attacks.

Vendor Specific Content: The Vendor Specific Content contains several (at least one) vendor-specific element fields. For version v2.0, x = 1512(1470 + 6*7), for version v1.0, x = 257(250 + 7).

The format of the vendor-specific element frame is as follows:

ESP-NOW v1.0:
------------------------------------------------------------------------------------------
| Element ID | Length | Organization Identifier | Type | Reserved | Version |    Body    |
------------------------------------------------------------------------------------------
                                                         7~4 bits | 3~0 bits
    1 byte     1 byte            3 bytes         1 byte        1 byte         0-250 bytes

ESP-NOW v2.0:
-----------------------------------------------------------------------------------------------------
| Element ID | Length | Organization Identifier | Type | Reserved | More data |Version |    Body    |
-----------------------------------------------------------------------------------------------------
                                                         7~5 bits |   1 bit   |3~0 bits
    1 byte     1 byte            3 bytes         1 byte             1 byte                0-250 bytes
Element ID: The Element ID field is set to the value (221), indicating the vendor-specific element.

Length: The length is the total length of Organization Identifier, Type, Version and Body, the maximum value is 255.

Organization Identifier: The Organization Identifier contains a unique identifier (0x18fe34), which is the first three bytes of MAC address applied by Espressif.

Type: The Type field is set to the value (4) indicating ESP-NOW.

Version: The Version field is set to the version of ESP-NOW.

Body: The Body contains the actual ESP-NOW data to be transmitted.

As ESP-NOW is connectionless, the MAC header is a little different from that of standard frames. The FromDS and ToDS bits of FrameControl field are both 0. The first address field is set to the destination address. The second address field is set to the source address. The third address field is set to broadcast address (0xff:0xff:0xff:0xff:0xff:0xff).

Security
ESP-NOW uses the CCMP method, which is described in IEEE Std. 802.11-2012, to protect the vendor-specific action frame. The Wi-Fi device maintains a Primary Master Key (PMK) and several Local Master Keys (LMKs, each paired device has one LMK). The lengths of both PMK and LMK are 16 bytes.

PMK is used to encrypt LMK with the AES-128 algorithm. Call esp_now_set_pmk() to set PMK. If PMK is not set, a default PMK will be used.

LMK of the paired device is used to encrypt the vendor-specific action frame with the CCMP method. If the LMK of the paired device is not set, the vendor-specific action frame will not be encrypted.

Encrypting multicast vendor-specific action frame is not supported.

Initialization and Deinitialization
Call esp_now_init() to initialize ESP-NOW and esp_now_deinit() to de-initialize ESP-NOW. ESP-NOW data must be transmitted after Wi-Fi is started, so it is recommended to start Wi-Fi before initializing ESP-NOW and stop Wi-Fi after de-initializing ESP-NOW.

When esp_now_deinit() is called, all of the information of paired devices are deleted.

Add Paired Device
Call esp_now_add_peer() to add the device to the paired device list before you send data to this device. If security is enabled, the LMK must be set. A device with a broadcast MAC address must be added before sending broadcast data.

You can send ESP-NOW data via both the Station and the SoftAP interface. Make sure that the interface is enabled before sending ESP-NOW data.

The range of the channel of paired devices is from 0 to 14. If the channel is set to 0, data will be sent on the current channel. Otherwise, the channel must be set as the channel that the local device is on.

For the receiving device, calling esp_now_add_peer() is not required. If no paired device is added, it can only receive broadcast packets and unencrypted unicast packets. To receive encrypted unicast packets, a paired device must be added, and the same LMK must be set.

The maximum number of paired devices is 20, and the paired encryption devices are no more than 17, the default is 7. If you want to change the number of paired encryption devices, set CONFIG_ESP_WIFI_ESPNOW_MAX_ENCRYPT_NUM in the Wi-Fi component configuration menu.

Send ESP-NOW Data
Call esp_now_send() to send ESP-NOW data and esp_now_register_send_cb() to register sending callback function. It will return ESP_NOW_SEND_SUCCESS in sending callback function if the data is received successfully on the MAC layer. Otherwise, it will return ESP_NOW_SEND_FAIL. Several reasons can lead to ESP-NOW fails to send data. For example, the destination device does not exist; the channels of the devices are not the same; the action frame is lost when transmitting on the air, etc. It is not guaranteed that application layer can receive the data. If necessary, send back ack data when receiving ESP-NOW data. If receiving ack data timeouts, retransmit the ESP-NOW data. A sequence number can also be assigned to ESP-NOW data to drop the duplicate data.

If there is a lot of ESP-NOW data to send, call esp_now_send() to send less than or equal to the maximum packet length (v1.0 is 250 bytes, v2.0 is 1470 bytes) of data once a time. Note that too short interval between sending two ESP-NOW data may lead to disorder of sending callback function. So, it is recommended that sending the next ESP-NOW data after the sending callback function of the previous sending has returned. The sending callback function runs from a high-priority Wi-Fi task. So, do not do lengthy operations in the callback function. Instead, post the necessary data to a queue and handle it from a lower priority task.

Receiving ESP-NOW Data
Call esp_now_register_recv_cb() to register receiving callback function. Call the receiving callback function when receiving ESP-NOW. The receiving callback function also runs from the Wi-Fi task. So, do not do lengthy operations in the callback function. Instead, post the necessary data to a queue and handle it from a lower priority task.

Config ESP-NOW Rate
Call esp_wifi_config_espnow_rate() to config ESP-NOW rate of specified interface. Make sure that the interface is enabled before config rate. This API should be called after esp_wifi_start().

Config ESP-NOW Power-saving Parameter
Sleep is supported only when ESP32-C3 is configured as station.

Call esp_now_set_wake_window() to configure Window for ESP-NOW RX at sleep. The default value is the maximum, which allowing RX all the time.

If Power-saving is needed for ESP-NOW, call esp_wifi_connectionless_module_set_wake_interval() to configure Interval as well.

Please refer to connectionless module power save to get more detail.

Application Examples
wifi/espnow demonstrates how to use the ESPNOW feature of ESP32-C3's Wi-Fi, including starting Wi-Fi, initializing ESP-NOW, registering ESP-NOW sending or receiving callback function, adding ESP-NOW peer information, and sending and receiving ESP-NOW data between two devices.

API Reference
Header File
components/esp_wifi/include/esp_now.h

This header file can be included with:

#include "esp_now.h"
This header file is a part of the API provided by the esp_wifi component. To declare that your component depends on esp_wifi, add the following to your CMakeLists.txt:

REQUIRES esp_wifi
or

PRIV_REQUIRES esp_wifi
Structures
struct esp_now_peer_info
ESPNOW peer information parameters.

Public Members

uint8_t peer_addr[ESP_NOW_ETH_ALEN]
ESPNOW peer MAC address that is also the MAC address of station or softap

uint8_t lmk[ESP_NOW_KEY_LEN]
ESPNOW peer local master key that is used to encrypt data

uint8_t channel
Wi-Fi channel that peer uses to send/receive ESPNOW data. If the value is 0, use the current channel which station or softap is on. Otherwise, it must be set as the channel that station or softap is on.

wifi_interface_t ifidx
Wi-Fi interface that peer uses to send/receive ESPNOW data

bool encrypt
ESPNOW data that this peer sends/receives is encrypted or not

void *priv
ESPNOW peer private data

struct esp_now_peer_num
Number of ESPNOW peers which exist currently.

Public Members

int total_num
Total number of ESPNOW peers, maximum value is ESP_NOW_MAX_TOTAL_PEER_NUM

int encrypt_num
Number of encrypted ESPNOW peers, maximum value is ESP_NOW_MAX_ENCRYPT_PEER_NUM

struct esp_now_recv_info
ESPNOW receive packet information.

Public Members

uint8_t *src_addr
Source address of ESPNOW packet

uint8_t *des_addr
Destination address of ESPNOW packet

wifi_pkt_rx_ctrl_t *rx_ctrl
Rx control info of ESPNOW packet