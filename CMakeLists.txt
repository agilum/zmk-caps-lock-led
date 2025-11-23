if(CONFIG_ZMK_CAPS_LOCK_LED)
    zephyr_library()
    zephyr_library_sources(src/caps_lock_led.c)
endif()

#zephyr_include_directories(${WEST_TOPDIR}/zmk/include)
#target_sources(app PRIVATE src/caps_lock_led.c)
