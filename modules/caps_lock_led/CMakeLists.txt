if(CONFIG_ZMK_CAPS_LOCK_LED)
    zephyr_library()
    zephyr_library_sources(src/caps_lock_led.c)
endif()
