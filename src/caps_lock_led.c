#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>  // Updated include for the event
#include <zmk/hid_indicators_types.h>  // Added for indicator bit definitions like ZMK_HID_INDICATOR_CAPS_LOCK

LOG_MODULE_REGISTER(caps_lock_led, CONFIG_ZMK_LOG_LEVEL);

static const struct device *led_dev = DEVICE_DT_GET(DT_NODELABEL(caps_leds));

static int caps_lock_callback(const struct zmk_event_header *eh) {
    struct zmk_hid_indicators_changed *ev = (struct zmk_hid_indicators_changed *)eh;

    if (device_is_ready(led_dev)) {
        if (ev->indicators & ZMK_HID_INDICATOR_CAPS_LOCK) {
            led_on(led_dev, 0);  // Turn on the LED (index 0, since it's the only one in the node)
        } else {
            led_off(led_dev, 0);  // Turn off the LED
        }
    } else {
        LOG_ERR("Caps lock LED device not ready");
    }

    return 0;
}

ZMK_LISTENER(caps_lock_led_listener, caps_lock_callback);
ZMK_SUBSCRIPTION(caps_lock_led_listener, zmk_hid_indicators_changed);
