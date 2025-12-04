#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/hid.h>
#include <zmk/backlight.h> 

LOG_MODULE_REGISTER(caps_lock_led, LOG_LEVEL_DBG);

// Get the PARENT device (the controller) using the node label defined in DTS
static const struct device *led_dev = DEVICE_DT_GET(DT_NODELABEL(caps_leds));

static int init_caps_lock_led(void) {
    if (!device_is_ready(led_dev)) {
        LOG_ERR("Caps lock LED device not ready at boot");
        return -ENODEV;
    }
    LOG_INF("Caps lock LED module initialized successfully");
    return 0;
}

static int caps_lock_callback(const struct zmk_event_t *eh) {
    const struct zmk_hid_indicators_changed *ev = as_zmk_hid_indicators_changed(eh);
    if (ev == NULL) return ZMK_EV_EVENT_BUBBLE;

    LOG_DBG("HID indicators changed event received: indicators = 0x%02X", ev->indicators);

    if (device_is_ready(led_dev)) {
        // Caps Lock is the second bit (BIT(1)) in the HID indicators mask
        bool caps_lock_active = (ev->indicators & BIT(1));
        
        // 1. Get the current persistent backlight level (0-8)
        int current_level = zmk_backlight_get_level();
        
        // 2. Convert the level index into a 0-100 duty cycle percentage
        int brightness_percent = zmk_backlight_calc_brighness_val(current_level);

        // 3. Add offset (e.g., +20%) to make it brighter than the backlight
        if (brightness_percent > 0) { // Only boost if backlight is arguably on/active logic
             brightness_percent += 20; 
        }
        
        // 4. Clamp to maximum 100%
        if (brightness_percent > 100) {
            brightness_percent = 100;
        }

        // 5. Set LED brightness: Use the calculated percentage if active, 0% if inactive
        int brightness = caps_lock_active ? brightness_percent : 0;
        
        // The Caps Lock LED is the first child node of caps_leds, so index is 0
        int ret = led_set_brightness(led_dev, 0, brightness); 

        if (ret != 0) {
            LOG_ERR("Failed to set Caps Lock LED brightness (err %d)", ret);
        } else {
            LOG_DBG("Caps Lock LED state updated: brightness=%d (from level %d)", brightness, current_level);
        }

    } else {
        LOG_ERR("Caps lock LED device not ready");
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(caps_lock_led_listener, caps_lock_callback);
ZMK_SUBSCRIPTION(caps_lock_led_listener, zmk_hid_indicators_changed);

SYS_INIT(init_caps_lock_led, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
