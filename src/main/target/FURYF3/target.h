/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef FURYF3OSD
    #define TARGET_BOARD_IDENTIFIER "FY3O"
//    #define USBD_PRODUCT_STRING     "FuryF3OSD"
#else
    #define TARGET_BOARD_IDENTIFIER "FYF3"
//    #define USBD_PRODUCT_STRING     "FuryF3"
#endif

#define LED0_PIN                PC14

#define USE_BEEPER
#define BEEPER_PIN              PC15
#define BEEPER_INVERTED

#define USE_EXTI
#define USE_GYRO_EXTI
#define GYRO_1_EXTI_PIN         PA3
#define USE_MPU_DATA_READY_SIGNAL
#define ENSURE_MPU_DATA_READY_IS_LOW

#define GYRO_1_CS_PIN           PA4
#define GYRO_1_SPI_INSTANCE     SPI1

#define USE_GYRO
#define USE_GYRO_SPI_MPU6000
#define USE_GYRO_SPI_MPU6500
#define USE_GYRO_SPI_ICM20689
// MPU6000 and ICM20689 are CW180 aligned.
// MPU6500 is CW90 aligned, but can't handle this.
// Must be configured after flashing.
#define GYRO_1_ALIGN            CW180_DEG

#define USE_ACC
#define USE_ACC_SPI_MPU6000
#define USE_ACC_SPI_MPU6500
#define USE_ACC_SPI_ICM20689

#define USE_SPI
#define USE_SPI_DEVICE_1
#define USE_SPI_DEVICE_2 // PB12,13,14,15 on AF5

#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#ifdef FURYF3OSD
    // include the max7456 driver
    #define USE_MAX7456
    #define MAX7456_SPI_INSTANCE    SPI1
    #define MAX7456_SPI_CS_PIN      PC13

    #define USE_FLASHFS
    #define USE_FLASH_M25P16
    #define FLASH_CS_PIN            PB12
    #define FLASH_SPI_INSTANCE      SPI2

    #define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

    #define DEFAULT_FEATURES        (0)
    #define DEFAULT_CURRENT_METER_SOURCE CURRENT_METER_ADC
#else

    #define USE_SDCARD
    #define USE_SDCARD_SPI
    #define SDCARD_DETECT_INVERTED
    #define SDCARD_DETECT_PIN                   PB2
    #define SDCARD_SPI_INSTANCE                 SPI2
    #define SDCARD_SPI_CS_PIN                   SPI2_NSS_PIN

    // Note, this is the same DMA channel as UART1_RX. Luckily we don't use DMA for USART Rx.
    #define SDCARD_DMA_CHANNEL_TX               DMA1_Channel5

    // Performance logging for SD card operations:
    // #define AFATFS_USE_INTROSPECTIVE_LOGGING

    #define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

    #define USE_BARO
    #define USE_BARO_MS5611

#endif

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C_DEVICE              (I2CDEV_1) // SDA (PB9/AF4), SCL (PB8/AF4)
#define I2C1_SCL            PB8
#define I2C1_SDA            PB9

#define USE_VCP
#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_SOFTSERIAL1
#define USE_SOFTSERIAL2

#define SERIAL_PORT_COUNT 6

#define SOFTSERIAL1_RX_PIN      PB0
#define SOFTSERIAL1_TX_PIN      PB1

#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_PIN  PB3  // (HARDARE=0,PPM)

#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define UART2_TX_PIN            PA14
#define UART2_RX_PIN            PA15

#define UART3_TX_PIN            PB10 // PB10 (AF7)
#define UART3_RX_PIN            PB11 // PB11 (AF7)

#define DEFAULT_VOLTAGE_METER_SOURCE VOLTAGE_METER_ADC
#define USE_ADC
#define ADC_INSTANCE            ADC1
#define VBAT_ADC_PIN            PA0
#define RSSI_ADC_PIN            PA1
#define CURRENT_METER_ADC_PIN   PA2

#define DEFAULT_RX_FEATURE      FEATURE_RX_PPM

#define REMAP_TIM17_DMA

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         (BIT(13)|BIT(14)|BIT(15))
#define TARGET_IO_PORTF         (BIT(0)|BIT(1)|BIT(3)|BIT(4))

#define USABLE_TIMER_CHANNEL_COUNT 8
#define USED_TIMERS             (TIM_N(1) | TIM_N(2) | TIM_N(3) | TIM_N(4) | TIM_N(8) | TIM_N(16) | TIM_N(17))
