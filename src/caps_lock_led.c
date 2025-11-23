#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/hid.h>  // For BIT(1) if needed, but use ZMK_HID_INDICATOR_CAPS_LOCK if available

LOG_MODULE_REGISTER(caps_lock_led, LOG_LEVEL_DBG);  // DBG for verbose output

static const struct device *led_dev = DEVICE_DT_GET(DT_NODELABEL(caps_leds));

static int init_caps_lock_led(void) {
    if (!device_is_ready(led_dev)) {
        LOG_ERR("Caps lock LED device not ready at boot");
        return -ENODEV;
    }
    LOG_INF("Caps lock LED module initialized successfully");
    return 0;
}

struct zmk_event_header;  // Forward declaration

static int caps_lock_callback(const struct zmk_event_header *eh) {
    struct zmk_hid_indicators_changed *ev = (struct zmk_hid_indicators_changed *)eh;
    LOG_DBG("HID indicators changed event received: indicators = 0x%02X", ev->indicators);

    if (device_is_ready(led_dev)) {
        if (ev->indicators & BIT(1)) {  // Caps Lock bit (0x02)
            LOG_DBG("Turning caps lock LED ON");
            led_on(led_dev, 0);
        } else {
            LOG_DBG("Turning caps lock LED OFF");
            led_off(led_dev, 0);
        }
    } else {
        LOG_ERR("Caps lock LED device not ready");
    }

    return 0;
}

ZMK_LISTENER(caps_lock_led_listener, caps_lock_callback);
ZMK_SUBSCRIPTION(caps_lock_led_listener, zmk_hid_indicators_changed);

SYS_INIT(init_caps_lock_led, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);  // Boot init
