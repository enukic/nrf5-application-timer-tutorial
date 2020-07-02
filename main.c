#include <stdbool.h>
#include "boards.h"
#include "bsp.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"


APP_TIMER_DEF(m_repeated_timer_id);     /**< Handler for repeated timer used to blink LED 1. */
APP_TIMER_DEF(m_single_shot_timer_id);  /**< Handler for single shot timer used to light LED 2. */

static uint32_t timeout = 0;


/**@brief Button event handler function.
 *
 * @details Responsible for controlling LEDs based on button presses.
 */
void button_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    ret_code_t      err_code;
    switch (pin)
    {
    case BUTTON_1:
        // Start repeated timer (start blinking LED).
        err_code = app_timer_start(m_repeated_timer_id, APP_TIMER_TICKS(200), NULL);
        APP_ERROR_CHECK(err_code);
        break;
    case BUTTON_2:
        // Stop the repeated timer (stop blinking LED).
        err_code = app_timer_stop(m_repeated_timer_id);
        APP_ERROR_CHECK(err_code);
        break;
    case BUTTON_3:
        // Start single shot timer which turns on LED2 when it expires.
        // Increase the timeout with 1 second every time.
        timeout += 1000;
        err_code = app_timer_start(m_single_shot_timer_id, APP_TIMER_TICKS(timeout), NULL);
        APP_ERROR_CHECK(err_code);
    case BUTTON_4:
        // Turn off LED 2.
        nrf_drv_gpiote_out_set(LED_2);
        break;
    default:
        break;
    }
}


/**@brief Function for initializing GPIO pins.
 */
static void gpio_config()
{
    ret_code_t err_code;

    // Initialize GPIOTE driver.
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Configure output pins for LEDs.
    nrf_gpio_range_cfg_output(LED_1, LED_2);

    // Set output pins (this will turn off the LEDs).
    nrf_drv_gpiote_out_set(LED_1);
    nrf_drv_gpiote_out_set(LED_2);

    // Make a configuration for input pints. This is suitable for both pins in this example.
    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    // Configure input pins for 4 buttons, all using the same event handler.
    err_code = nrf_drv_gpiote_in_init(BUTTON_1, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_2, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_3, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(BUTTON_4, &in_config, button_event_handler);
    APP_ERROR_CHECK(err_code);

    // Enable input pins for buttons.
    nrf_drv_gpiote_in_event_enable(BUTTON_1, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_2, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_3, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_4, true);
}


/**@brief Function for initializing logging.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


static void lfclk_request(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
}

static void repeated_timer_handler(void * p_context)
{
    nrf_drv_gpiote_out_toggle(LED_1);
}

static void single_shot_timer_handler(void * p_context)
{
    nrf_drv_gpiote_out_clear(LED_2);
}

static void create_timers()
{
    ret_code_t err_code;

    // Create timers
    err_code = app_timer_create(&m_repeated_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                repeated_timer_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_single_shot_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                single_shot_timer_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Main function.
 */
int main(void)
{
    lfclk_request();

    app_timer_init();
    log_init();
    gpio_config();
    
    create_timers();

    NRF_LOG_INFO("Application timer tutorial example started.");

    // Enter main loop.
    while (true)
    {
        __WFI();
    }
}
