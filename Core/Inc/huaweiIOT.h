#ifndef __HUAWEIIOT_H
#define __HUAWEIIOT_H

///////////////////////////////////////////////////////////
/*ͷ�ļ�������*/
#include "main.h"
///////////////////////////////////////////////////////////
/*�궨����*/
//ʹ��ʱ�������ﲹȫ�����Ϣ
#define WIFI_SSID									"HUAWEI2"
#define WIFI_PWD									"13539928668"
//#define HUAWEI_MQTT_USERNAME			"645b7a79eb2ee73fc4d21149_20230510"
#define HUAWEI_MQTT_USERNAME			"64830c9901554a59339fdf41_20230609"
//#define HUAWEI_MQTT_PASSWORD			"f493d4ee82e57e3b915535b4ac6b37cd632c75a413090b16e3cd4a10342136de"
#define HUAWEI_MQTT_PASSWORD			"fd31f9dc508eec63a824cb50922d4667291d35ad853b7e44b5ec73509575fa8d"
//#define HUAWEI_MQTT_ClientID			"645b7a79eb2ee73fc4d21149_20230510_0_0_2023051102"
#define HUAWEI_MQTT_ClientID			"64830c9901554a59339fdf41_20230609_0_0_2023060912"
//#define HUAWEI_MQTT_ADDRESS				"746ad467ac.st1.iotda-device.cn-north-4.myhuaweicloud.com"
#define HUAWEI_MQTT_ADDRESS				"38702c0e79.iot-mqtts.cn-north-4.myhuaweicloud.com"
#define HUAWEI_MQTT_PORT					"1883"
//#define HUAWEI_MQTT_DeviceID			"645b7a79eb2ee73fc4d21149_20230510"
#define HUAWEI_MQTT_DeviceID			"64830c9901554a59339fdf41_20230609"
#define HUAWEI_MQTT_ServiceID			"BasicData"
///////////////////////////////////////////////////////////
/*�ⲿ����������*/
extern uint8_t atok_rec_flag;
extern uint8_t sync_time_flag;//ʱ��ͬ����־��1δͬ����0�ɹ�ͬ��
///////////////////////////////////////////////////////////
/*����������*/
void AT_write(char atstring[512]);//�����ȴ�OK
void HuaweiIot_init(void);
void HuaweiIot_DevDate_publish(char * att,float data);
void HuaweiIot_DevDate_publish3(char *att1,float data1,char *att2,float data2,char *att3,float data3);
///////////////////////////////////////////////////////////
#endif
