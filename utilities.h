/**
 * @file      utilities.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-11-15
 *
 */

#pragma once

#define MODEM_BAUDRATE                      (115200)
#define MODEM_DTR_PIN                       (25)
#define MODEM_TX_PIN                        (26)
#define MODEM_RX_PIN                        (27)
// The modem boot pin needs to follow the startup sequence.
#define BOARD_PWRKEY_PIN                    (4)
#define BOARD_ADC_PIN                       (35)
// The modem power switch must be set to HIGH for the modem to supply power.
#define BOARD_POWERON_PIN                   (12)
#define MODEM_RING_PIN                      (33)
#define MODEM_RESET_PIN                     (5)
#define BOARD_MISO_PIN                      (2)
#define BOARD_MOSI_PIN                      (15)
#define BOARD_SCK_PIN                       (14)
#define BOARD_SD_CS_PIN                     (13)
#define BOARD_BAT_ADC_PIN                   (35)
#define MODEM_RESET_LEVEL                   HIGH
#define SerialAT                            Serial1

#define MODEM_GPS_ENABLE_GPIO               (-1)
#define MODEM_GPS_ENABLE_LEVEL              (-1)

#ifndef TINY_GSM_MODEM_A7670
#define TINY_GSM_MODEM_A7670
#endif

// It is only available in V1.4 version. In other versions, IO36 is not connected.
#define BOARD_SOLAR_ADC_PIN                 (36)

#define CAN_STBY                            (13)
#define CAN_RX                              (14)
#define CAN_TX                              (15)

#define LED_ERR                             (0)
#define LED_ACT                             (2)

#define CNT_HALL                            (23)

#define ACCEL_ADDR                          (0x1C)

// ERROR CODES
#define OK_INFO       0
#define CRC_ERROR     1
#define OL_ERROR      2
#define TIME_ERROR    3
#define PSU_IN_ERROR  4
#define CAN_ERROR     5
#define GSM_RECONN    6
#define GPS_NODATA    7

// // 127 is defined in GSM as the AUXVDD index
// #define MODEM_GPS_ENABLE_GPIO               (4)
// #define MODEM_GPS_ENABLE_LEVEL              (0)



