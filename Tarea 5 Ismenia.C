#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_PIN 2  

bool led_state = false;  

void task_toggle_led(void *pvParameter);  

void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    xTaskCreate(task_toggle_led, "Task LED Blink", 2048, NULL, 5, NULL);
}

void task_toggle_led(void *pvParameter)
{
    while (true)  
    {
        led_state = !led_state;
        gpio_set_level(LED_PIN, led_state);
        printf("LED ahora está: %s\n", led_state ? "ON" : "OFF");
        vTaskDelay(pdMS_TO_TICKS(150));  
    }
}
