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
#include "buzzer.h"
#include "mode.h"
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
// buzzer
using BUZZER = mcu::PA8;
// sensors
using TEMP  = mcu::PA0;
using LEVEL = mcu::PC13;
using COVER = mcu::PC14;
// seven segment indicator
using A  = mcu::PB10;
using B  = mcu::PB2;
using C  = mcu::PB0;
using D  = mcu::PA7;
using E  = mcu::PA6;
using F  = mcu::PB12;
using G  = mcu::PB11;
using H  = mcu::PB1;
using K1 = mcu::PB3;
using K2 = mcu::PA15;
using K3 = mcu::PB9;
using K4 = mcu::PB8;
using K5 = mcu::PB7;
using K6 = mcu::PB6;

int main()
{
   struct Flash_data {
      uint16_t min_temperature = 5;
      uint16_t max_temperature = 99;
      uint16_t max_time = 99;
      uint16_t time = 0;
      uint16_t temp = 20;
      uint16_t mode = 0;
   }flash;

   [[maybe_unused]] auto _ = Flash_updater<
        mcu::FLASH::Sector::_26
      , mcu::FLASH::Sector::_25
   >::make (&flash);

   auto[cover, level] = make_pins<mcu::PinMode::Input, COVER, LEVEL>();
   auto[uz_1, uz_2, uz_3, heater, led_fn_en] 
        = make_pins<mcu::PinMode::Output, UZ1, UZ2, UZ3, HEATER, LED_F_EN>();

   constexpr bool inverted{true};

   auto time_up      = Button<TIME_UP,   inverted>();
   auto time_down    = Button<TIME_DOWN, inverted>();
   auto temp_up      = Button<TEMP_UP,   inverted>();
   auto temp_down    = Button<TEMP_DOWN, inverted>();
   auto start        = Button<START,     inverted>();
   auto mode         = Button<MODE,      inverted>();
   auto f_en         = Button<F_EN,      inverted>();
   auto set_max_temp = Tied_buttons(temp_up, temp_down);
   auto set_max_time = Tied_buttons(time_up, time_down);

   ADC_ adc;

   decltype (auto) pwm = PWM::make<mcu::Periph::TIM1, BUZZER>();
   Buzzer buzzer{pwm};

   auto mode_switch = Mode_switch::make<LED_M1, LED_M2, LED_M3>();

   using Flash  = decltype(flash);

   UZV<Flash, SSI<A, B, C, D, E, F, G, H, K1, K2, K3, K4, K5, K6>> 
         uzv (adc, flash, mode_switch
            , time_up, time_down, temp_up, temp_down, start, f_en, mode, set_max_temp, set_max_time
            , cover, level
            , uz_1, uz_2, uz_3, heater, led_fn_en
            , buzzer
         );

   while(1){

      uzv();

      __WFI();
   }

}



