zephyr_include_directories(${WEST_TOPDIR}/zmk/include)
target_sources(app PRIVATE src/caps_lock_led.c)
