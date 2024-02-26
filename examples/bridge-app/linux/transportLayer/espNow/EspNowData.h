#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef enum __attribute__((packed))
{
    ESP_NOW_DEVICE_TYPE_DHT,
    ESP_NOW_DEVICE_TYPE_MOTION,
    ESP_NOW_DEVICE_TYPE_ON_OFF,
    ESP_NOW_DEVICE_TYPE_TOGGLE,
    ESP_NOW_DEVICE_TYPE_LIGHT_ON_OFF,
    ESP_NOW_DEVICE_TYPE_LIGHT_DIMMER,
    ESP_NOW_DEVICE_TYPE_LIGHT_RGB,
    ESP_NOW_DEVICE_TYPE_LIGHT_TEMP,
    ESP_NOW_DEVICE_TYPE_LIGHT_TEMP_RGB,
    ESP_NOW_DEVICE_TYPE_COUNT
}ESP_NOW_DEVICE_TYPE;

typedef struct __attribute__((packed))
{
    float temperature;
    float humidity;
}ESP_NOW_DATA_DHT;

typedef struct __attribute__((packed))
{
    bool dummy; //always true
}ESP_NOW_DATA_MOTION;

typedef struct __attribute__((packed))
{
    bool dummy; //always true
}ESP_NOW_DATA_TOGGLE;

typedef struct __attribute__((packed))
{
    bool onOff;
}ESP_NOW_DATA_ON_OFF;

typedef struct __attribute__((packed))
{
    bool onOff;
}ESP_NOW_DATA_LIGHT_ON_OFF;

typedef struct __attribute__((packed))
{
    bool onOff;
    uint8_t brightness;
}ESP_NOW_DATA_LIGHT_DIMMER;

typedef enum __attribute__((packed))
{
    ESP_NOW_DATA_LIGHT_RGB_MODE_STATIC,
    ESP_NOW_DATA_LIGHT_RGB_MODE_RAINBOW,
    ESP_NOW_DATA_LIGHT_RGB_MODE_COUNT,
}ESP_NOW_DATA_LIGHT_RGB_MODE;
typedef struct __attribute__((packed))
{
    bool onOff;
    ESP_NOW_DATA_LIGHT_RGB_MODE mode;
    uint8_t hue;
    uint8_t saturation;
    uint8_t brightness;
}ESP_NOW_DATA_LIGHT_RGB;

typedef struct __attribute__((packed))
{
    bool onOff;
    uint16_t tempKelvin;
}ESP_NOW_DATA_LIGHT_TEMP;

typedef struct __attribute__((packed))
{
    bool onOff;
    uint8_t hue;
    uint8_t saturation;
    uint8_t brightness;
    uint16_t tempKelvin;
}ESP_NOW_DATA_LIGHT_TEMP_RGB;

typedef union __attribute__((packed))
{
    ESP_NOW_DATA_DHT dht;
    ESP_NOW_DATA_MOTION motion;
    ESP_NOW_DATA_ON_OFF onOff;
    ESP_NOW_DATA_TOGGLE toggle;
    ESP_NOW_DATA_LIGHT_ON_OFF lightOnOff;
    ESP_NOW_DATA_LIGHT_DIMMER lightDimmer;
    ESP_NOW_DATA_LIGHT_RGB lightRgb;
    ESP_NOW_DATA_LIGHT_TEMP lightTemp;
    ESP_NOW_DATA_LIGHT_TEMP_RGB lightTempRgb;
}ESP_NOW_DATA_UNION;

typedef struct __attribute__((packed))
{
    uint8_t macAddr[6];
    ESP_NOW_DEVICE_TYPE type;
    ESP_NOW_DATA_UNION data;
}ESP_NOW_DATA;
/**************************************************************************
 *                                  Variables
 **************************************************************************/
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
const char* EspNowGetName(const ESP_NOW_DATA* pDev);