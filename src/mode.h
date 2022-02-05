#pragma once

// #include <tuple>
#include "pin.h"
#include "button.h"

// static std::tuple<int, int, int> led [] = {
//    std::make_tuple(1, 0, 0),
//    std::make_tuple(0, 1, 0),
//    std::make_tuple(0, 0, 1)
// };

class Mode_switch
{
   uint8_t mode{1};
   
   // std::tuple<Pin, Pin, Pin> mode_led;

   Pin& led_1;
   Pin& led_2;
   Pin& led_3;

   Mode_switch (Pin& led_1, Pin& led_2, Pin& led_3)
      : led_1 {led_1}
      , led_2 {led_2}
      , led_3 {led_3}
   {}

public:

   template <class Mode_1, class Mode_2, class Mode_3>
   static auto &make()
   {
      static Mode_switch mode_switch {
         Pin::make<Mode_1, mcu::PinMode::Output>(),
         Pin::make<Mode_2, mcu::PinMode::Output>(),
         Pin::make<Mode_3, mcu::PinMode::Output>()
         };

      return mode_switch;
   }

   void init(uint8_t set_mode) {
      mode = set_mode;
      if (mode == 0) {
         led_1 = true;
         led_2 = false;
         led_3 = false;
      } else if (mode == 1) {
         led_1 = false;
         led_2 = true;
         led_3 = false;
      } else if (mode == 2) {
         led_1 = false;
         led_2 = false;
         led_3 = true;
      }
      // mode_led = led[mode];
   }

   void change()
   { 
      mode++; if (mode > 2) mode = 0;
      if (mode == 0) {
         led_1 = true;
         led_2 = false;
         led_3 = false;
      } else if (mode == 1) {
         led_1 = false;
         led_2 = true;
         led_3 = false;
      } else if (mode == 2) {
         led_1 = false;
         led_2 = false;
         led_3 = true;
      }
      // mode_led = led[mode]; 
   }

   uint8_t operator() () { return mode; }

};