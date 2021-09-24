#pragma once

#include <tuple>
#include "pin.h"
#include "button.h"

static std::tuple<int, int, int> led [] = {
   std::make_tuple(1, 0, 0),
   std::make_tuple(0, 1, 0),
   std::make_tuple(0, 0, 1)
};

class Mode_switch
{
   uint8_t mode{1};
   
   std::tuple<Pin, Pin, Pin> mode_led;

   Mode_switch (std::tuple<Pin, Pin, Pin> mode_led)
      : mode_led    {mode_led}
   {}

public:

   template <class Mode_1, class Mode_2, class Mode_3>
   static auto &make()
   {
      static Mode_switch mode_switch {
         make_pins<mcu::PinMode::Output, Mode_1, Mode_2, Mode_3>()};

      return mode_switch;
   }

   void init(uint8_t set_mode) {
      mode = set_mode;
      mode_led = led[mode];
   }

   void change()
   { 
      mode++; if (mode > 2) mode = 0;
      mode_led = led[mode]; 
   }

   uint8_t operator() () { return mode; }

};