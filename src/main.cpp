#define STM32F030x6
#define F_OSC   8000000UL
#define F_CPU   48000000UL

#include "init_clock.h"
#include "periph_rcc.h"
#include "periph_flash.h"
#include "flash.h"
#include "pin.h"
#include "adc.h"
#include "button.h"
#include "hysteresis.h"
#include "NTC_table.h"
#include "uzv.h"

/// эта функция вызывается первой в startup файле
extern "C" void init_clock () { init_clock<F_CPU>(); }

// time setting buttons
using TIME_UP   = mcu::PB5;
using TIME_DOWN = mcu::PB4;
// temperature setting buttons
using TEMP_UP   = mcu::PA11;
using TEMP_DOWN = mcu::PA10;
// control buttons
using START = mcu::PF7;
using MODE  = mcu::PB13;
using F_EN  = mcu::PA12;
// indication of operating modes
using LED_M1   = mcu::PB14;
using LED_M2   = mcu::PB15;
using LED_M3   = mcu::PA9;
using LED_F_EN = mcu::PF6;
// generator control
using UZ1 = mcu::PA2;
using UZ2 = mcu::PA3;
using UZ3 = mcu::PA4;
// heater control
using HEATER = mcu::PA5;
// sensors
using TEMP  = mcu::PA0;
using LEVEL = mcu::PC13;
using COVER = mcu::PC14;
// seven segment indicator
using A_  = mcu::PB10;
using B_  = mcu::PB2;
using C_  = mcu::PB0;
using D_  = mcu::PA7;
using E_  = mcu::PA6;
using F_  = mcu::PB12;
using G_  = mcu::PB11;
using H_  = mcu::PB1;
using K1_ = mcu::PB3;
using K2_ = mcu::PA15;
using K3_ = mcu::PB9;
using K4_ = mcu::PB8;
using K5_ = mcu::PB7;
using K6_ = mcu::PB6;

constexpr uint8_t min_temperature = 5;
constexpr uint8_t max_temperature = 99;

int main()
{
   struct Flash_data {
      uint8_t time = 2;
      uint8_t temp = min_temperature;
   }flash;

   [[maybe_unused]] auto _ = Flash_updater<
        mcu::FLASH::Sector::_26
      , mcu::FLASH::Sector::_25
   >::make (&flash);

   auto[cover, level] = make_pins<mcu::PinMode::Input, COVER, LEVEL>();
   auto[uz_1, uz_2, uz_3, heater, led_m_1, led_m_2, led_m_3, led_fn_en] 
        = make_pins<mcu::PinMode::Output, UZ1, UZ2, UZ3, HEATER, LED_M1, LED_M2, LED_M3, LED_F_EN>();

   auto time_up   = Button<TIME_UP>();
   auto time_down = Button<TIME_DOWN>();
   auto temp_up   = Button<TEMP_UP>();
   auto temp_down = Button<TEMP_DOWN>();
   auto start     = Button<START>();
   auto mode      = Button<MODE>();
   auto f_en      = Button<F_EN>();

   ADC_ adc;

   using Flash  = decltype(flash);

   UZV<Flash> uzv (adc, flash
                 , time_up, time_down, temp_up, temp_down, start, mode, f_en
                 , cover, level
                 , uz_1, uz_2, uz_3, heater, led_m_1, led_m_2, led_m_3, led_fn_en
                 );

   while(1){

      uzv();

      __WFI();
   }

}



