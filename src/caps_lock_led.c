#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/hid.h>

// Safely include backlight header only if the feature is enabled
#if IS_ENABLED(CONFIG_ZMK_BACKLIGHT)
    #include <zmk/backlight.h>
#endif

LOG_MODULE_REGISTER(caps_lock_led, LOG_LEVEL_DBG);

// Default brightness if backlight is disabled or unavailable
#define DEFAULT_CAPS_BRIGHTNESS 50

// Get the PARENT device (the controller) using the node label defined in DTS
// Assumes a structure like: caps_leds: caps_pwm_leds { caps_lock_led: ... }
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
        
        int brightness_percent;

#if IS_ENABLED(CONFIG_ZMK_BACKLIGHT)
        // 1. Get the current persistent backlight level (0-100% or levels)
        // If backlight is off, this might return 0, so we handle that.
        int current_level = zmk_backlight_get_level();
        
        // 2. Convert level to percentage (0-100)
        brightness_percent = zmk_backlight_calc_brighness_val(current_level);

        // 3. Logic: If backlight is ON, make Caps Lock slightly brighter (+20%)
        // If backlight is OFF (0%), keep Caps Lock somewhat visible (e.g. 20%) or off?
        // Let's assume if backlight is 0, we still want Caps Lock visible if active.
        if (brightness_percent == 0) {
             brightness_percent = 30; // Standalone brightness if backlight is off
        } else {
             brightness_percent += 20; // Boost above ambient backlight
        }
#else
        // Fallback if CONFIG_ZMK_BACKLIGHT is not enabled in Kconfig
        brightness_percent = DEFAULT_CAPS_BRIGHTNESS;
#endif
        
        // 4. Clamp to maximum 100% to avoid driver errors
        if (brightness_percent > 100) {
            brightness_percent = 100;
        }

        // 5. Final calculation: 0 if inactive, calculated percent if active
        int final_brightness = caps_lock_active ? brightness_percent : 0;
        
        // Caps Lock LED is index 0 of the 'caps_leds' parent device
        int ret = led_set_brightness(led_dev, 0, final_brightness); 

        if (ret != 0) {
            LOG_ERR("Failed to set Caps Lock LED brightness (err %d)", ret);
        } else {
            LOG_DBG("Caps Lock LED state updated: brightness=%d", final_brightness);
        }

    } else {
        LOG_ERR("Caps lock LED device not ready");
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(caps_lock_led_listener, caps_lock_callback);
ZMK_SUBSCRIPTION(caps_lock_led_listener, zmk_hid_indicators_changed);

SYS_INIT(init_caps_lock_led, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
