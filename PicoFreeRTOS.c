/*
 * Traffic Light Controller using FreeRTOS on Raspberry Pi Pico
 * 
 * This program implements a smart traffic light system with:
 * - LED matrix display for countdown
 * - RGB LED for color indication
 * - OLED display for messages
 * - Buzzer for audio feedback
 * - Button for mode switching
 */

// Hardware and OS headers
#include "pico/stdlib.h"         // Pico SDK core functionality
#include "FreeRTOS.h"            // FreeRTOS core
#include "task.h"                // FreeRTOS task management
#include <stdio.h>               // Standard I/O
#include "lib/ws2812b.h"         // WS2812B LED matrix control
#include "pico/bootrom.h"        // Boot ROM utilities
#include "hardware/clocks.h"     // Clock control
#include "lib/rgb.h"             // RGB LED control
#include "lib/mlt8530.h"         // Buzzer control
#include "lib/oledgfx.h"         // OLED display graphics
#include "lib/push_button.h"     // Button handling

// Hardware pin definitions
#define led_pin_red 12           // Red LED pin (unused in current implementation)
#define botaoB 6                 // Button B pin

// Semaphore timing configuration (in seconds)
#define SEMAPHORE_GREEN_DURATION_SEC    9  
#define SEMAPHORE_RED_DURATION_SEC      6  
#define SEMAPHORE_YELLOW_DURATION_SEC   3
#define SEMAPHORE_DURATION_TIMEOUT      0   // Value when countdown reaches zero

// Semaphore state definitions
#define SEMAPHORE_GREEN_STATE  1
#define SEMAPHORE_RED_STATE    2
#define SEMAPHORE_YELLOW_STATE 0

// LED color codes
#define SEMAPHORE_LED_COLOR_RED    0
#define SEMAPHORE_LED_COLOR_GREEN  1
#define SEMAPHORE_LED_COLOR_YELLOW 3

// Operation modes
#define SEMAPHORE_DAILY_MODE 0    // Normal day mode with full cycle
#define SEMAPHORE_NIGHT_MODE 1    // Night mode (yellow blinking)

// ==================== Hardware Configuration ====================

/// I2C port used for OLED communication
#define I2C_PORT i2c1

/// OLED display pin configuration
#define OLED_SDA 14      ///< Serial data pin
#define OLED_SCL 15      ///< Serial clock pin
#define OLED_ADDR 0x3C   ///< I2C address of OLED
#define OLED_BAUDRATE 400000  ///< I2C communication speed

/// Joystick pin configuration (unused in current implementation)
#define JOYSTICK_VRX 27  ///< X-axis analog input
#define JOYSTICK_VRY 26  ///< Y-axis analog input
#define JOYSTICK_PB  22  ///< Push button input

/// Buzzer pin configuration
#define BUZZER_A 10      ///< Primary buzzer pin
#define BUZZER_B 21      ///< Secondary buzzer pin (unused)

/// RGB LED pin configuration
#define RED_PIN   13     ///< Red LED PWM pin
#define BLUE_PIN  12     ///< Blue LED PWM pin (unused)
#define GREEN_PIN 11     ///< Green LED PWM pin

/// Button definitions
#define BUTTON_A 5       ///< Mode switch button

// Global state variables
static volatile uint16_t g_semaphore_counter = SEMAPHORE_GREEN_DURATION_SEC;  // Current countdown value
static volatile uint8_t g_sempahore_state = SEMAPHORE_GREEN_STATE;            // Current light state
static volatile uint8_t g_semaphore_led_color = SEMAPHORE_LED_COLOR_GREEN;    // Current LED color
static volatile uint8_t g_semaphore_mode = SEMAPHORE_DAILY_MODE;              // Current operation mode

/**
 * @brief Updates the semaphore counter and transitions states when timeout occurs
 * @param current_state The current semaphore state
 */
void update_semaphore_counter(uint8_t current_state)
{
    if(g_semaphore_counter == SEMAPHORE_DURATION_TIMEOUT)
    {
        switch(current_state)
        {
            case SEMAPHORE_GREEN_STATE:
                // Transition to yellow state
                g_semaphore_counter = SEMAPHORE_YELLOW_DURATION_SEC;
                g_sempahore_state = SEMAPHORE_YELLOW_STATE;
                g_semaphore_led_color = SEMAPHORE_LED_COLOR_YELLOW;
                break;
            case SEMAPHORE_YELLOW_STATE:
                // Transition to red state
                g_semaphore_counter = SEMAPHORE_RED_DURATION_SEC;
                g_sempahore_state = SEMAPHORE_RED_STATE;
                g_semaphore_led_color = SEMAPHORE_LED_COLOR_RED;
                break;
            case SEMAPHORE_RED_STATE:
                // Transition to green state (complete cycle)
                g_semaphore_counter = SEMAPHORE_GREEN_DURATION_SEC;
                g_sempahore_state = SEMAPHORE_GREEN_STATE;
                g_semaphore_led_color = SEMAPHORE_LED_COLOR_GREEN;
                break;
        }
    }
}

/**
 * @brief Task to handle button presses for mode switching
 * @param pvParameters Task parameters (unused)
 */
void vPushButtonTask(void *pvParameters)
{
    while(1)
    {
        // Toggle mode when button A is pressed
        if(!gpio_get(BUTTON_A))
        {
            if (g_semaphore_mode == SEMAPHORE_NIGHT_MODE)
            {
                g_semaphore_counter = SEMAPHORE_GREEN_DURATION_SEC;
                g_semaphore_led_color = SEMAPHORE_LED_COLOR_GREEN;
                g_sempahore_state = SEMAPHORE_GREEN_STATE;
                g_semaphore_mode = SEMAPHORE_DAILY_MODE;
            }
            else g_semaphore_mode = SEMAPHORE_NIGHT_MODE;
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // Debounce delay
    }
}

/**
 * @brief Task to update the OLED display with current state messages
 * @param pvParameters Pointer to SSD1306 display structure
 */
void vDisplayTask(void *pvParameters)
{
    ssd1306_t *ssd = (ssd1306_t *) pvParameters;
    while(1)
    {
        oledgfx_clear_line(ssd, 40);
        if(g_semaphore_mode == SEMAPHORE_DAILY_MODE)
        {
            if(g_sempahore_state == SEMAPHORE_GREEN_STATE) 
                ssd1306_draw_string(ssd, "Siga", 24, 40);
            else if(g_sempahore_state == SEMAPHORE_YELLOW_STATE) 
                ssd1306_draw_string(ssd, "Atencao", 24, 40);
            else if(g_sempahore_state == SEMAPHORE_RED_STATE) 
                ssd1306_draw_string(ssd, "Pare", 24, 40);
        }
        // Display appropriate message based on current state
        oledgfx_render(ssd);  // Update display
        vTaskDelay(pdMS_TO_TICKS(1000));  // Update once per second
    }
}

/**
 * @brief Task to control buzzer patterns based on semaphore state
 */
void vBuzzerTask()
{
    while(1)
    {
        if(g_semaphore_mode == SEMAPHORE_DAILY_MODE)
        {
            if(g_sempahore_state == SEMAPHORE_GREEN_STATE)
            {
                // Green state: short beep once per second
                buzzer_beep(BUZZER_A, 251, 300);
                vTaskDelay(pdMS_TO_TICKS(749));
            }
            else if(g_sempahore_state == SEMAPHORE_YELLOW_STATE)
            {
                // Yellow state: rapid beeping
                buzzer_beep(BUZZER_A, 251, 300);
            }
            else if(g_sempahore_state == SEMAPHORE_RED_STATE)
            {
                // Red state: longer beep every 1.5 seconds
                buzzer_beep(BUZZER_A, 500, 300);
                vTaskDelay(pdMS_TO_TICKS(1500));
            }
        }
        else
        {
            buzzer_beep(BUZZER_A, 500, 300);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}

/**
 * @brief Task to control RGB LED colors based on semaphore state
 * @param pvParameters Pointer to RGB LED structure
 */
void vLedColorTask(void *pvParameters)
{
    rgb_t *led_rgb = (rgb_t *) pvParameters;
    while(1)
    {
        if(g_semaphore_mode == SEMAPHORE_DAILY_MODE)
        {
            if(g_sempahore_state == SEMAPHORE_GREEN_STATE)
            {
                printf("VERDE\n");
                rgb_turn_on_by_color(led_rgb, RGB_COLOR_GREEN);
            }
            else if(g_sempahore_state == SEMAPHORE_YELLOW_STATE)
            {
                printf("AMARELO\n");
                rgb_turn_on_by_color(led_rgb, RGB_COLOR_YELLOW);
            }
            else if(g_sempahore_state == SEMAPHORE_RED_STATE)
            {
                printf("VERMELHO\n");
                rgb_turn_on_by_color(led_rgb, RGB_COLOR_RED);
            }
        }
        else
        {
            rgb_turn_on_by_color(led_rgb, RGB_COLOR_YELLOW);
        }
        vTaskDelay(pdMS_TO_TICKS(1));  // Small delay to prevent task starvation
    }
}

/**
 * @brief Main task to control LED matrix countdown display
 * @param pvParameters Pointer to WS2812B LED matrix structure
 */
void vBlinkTask(void *pvParameters)
{
    ws2812b_t *ws = (ws2812b_t *) pvParameters;
    while (true)
    {
        // Display current countdown number with appropriate color
        if(g_semaphore_mode == SEMAPHORE_DAILY_MODE)
            ws2812b_draw(ws, NUMERIC_GLYPHS[g_semaphore_counter--], g_semaphore_led_color, 1);
        else
            ws2812b_draw(ws, NUMERIC_GLYPHS[0], WS2812B_COLOR_YELLOW, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));  // Update every second
        
        // Check for state transitions
        update_semaphore_counter(g_sempahore_state);
    }
}

/**
 * @brief IRQ handler for BOOTSEL button (Button B)
 * @param gpio GPIO pin that triggered the interrupt
 * @param events Type of interrupt event
 */
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);  // Enter USB bootloader mode
}

/**
 * @brief Main application entry point
 */
int main()
{
    // Configure system clock
    set_sys_clock_khz(128000, false);
    
    // Initialize hardware peripherals
    pb_config_btn_a();  // Configure button A (mode switch)
    pb_config_btn_b();  // Configure button B (BOOTSEL)
    pb_set_irq_callback(&gpio_irq_handler);
    pb_enable_irq(BUTTON_B);
    
    stdio_init_all();  // Initialize stdio for debug output
    
    // Initialize hardware components
    ws2812b_t ws;  // LED matrix
    rgb_t rgb;     // RGB LED
    ssd1306_t ssd; // OLED display
    
    // OLED display initialization
    oledgfx_init_all(&ssd, I2C_PORT, OLED_BAUDRATE, OLED_SDA, OLED_SCL, OLED_ADDR);
    buzzer_init(BUZZER_A);
    ws2812b_init(&ws, pio0, WS2812B_PIN);
    
    // Draw initial display content
    oledgfx_clear_screen(&ssd);
    oledgfx_draw_border(&ssd, BORDER_LIGHT);
    ssd1306_line(&ssd, 3, 25, 123, 25, 1);
    ssd1306_line(&ssd, 3, 37, 123, 37, 1);
    ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6);
    ssd1306_draw_string(&ssd, "TrafficLightRTOS", 0, 16);
    ssd1306_draw_string(&ssd, "  FreeRTOS", 10, 28);
    
    // Initialize RGB LED
    rgb_init_all(&rgb, RED_PIN, GREEN_PIN, BLUE_PIN, 1.0, 255);
    oledgfx_render(&ssd);
    
    // Create FreeRTOS tasks
    xTaskCreate(vBlinkTask, "Blink Task", 
        configMINIMAL_STACK_SIZE, (void *) &ws, tskIDLE_PRIORITY + 4, NULL);
    xTaskCreate(vLedColorTask, "LED RGB Task", 
        configMINIMAL_STACK_SIZE, (void *) &rgb, tskIDLE_PRIORITY + 3, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer task", 
        configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vDisplayTask, "Display Task", 
        configMINIMAL_STACK_SIZE, (void *) &ssd, tskIDLE_PRIORITY + 1, NULL);
        xTaskCreate(vPushButtonTask, "Change Mode Button", 
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    // Start the RTOS scheduler
    vTaskStartScheduler();
    
    // Should never reach here
    panic_unsupported();
}