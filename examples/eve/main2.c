/**
 * This example shows features related to the touch screen of the EVE Click
 * wrapper of the LetMeCreate library.
 *
 * It first performs calibration of the touch screen, then it prints the
 * coordinates of the touch event. Press Ctrl+C to exit program.
 *
 * Before running this program:
 *   - the EVE Click must be inserted in Mikrobus 1
 *   - a WQVGA screen must be connected to the EVE Click.
 */
#include <linux/spi/spidev.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <letmecreate/letmecreate.h>

static volatile bool running = true;
static volatile bool touch_screen_event = false;
static uint16_t last_event_x = 0, last_event_y = 0;

static void exit_program(int __attribute__ ((unused))signo)
{
    running = false;
}

static void callback(uint16_t x, uint16_t y)
{
    last_event_x = x;
    last_event_y = y;
    touch_screen_event = true;
}

int main(void)
{
    /* Set signal handler to exit program when Ctrl+c is pressed */
    struct sigaction action = {
        .sa_handler = exit_program,
        .sa_flags = 0
    };
    sigemptyset(&action.sa_mask);
    sigaction (SIGINT, &action, NULL);

    spi_init();
    spi_set_mode(MIKROBUS_1, SPI_MODE_0);

    if (eve_click_enable(MIKROBUS_1) < 0)
        return -1;

    /* Perform calibration of touch screen */
    if (eve_click_calibrate() < 0)
        goto exit;

    printf("Press Ctrl+C to exit program.\n");

    /* Add your own callback after calibration, otherwise you might get
     * incorrect coordinates.
     */
    eve_click_attach_touch_callback(callback);

    eve_click_clear(0, 0, 0);
    eve_click_draw(FT800_TEXT,
                   240,
                   136,
                   31,
                   FT800_OPT_CENTER,
                   "Tap on the screen");
    eve_click_display();

    while (running) {
        char str[255];

        /* Wait until the user touches the screen or that it exits */
        while (touch_screen_event == false && running == true)
            ;

        if (running == false)
            goto exit;

        snprintf(str, 255, "x: %u, y: %u", last_event_x, last_event_y);

        eve_click_clear(0, 0, 0);
        eve_click_draw(FT800_TEXT,
                       240,
                       136,
                       25,
                       FT800_OPT_CENTER,
                       "Touch event detected at:");
        eve_click_draw(FT800_TEXT,
                       240,
                       180,
                       25,
                       FT800_OPT_CENTER,
                       str);
        eve_click_display();
    }

exit:
    eve_click_disable(MIKROBUS_1);
    spi_release();

    return 0;
}
