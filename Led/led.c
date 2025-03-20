#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#define LED_PIN 2       // Modificar según el hardware
#define BUTTON_PIN 4    // Modificar según el hardware

TimerHandle_t xTimerBlink;
volatile TickType_t buttonPressDuration = 0;
volatile bool buttonPressed = false;
TaskHandle_t xTaskHandle = NULL;

void IRAM_ATTR buttonISR(void *arg) {
    static TickType_t pressStartTime = 0;
    if (gpio_get_level(BUTTON_PIN) == 1) {
        // Botón presionado: iniciar conteo de tiempo
        pressStartTime = xTaskGetTickCount();
        gpio_set_level(LED_PIN, 1);
    } else {
        // Botón liberado: calcular tiempo y notificar a la tarea
        buttonPressDuration = xTaskGetTickCount() - pressStartTime;
        gpio_set_level(LED_PIN, 0);
        if (buttonPressDuration > 0) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyFromISR(xTaskHandle, buttonPressDuration, eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                portYIELD_FROM_ISR();
            }
        }
    }
}

void vTimerBlinkCallback(TimerHandle_t xTimer) {
    static bool ledState = false;
    ledState = !ledState;
    gpio_set_level(LED_PIN, ledState);
}

void vTaskLedBlink(void *pvParameters) {
    TickType_t receivedTime;
    while (1) {
        if (xTaskNotifyWait(0, 0, &receivedTime, portMAX_DELAY) == pdTRUE) {
            TickType_t blinkPeriod = pdMS_TO_TICKS(200);
            TickType_t totalBlinks = receivedTime / blinkPeriod;
            if (totalBlinks == 0) totalBlinks = 1;

            xTimerChangePeriod(xTimerBlink, blinkPeriod, 0);
            xTimerStart(xTimerBlink, 0);
            vTaskDelay(receivedTime);
            xTimerStop(xTimerBlink, 0);
            gpio_set_level(LED_PIN, 0);
        }
    }
}

void app_main() {
    // Configuración de GPIOs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_PIN) | (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);

    // Crear timer
    xTimerBlink = xTimerCreate("BlinkTimer", pdMS_TO_TICKS(200), pdTRUE, (void *)0, vTimerBlinkCallback);

    // Crear tarea para manejar los blinks
    xTaskCreate(vTaskLedBlink, "TaskLedBlink", 2048, NULL, 5, &xTaskHandle);

    // Configurar interrupciones del botón
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, buttonISR, NULL);
}

