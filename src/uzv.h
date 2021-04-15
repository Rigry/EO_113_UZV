#pragma once

#include "pin.h"
#include "adc.h"
#include "flash.h"
#include "timers.h"
#include "NTC_table.h"



constexpr auto conversion_on_channel {16};
struct ADC_{
   ADC_average& control     = ADC_average::make<mcu::Periph::ADC1>(conversion_on_channel);
   ADC_channel& temperature = control.add_channel<mcu::PA0>();
};

template<class Flash_data>
class UZV
{
   ADC_& adc;
   Flash_data& flash;
   
   Button_event& time_up;
   Button_event& time_down;
   Button_event& temp_up;
   Button_event& temp_down;
   Button_event& start;
   Button_event& mode;
   Button_event& f_en;

   Pin& cover;
   Pin& level;

   Pin& uz_1;
   Pin& uz_2;
   Pin& uz_3;
   Pin& heater;
   Pin& led_m_1;
   Pin& led_m_2;
   Pin& led_m_3;
   Pin& led_fn_en;
   
   uint16_t temperature{0};

   const size_t U = 33;
   const size_t R = 5100;

   void temp (uint16_t adc) {
      adc = adc / conversion_on_channel;
      auto p = std::lower_bound(
         std::begin(NTC::u2904<33,5100>),
         std::end(NTC::u2904<33,5100>),
         adc,
         std::greater<uint32_t>());
      temperature = (p - NTC::u2904<33,5100>);
   }
public:

   UZV (
        ADC_& adc
      , Flash_data& flash
      , Button_event& time_up
      , Button_event& time_down
      , Button_event& temp_up
      , Button_event& temp_down
      , Button_event& start
      , Button_event& mode
      , Button_event& f_en
      , Pin& cover
      , Pin& level
      , Pin& uz_1
      , Pin& uz_2
      , Pin& uz_3
      , Pin& heater
      , Pin& led_m_1
      , Pin& led_m_2
      , Pin& led_m_3
      , Pin& led_fn_en
   )  : adc       {adc}
      , flash     {flash}
      , time_up   {time_up}
      , time_down {time_down}
      , temp_up   {temp_up}
      , temp_down {temp_down}
      , start     {start}
      , mode      {start}
      , f_en      {f_en}
      , cover     {cover}
      , level     {level}
      , uz_1      {uz_1}
      , uz_2      {uz_2}
      , uz_3      {uz_3}
      , heater    {heater}
      , led_m_1   {led_m_1}
      , led_m_2   {led_m_2}
      , led_m_3   {led_m_3}
      , led_fn_en {led_fn_en}
      {
         adc.control.start();
      }

   
   void operator() () {

      temp(adc.temperature);
      
      
   }
   
};