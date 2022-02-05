#pragma once

#include "pin.h"
#include "adc.h"
#include "flash.h"
#include "timers.h"
#include "NTC_table.h"
#include "literals.h"
#include "seven_segment_indicator_2.h"



constexpr auto conversion_on_channel {16};
struct ADC_{
   ADC_average& control     = ADC_average::make<mcu::Periph::ADC1>(conversion_on_channel);
   ADC_channel& temperature = control.add_channel<mcu::PA0>();
};

constexpr size_t U = 33;
constexpr size_t R = 5100;

template<class Flash_data, class SSI>
class UZV
{

public:

   UZV (
        ADC_& adc
      , Flash_data& flash
      , Mode_switch& mode
      , Button_event& time_up
      , Button_event& time_down
      , Button_event& temp_up
      , Button_event& temp_down
      , Button_event& start
      , Button_event& f_en
      , Button_event& button_mode
      , Button_event& set_max_temp
      , Button_event& set_max_time
      , Pin& cover
      , Pin& level
      , Pin& uz_1
      , Pin& uz_2
      , Pin& uz_3
      , Pin& heater
      , Pin& led_fn_en
      , Buzzer& buzzer
   
   )  : adc         {adc}
      , flash       {flash}
      , mode        {mode}
      , time_up     {time_up}
      , time_down   {time_down}
      , temp_up     {temp_up}
      , temp_down   {temp_down}
      , start       {start}
      , f_en        {f_en}
      , button_mode {button_mode}
      , set_max_temp{set_max_temp}
      , set_max_time{set_max_time}
      , cover       {cover}
      , level       {level}
      , uz_1        {uz_1}
      , uz_2        {uz_2}
      , uz_3        {uz_3}
      , heater      {heater}
      , led_fn_en   {led_fn_en}
      , buzzer      {buzzer}
      {
         adc.control.start();
         buzzer.longer();
         init_set_max_temp();
         init_set_max_time();
      }

   void operator() () {

      level_control();
      cover_control();

      show_temperature(temperature);

      if (setting_time) {
         show_set_time(flash.max_time);
      } else if (state == State::wait or state == State::preheating) {
         show_set_time(set_time);
      } else if (state == State::pause and blink) {
         show_sign(Sign::Space, Sign::Space, 0, 1);
      } else {
         show_set_time((timer.timeLeft() / 60000) + 1);
      }

      if (state == State::emergency or state == State::pause) {
           not level & not cover ? show_sign(Sign::Level, Sign::Cover)
         : not level             ? show_sign(Sign::Level, Sign::Space)
         : not cover             ? show_sign(Sign::Cover, Sign::Space)
         : mode() == 0           ? show_sign(Sign::Hyphen, Sign::Hyphen)
         : show_temperature(temperature);
      } else if (state != emergency) {
         mode() == 0 ? show_sign(Sign::Hyphen, Sign::Hyphen)
                     : setting_temp ? show_set_temperature(flash.max_temperature)
                     : show_set_temperature(set_temperature);
      }

      heating();

      if (refresh.event()) {

         state == State::emergency or state == State::pause ? blink ^= 1 : blink = false;
         
         if (heater) {
            ssi.point[2] ^= 1;
         } else { 
            ssi.point[2] = false;
         }

         if (state == State::pusk) {
            n++;
            ssi.point[0] ^= 1;
            if (n == 6)           uz_1 = true;
            if (uz_1 and n == 12) uz_2 = true;
            if (uz_2 and n == 18) uz_3 = true;
         }else {
            n = 0;
            ssi.point[0] = false;
            uz_1 = false;
            uz_2 = false;
            uz_3 = false;
         }

         temp(adc.temperature);
      }

      switch (state)
      {
         case init:
            mode.init(flash.mode);
            init_mode();
            init_time();
            init_start();
            state = State::wait;
         break;
         case wait:
            heat = Heat::_1;
            if (on) {
               if (mode() == 2) {
                  state = State::preheating;
               } else {
                  state = State::pusk;
                  timer.start(set_time * 60000);
               }
               flash.mode = mode();
               flash.temp = set_temperature;
               flash.time = set_time;
               left_time = set_time;
               deinit();
            }
         break;
         case preheating:
            if (heating()) {
               state = State::pusk;
               timer.start(set_time * 60000);
            }
            if (not hold){
               state = State::init;
            }
         break;
         case pusk:
            if (not on){
               timer.pause();
               state = State::pause;
            }
            if (not hold){
               timer.stop();
               state = State::init;
            }
            if (timer.done()) {
               on = false;
               hold = false;
               timer.stop();
               buzzer.longer();
               state = State::init;
            }

         break;
         case pause:
            if (not hold){
               state = State::init;
            }
            if (on) {
               timer.start();
               init_start();
               state = State::pusk;
            } else {
               init_start();
            }

         break;
         case emergency:
            if (cover & level) state = State::init;
         break;
      }

   }

private:

   enum State {init, wait, preheating, pusk, pause, emergency} state{State::init};
   enum Heat {_1, _2} heat{Heat::_1};
   
   ADC_& adc;
   Flash_data& flash;
   SSI ssi;
   Mode_switch& mode;
   
   Button_event& time_up;
   Button_event& time_down;
   Button_event& temp_up;
   Button_event& temp_down;
   Button_event& start;
   Button_event& f_en;
   Button_event& button_mode;
   Button_event& set_max_temp;
   Button_event& set_max_time;

   Pin& cover;
   Pin& level;

   Pin& uz_1;
   Pin& uz_2;
   Pin& uz_3;
   Pin& heater;
   Pin& led_fn_en;

   Buzzer& buzzer;

   Timer refresh {500_ms};
   Timer timer;
   
   uint16_t temperature{0};
   uint16_t set_temperature{flash.temp};
   
   uint16_t set_time {flash.time};
   uint16_t left_time{0};

   uint8_t mode_state{1};
   uint8_t n{0};

   bool on{false};
   bool hold{false};
   bool blink{false};
   bool setting_temp{false};
   bool setting_time{false};

   bool overheat{false};

   void temp (uint16_t adc) {
      adc = adc / conversion_on_channel;
      auto p = std::lower_bound(
         std::begin(NTC::u2904<U,R>),
         std::end(NTC::u2904<U,R>),
         adc,
         std::greater<uint32_t>());
      temperature = (p - NTC::u2904<U,R>);
   }

   void show_temperature (uint16_t t) {
      ssi.buffer[4] = t % 10;
      ssi.buffer[5] = t / 10;
   }

   void show_set_temperature (uint16_t t) {
      ssi.buffer[2] = t % 10;
      ssi.buffer[3] = t / 10;
   }

   void show_set_time (uint16_t t) {
      ssi.buffer[0] = t % 10;
      ssi.buffer[1] = t / 10;
   }

   void show_sign (Sign sign_1, Sign sign_2, int _1 = 2, int _2 = 3) {
      ssi.buffer[_1] = static_cast<uint8_t>(sign_2);
      ssi.buffer[_2] = static_cast<uint8_t>(sign_1);
   }

   void up_time(int i = 1) 
   { 
      set_time += 1; 
      if (set_time >= flash.max_time) set_time = flash.max_time;
      buzzer.brief();
   }

   void down_time(int i = 1) 
   { 
      set_time -= 1; 
      if (set_time <= 1) set_time = 1;
      buzzer.brief();
   }

   void up_temp(int i = 1) 
   { 
      set_temperature += 1; 
      if (set_temperature >= flash.max_temperature) set_temperature = flash.max_temperature;
      buzzer.brief();
   }

   void down_temp(int i = 1) 
   { 
      set_temperature -= 1; 
      if (set_temperature <= flash.min_temperature) set_temperature = flash.min_temperature;
      buzzer.brief();
   }

   void init_mode() {
      mode() == 0 ? deinit_temp() : init_temp();
      button_mode.set_down_callback(
         [&]{mode.change();
             mode() == 0 ? deinit_temp() : init_temp();
         buzzer.brief();}
      );
   }

   void init_time() {
      time_up.set_down_callback(
         [&]{set_time++; 
         if (set_time >= 99) set_time = 99;
         buzzer.brief();}
      );

      time_down.set_down_callback(
         [&]{set_time--; 
         if (set_time <= 1) set_time = 1;
         buzzer.brief();}
      );

      time_up.set_increment_callback(
         [this](auto i){up_time(i);}
      );

      time_down.set_increment_callback(
         [this](auto i){down_time(i);} 
      );
   }

   void init_temp() {
      temp_up.set_down_callback(
         [&]{set_temperature++; 
         if (set_temperature >= flash.max_temperature) set_temperature = flash.max_temperature;
         buzzer.brief();}
      );

      temp_down.set_down_callback(
         [&]{set_temperature--; 
         if (set_temperature <= flash.min_temperature) set_temperature = flash.min_temperature;
         buzzer.brief();}
      );

      temp_up.set_increment_callback(
         [this](auto i){up_temp(i);}
      );

      temp_down.set_increment_callback(
         [this](auto i){down_temp(i);} 
      );
   }

   void init_start() {
      start.set_down_callback(
         [&](){on ^= 1; hold = true; buzzer.brief();}
      );

      start.set_long_push_callback(
         [&](){hold = false; on = false; timer.stop(); buzzer.longer(); overheat = false;}
      );
   }

   void init_set_max_temp() {
      set_max_temp.set_long_push_callback(
         [&](){
            if (not setting_temp) {
               buzzer.longer();
               setting_temp = true;
               deinit();
               deinit_start();
               temp_up.set_long_push_callback(
                  [&]{flash.max_temperature++; 
                  if (flash.max_temperature >= 99) flash.max_temperature = 99;
                  buzzer.longer();}
               );
               temp_down.set_long_push_callback(
                  [&]{flash.max_temperature--; 
                  if (flash.max_temperature <= flash.min_temperature) flash.max_temperature = flash.min_temperature;
                  buzzer.longer();}
               );
            } else {
               if (setting_temp) {
                  setting_temp = false;
                  temp_up.set_long_push_callback(nullptr);
                  temp_down.set_long_push_callback(nullptr);
                  state = State::init;
               }
            }
         }
      );
   }

   void init_set_max_time() {
      set_max_time.set_long_push_callback(
         [&](){
            if (not setting_time) {
               buzzer.longer();
               setting_time = true;
               deinit();
               deinit_start();
               time_up.set_long_push_callback(
                  [&]{flash.max_time++; 
                  if (flash.max_time >= 99) flash.max_time = 99;
                  buzzer.longer();}
               );
               time_down.set_long_push_callback(
                  [&]{flash.max_time--; 
                  if (flash.max_time <= 0) flash.max_time = 0;
                  buzzer.longer();}
               );
            } else {
               if (setting_time) {
                  setting_time = false;
                  time_up.set_long_push_callback(nullptr);
                  time_down.set_long_push_callback(nullptr);
                  state = State::init;
               }
            }
         }
      );
   }

   void deinit_start() {
      start.set_down_callback(nullptr);
      start.set_long_push_callback(nullptr);
   }

   void deinit()
   {
      time_up.set_down_callback       (nullptr);
      time_down.set_down_callback     (nullptr);
      time_up.set_increment_callback  (nullptr);
      time_down.set_increment_callback(nullptr);
      button_mode.set_down_callback   (nullptr);
      deinit_temp();
   }

   void deinit_temp()
   {
      temp_up.set_down_callback       (nullptr);
      temp_down.set_down_callback     (nullptr);
      temp_up.set_increment_callback  (nullptr);
      temp_down.set_increment_callback(nullptr);
   }

   void level_control() {
      if (not level) {
         on = false;
         hold = false;
         uz_1   = false;
         uz_2   = false;
         uz_3   = false;
         heater = false;
         state = State::emergency;
         timer.stop();
         deinit();
         deinit_start();
      }
   }

   void cover_control() {
      if (not cover and state == State::pusk) {
         uz_1   = false;
         uz_2   = false;
         uz_3   = false;
         heater = false;
         state = State::pause;
         timer.pause();
         deinit();
         deinit_start();
      }
   }

   bool heating() 
   {
      switch (heat)
      {
         case _1:
            if ((state == State::pusk or state == State::pause or state == State::preheating ) and mode() != 0) {
               heater = temperature < (set_temperature) ? true : false;
               heat = Heat::_2;
            } else {
               heater = false;
            }
         break;
         case _2:
            if (temperature >= set_temperature) {
               heater = false;
            } else if (temperature < (set_temperature - 1)) {
               heater = true;
            }
         break;
      }

      if (state == State::emergency) {
         heater = false;
      }
      
      return (not heater);
   }


   
};