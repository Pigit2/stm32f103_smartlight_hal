///////////////////////////////////////////////////////////
/*ͷ�ļ�������*/
#include "huaweiIOT.h"
#include "string.h"
//#include "usart.h"
#include "oled.h"

///////////////////////////////////////////////////////////
/*�궨����*/

///////////////////////////////////////////////////////////
/*�ⲿ����������*/
uint8_t atok_rec_flag;
uint8_t sync_time_flag=1;//ʱ��ͬ����־��1δͬ����0�ɹ�ͬ��
///////////////////////////////////////////////////////////
/*����������*/

///////////////////////////////////////////////////////////
/*����ʵ����*/

/**********************************************************************************************************
* �� �� ��: HuaweiIot_init
* ����˵��: ��Ϊ��������ƽ̨��ʼ��
* �� �Σ���
* �� �� ֵ: ��
**********************************************************************************************************/
void HuaweiIot_init(void)
{
    uint8_t i=0;
	
		char str_temp[256];	//��Ȼ��ʵ��ʹ��ʱstr_temp��󲻳�100�����ǲ���ʱ����char str_temp[128]ʱ����Ῠ��
												//���������ǿ������������Ϊchar str_temp[256]ʱ�����������У����߽��䶨��Ϊȫ�ֱ���
												//��������Ϊ������ͬ���·���ջ�ռ䲻ͬ������ԭ����պ���������ֹ�����-2023.3.10.1
	
		HAL_Delay(1000);			//�ȴ�ģ���ϵ��ȶ�

    for(i=0;i<10;i++)
    {
       if(atok_rec_flag==1)
				{
          AT_write("AT");					//AT����
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"Power ON",16);//Power ON
					
          printf("AT+RST\r\n");		//�����豸
          HAL_Delay(1800);					//�ȴ��豸����
          AT_write("AT");					//AT����
          AT_write("AT+CWMODE=1");//����ģ��ΪSTAģʽ
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"AT+CWMODE",16);//AT+CWMODE
					
					
					sprintf(str_temp,"AT+CWJAP=\"%s\",\"%s\"",WIFI_SSID,WIFI_PWD);	//����WiFi
          AT_write(str_temp);
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"CONNECT SSID:",16);//CONNECT SSID:
					OLED_ShowString(0,2,(u8 *)WIFI_SSID,16);
					HAL_Delay(1000);
					
					AT_write("AT+CIPSNTPCFG=1,8");//����ʱ��GMT+8
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"SET GMT+8",16);//SET GMT+8
					
					sprintf(str_temp,"AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"",HUAWEI_MQTT_USERNAME,HUAWEI_MQTT_PASSWORD);//����MQTT�ĵ�½�û���������
          AT_write(str_temp);
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"AT+MQTTUSERCFG",16);//AT+MQTTUSERCFG
					
					sprintf(str_temp,"AT+MQTTCLIENTID=0,\"%s\"",HUAWEI_MQTT_ClientID);//����MQTT��ClientID
          AT_write(str_temp);
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"AT+MQTTCLIENTID",16);//AT+MQTTCLIENTID
					
					sprintf(str_temp,"AT+MQTTCONN=0,\"%s\",%s,1",HUAWEI_MQTT_ADDRESS,HUAWEI_MQTT_PORT);//����MQTT�����ַ��˿ں�
					AT_write(str_temp);
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"AT+MQTTCONN",16);//AT+MQTTCONN
					
					sprintf(str_temp,"AT+MQTTSUB=0,\"$oc/devices/%s/sys/properties/report\",1",HUAWEI_MQTT_DeviceID);	//�����豸�����ϱ�������
					AT_write(str_temp);
					OLED_Clear();
					OLED_ShowString(0,0,(u8 *)"AT+MQTTSUB",16);//AT+MQTTSUB
						
					sprintf(str_temp,"AT+MQTTSUB=0,\"$oc/devices/%s/sys/commands/#\",1",HUAWEI_MQTT_DeviceID);//�����豸������յ�����
          AT_write(str_temp);
						
					sprintf(str_temp,"AT+MQTTSUB=0,\"$oc/devices/%s/sys/commands/response/#\",1",HUAWEI_MQTT_DeviceID);//�����豸������Ӧ������
          AT_write(str_temp);
						
            break;
        }    
        else
        {
						if(i==9)
						{
							printf("Connected ESP01s Fail!\r\n");	//���ӳ�ʱ
							OLED_Clear();
							OLED_ShowString(0,0,(u8 *)"CONNECTED FAILD",16);//CONNECTED FAILD
							break;
						}
						else 
						{
							printf("AT\r\n");
							HAL_Delay(1000);
						}
						
        }            
    }
}
/**********************************************************************************************************
* �� �� ��: AT_write
* ����˵��: ATָ���
* �� �Σ���
* �� �� ֵ: ��
**********************************************************************************************************/
void AT_write(char atstring[512])//�����ȴ�OK
{
		atok_rec_flag=0;
    printf("%s\r\n",atstring);
		while(1)
		{
			if(atok_rec_flag==1)	//���յ�OK�󴮿��жϻὫatok_rec_flag��1
			{
				atok_rec_flag=0;
				break;
			}
			else 
				HAL_Delay(50);
		}	
}
/**********************************************************************************************************
* �� �� ��: HuaweiIot_publish
* ����˵��: ��Ϊ���豸�����ϱ�
* �� �Σ�char * att,��������uint16_t data������ֵ
* �� �� ֵ: ��
**********************************************************************************************************/
void HuaweiIot_DevDate_publish(char *att,float data)       //�ϱ�����
{
	printf("AT+MQTTPUB=0,\"$oc/devices/%s/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"%s\\\"\\,\\\"properties\\\":{\\\"%s\\\": %f}}]}\",0,0\r\n",HUAWEI_MQTT_DeviceID,HUAWEI_MQTT_ServiceID,att,data);
}

void HuaweiIot_DevDate_publish3(char *att1,float data1,char *att2,float data2,char *att3,float data3)      //�ϱ�����
{
	printf("AT+MQTTPUB=0,\"$oc/devices/%s/sys/properties/report\",\"{\\\"services\\\":[{\\\"service_id\\\":\\\"%s\\\"\\,\\\"properties\\\":{\\\"%s\\\":%f\\,\\\"%s\\\":%f\\,\\\"%s\\\":%f}}]}\",0,0\r\n",HUAWEI_MQTT_DeviceID,HUAWEI_MQTT_ServiceID,att1,data1,att2,data2,att3,data3);
}
///////////////////////////////////////////////////////////
