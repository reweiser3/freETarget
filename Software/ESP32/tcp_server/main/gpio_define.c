/*************************************************************************
 * 
 * file: gpio_init.c
 * 
 * description:  Initialize all of the GPIO pins
 * 
 **************************************************************************
 *
 * This file sets up the configuration of ALL of the GPIOs in one place
 * 
 * It contains definitions for all gpios even if they are not used in thie
 * particular implemenation.  The final table gpio_table contains a list of
 * the GPIOs and a link to the initialization table used for that particular
 * pin
 * 
 ***************************************************************************/
#include "string.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"

#include "helpers.h"
#include "gpio_define.h"

/*
 *  Generic Definitions
 */

/*
 *  Digital IO definitions
 */

DIO_struct_t dio00 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio01 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio02 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio03 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio04 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio05 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio06 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio07 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio08 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio09 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value

DIO_struct_t dio10 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio11 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio12 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio13 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio14 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio15 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio16 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio17 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio18 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio19 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value

DIO_struct_t dio20 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio21 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio22 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio23 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio24 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio25 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio26 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio27 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio28 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio29 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value

DIO_struct_t dio30 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio31 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio32 = { DIGITAL_IO, GPIO_MODE_OUTPUT, 0};                 // Mode and Initial Value
DIO_struct_t dio33 = { DIGITAL_IO, GPIO_MODE_OUTPUT, 0};                 // Mode and Initial Value
DIO_struct_t dio34 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Can only be input
DIO_struct_t dio35 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Can only be input
DIO_struct_t dio36 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Can only be input
DIO_struct_t dio37 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio38 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio39 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Can only be input

#if ( CONFIG_IDF_TARGET_ESP32S2 )
DIO_struct_t dio40 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio41 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio42 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio43 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio44 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio45 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio46 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio47 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio48 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio49 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
#endif

/*
 *  Analog IO usage
 */


analogIO_struct_t adc1_ch0 = { ANALOG_IO, 0, {0, 0} };          // CHANNEL 1, ADC 0
analogIO_struct_t adc1_ch1 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 1
analogIO_struct_t adc1_ch2 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 2
analogIO_struct_t adc1_ch3 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 3
analogIO_struct_t adc1_ch4 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 4
analogIO_struct_t adc1_ch5 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 5
analogIO_struct_t adc1_ch6 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 6
analogIO_struct_t adc1_ch7 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 7
analogIO_struct_t adc1_ch8 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 8
analogIO_struct_t adc1_ch9 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 9

analogIO_struct_t adc2_ch0 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 0
analogIO_struct_t adc2_ch1 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 1
analogIO_struct_t adc2_ch2 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 2
analogIO_struct_t adc2_ch3 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 3
analogIO_struct_t adc2_ch4 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 4
analogIO_struct_t adc2_ch5 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 5
analogIO_struct_t adc2_ch6 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 6
analogIO_struct_t adc2_ch7 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 7
analogIO_struct_t adc2_ch8 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 8
analogIO_struct_t adc2_ch9 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 9


/*
 *  GOIO Usage
 *
 *  This table contains the use of each of the individual pins
 * 
 */

gpio_struct_t gpio_table[] = {
//   Name      Number       Assigned
    {"GPIO0", GPIO_NUM_0,  NULL},   // RTC_GPIO0, GPIO0                       (Strapping EN)
    {"GPIO1", GPIO_NUM_1,  NULL},   // RTC_GPIO1, GPIO1, TOUCH1, ADC1_CH0
    {"GPIO2", GPIO_NUM_2,  NULL},   // RTC_GPIO2, GPIO2, TOUCH2, ADC1_CH1
    {"GPIO3", GPIO_NUM_3,  NULL},   // RTC_GPIO3, GPIO3, TOUCH3, ADC1_CH2     (Strapping JTAG source)
    {"GPIO4", GPIO_NUM_4,  NULL},   // RTC_GPIO4, GPIO4, TOUCH4, ADC1_CH3
    {"GPIO5", GPIO_NUM_5,  NULL},   // RTC_GPIO5, GPIO5, TOUCH5, ADC1_CH4
    {"GPIO6", GPIO_NUM_6,  NULL},   // RTC_GPIO6, GPIO6, TOUCH6, ADC1_CH5
    {"GPIO7", GPIO_NUM_7,  NULL},   // RTC_GPIO7, GPIO7, TOUCH7, ADC1_CH6
    {"GPIO8", GPIO_NUM_8,  NULL},   // RTC_GPIO8, GPIO8, TOUCH8, ADC1_CH7, SUBSPICS1
    {"GPIO9", GPIO_NUM_9,  NULL},   // RTC_GPIO9, GPIO9, TOUCH9, ADC1_CH8, FSPIHD, SUBSPIHD

    {"GPIO10", GPIO_NUM_10, NULL},   // RTC_GPIO10, GPIO10, TOUCH10, ADC1_CH9, FSPICS0, FSPIIO4, SUBSPICS0
    {"GPIO11", GPIO_NUM_11, NULL},   // RTC_GPIO11, GPIO11, TOUCH11, ADC2_CH0, FSPID, FSPIIO5, SUBSPID
    {"GPIO12", GPIO_NUM_12, NULL},   // RTC_GPIO12, GPIO12, TOUCH12, ADC2_CH1, FSPICLK, FSPIIO6, SUBSPICLK
    {"GPIO13", GPIO_NUM_13, NULL},   // RTC_GPIO13, GPIO13, TOUCH13, ADC2_CH2, FSPIQ, FSPIIO7, SUBSPIQ
    {"GPIO14", GPIO_NUM_14, NULL},   // RTC_GPIO14, GPIO14, TOUCH14, ADC2_CH3, FSPIWP, FSPIDQS, SUBSPIWP
    {"GPIO15", GPIO_NUM_15, NULL},   // RTC_GPIO15, GPIO15, U0RTS, ADC2_CH4, XTAL_32K_P
    {"GPIO16", GPIO_NUM_16, NULL},   // RTC_GPIO16, GPIO16, U0CTS, ADC2_CH5, XTAL_32K_N
    {"GPIO17", GPIO_NUM_17, NULL},   // RTC_GPIO17, GPIO17, U1TXD, ADC2_CH6
    {"GPIO18", GPIO_NUM_18, NULL},   // RTC_GPIO18, GPIO18, U1RXD, ADC2_CH7, CLK_OUT3
    {"GPIO19", GPIO_NUM_19, NULL},   // RTC_GPIO19, GPIO19, U1RTS, ADC2_CH8, CLK_OUT2, USB_D-
    {"GPIO20", GPIO_NUM_20, NULL},   // RTC_GPIO20, GPIO20, U1CTS, ADC2_CH9, CLK_OUT1, USB_D+

    {"GPIO21", GPIO_NUM_21, NULL},   // RTC_GPIO21, GPIO21
    {"GPIO22", GPIO_NUM_22, NULL},
    {"GPIO23", GPIO_NUM_23, NULL},

    {"GPIO25", GPIO_NUM_25, NULL},
    {"GPIO26", GPIO_NUM_26, NULL},
    {"GPIO27", GPIO_NUM_27, NULL},
    {"GPIO28", GPIO_NUM_28, NULL},
    {"GPIO29", GPIO_NUM_29, NULL},

    {"GPIO30",    GPIO_NUM_30, NULL},
    {"GPIO31",    GPIO_NUM_31, NULL},
    {"GREEN_LED", GPIO_NUM_32, (void*)&dio32},
    {"RED_LED",   GPIO_NUM_33, (void*)&dio33},
    {"SW0",       GPIO_NUM_34, (void*)&dio34},
    {"SW1",       GPIO_NUM_35, (void*)&dio35},
    {"SW2",       GPIO_NUM_36, (void*)&dio36},     // GPIO38, FSPIWP, SUBSPIWP
    {"GPIO37",    GPIO_NUM_37, NULL },             // GPIO38, FSPIWP, SUBSPIWP
    {"GPIO38",    GPIO_NUM_38, NULL },             // GPIO38, FSPIWP, SUBSPIWP
    {"SW3",       GPIO_NUM_39, (void*)&dio39},     // MTCK, GPIO39, CLK_OUT3, SUBSPICS1

#if ( CONFIG_IDF_TARGET_ESP32S2 )
    {"GPIO40", GPIO_NUM_40, NULL },   // MTDO, GPIO40, CLK_OUT1
    {"GPIO41", GPIO_NUM_41, NULL },   // MTDI, GPIO41, CLK_OUT1
    {"GPIO42", GPIO_NUM_42, NULL },   // MTMS, GPIO42
    {"GPIO43", GPIO_NUM_43, NULL },
    {"GPIO44", GPIO_NUM_44, NULL },   // U0RXD, GPIO44, CLK_OUT2
    {"GPIO45", GPIO_NUM_45, NULL },   // GPIO45                                     (Strapping VDD_SPI)
    {"GPIO46", GPIO_NUM_46, NULL },   //                                            (Strapping BOOT, ROM Message)
    {"GPIO47", GPIO_NUM_47, NULL },   // SPICLK_P_DIFF, GPIO47, SUBSPICLK_P_DIFF
    {"GPIO48", GPIO_NUM_48, NULL },   // SPICLK_N_DIFF, GPIO48, SUBSPICLK_N_DIFF
    {"GPIO49", GPIO_NUM_49, NULL },
#endif
    {0, 0, 0 } 
};


/*************************************************************************
 * 
 * file: gpio_init()
 * 
 * description:  Initialize the GPIO states
 * 
 * return:  Nope
 * 
 **************************************************************************
 *
 * The gpio_table is used to program the gpio hardware
 * 
 ***************************************************************************/

void gpio_init(void)
{
    int i;

/*
 *  Loop and setup the GPIO
 */
    i=0;
    while (gpio_table[i].gpio_name != 0 )
    {
        if (gpio_table[i].gpio_uses != NULL )
        {
            switch (((DIO_struct_t*)(gpio_table[i].gpio_uses))->type)
            {      
                default:
                    break;

                case DIGITAL_IO:
                    switch (((DIO_struct_t*)(gpio_table[i].gpio_uses))->mode)
                    {
                        case GPIO_MODE_INPUT:
                            gpio_set_direction(gpio_table[i].gpio_number, ((DIO_struct_t*)(gpio_table[i].gpio_uses))->mode);
                            break;

                        case GPIO_MODE_OUTPUT:
                        case GPIO_MODE_OUTPUT_OD:
                        case GPIO_MODE_INPUT_OUTPUT_OD:
                        case GPIO_MODE_INPUT_OUTPUT:
                            gpio_set_direction(gpio_table[i].gpio_number, ((DIO_struct_t*)(gpio_table[i].gpio_uses))->mode);
                            gpio_set_level(gpio_table[i].gpio_number, ((DIO_struct_t*)(gpio_table[i].gpio_uses))->initial_value);
                            break;

                        default:
                            break;
                    }
                    break;

                case ANALOG_IO:
#if (0)
                adc_oneshot_new_unit(gpio_table[i].config[CONFIG], &analogIO_table[i].config[HANDLE]);
                adc_oneshot_config_channel(gpio_table[i].config[CONFIG], &analogIO_table[i].config[HANDLE]));
#endif
                    break;
            }
        }
        i++;
    }

/*
 *  all done, return
 */
    return;
}
