#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>  // For struct zmk_event_header, ZMK_LISTENER, ZMK_SUBSCRIPTION
#include <zmk/events/hid_indicators_changed.h>  // For the event struct
#include <zmk/hid.h>  // For ZMK_HID_INDICATOR_CAPS_LOCK

LOG_MODULE_REGISTER(caps_lock_led, CONFIG_ZMK_LOG_LEVEL);

static const struct device *led_dev = DEVICE_DT_GET(DT_NODELABEL(caps_leds));

static int caps_lock_callback(const struct zmk_event_header *eh) {
    struct zmk_hid_indicators_changed *ev = (struct zmk_hid_indicators_changed *)eh;

    if (device_is_ready(led_dev)) {
        if (ev->indicators & ZMK_HID_INDICATOR_CAPS_LOCK) {
            led_on(led_dev, 0);
        } else {
            led_off(led_dev, 0);
        }
    } else {
        LOG_ERR("Caps lock LED device not ready");
    }

    return 0;
}

ZMK_LISTENER(caps_lock_led_listener, caps_lock_callback);
ZMK_SUBSCRIPTION(caps_lock_led_listener, zmk_hid_indicators_changed);
