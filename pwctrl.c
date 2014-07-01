#include "pwctrl.h"
#include <stdlib.h> 
#include <unistd.h>

void sim900a_power_on(void)
{
	system("echo 0 > /sys/class/leds/sim900_power_en/brightness");
	sleep(1);
	system("echo 1 > /sys/class/leds/sim900_power_en/brightness");	
	sleep(1);
	system("echo 1 > /sys/class/leds/sim900_pwrkey/brightness");
	sleep(1);
}

void sim900a_power_off(void)
{	
	system("echo 0 > /sys/class/leds/sim900_pwrkey/brightness");
	sleep(1);
	system("echo 0 > /sys/class/leds/sim900_power_en/brightness");
	sleep(1);
}


void td3020c_power_on(void)
{
	system("echo 1 > /sys/class/leds/gps_power_en/brightness");
	sleep(1);
	system("echo 0 > /sys/class/leds/gps_reset/brightness");	
	sleep(1);
	system("echo 0 > /sys/class/leds/pwr_dwn/brightness");
	td3020c_led_on();
	
	
		
}
void td3020c_power_off(void)
{	
	system("echo 0 > /sys/class/leds/gps_power_en/brightness");
	sleep(1);
       td3020c_led_off();
}

void td3020c_led_on(void)
{
	system("echo 1 > /sys/class/leds/led_green_td/brightness");
}
void td3020c_led_off(void)
{
	system("echo 0 > /sys/class/leds/led_green_td/brightness");
}
void td3020c_led_ctrl(void)
{
	td3020c_led_on();
	sleep(1);
	td3020c_led_off();
}
