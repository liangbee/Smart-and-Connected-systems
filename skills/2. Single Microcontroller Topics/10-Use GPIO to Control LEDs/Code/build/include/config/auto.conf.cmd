deps_config := \
	/Users/nliang/esp/esp-idf/components/app_trace/Kconfig \
	/Users/nliang/esp/esp-idf/components/aws_iot/Kconfig \
	/Users/nliang/esp/esp-idf/components/bt/Kconfig \
	/Users/nliang/esp/esp-idf/components/driver/Kconfig \
	/Users/nliang/esp/esp-idf/components/esp32/Kconfig \
	/Users/nliang/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/Users/nliang/esp/esp-idf/components/esp_http_client/Kconfig \
	/Users/nliang/esp/esp-idf/components/ethernet/Kconfig \
	/Users/nliang/esp/esp-idf/components/fatfs/Kconfig \
	/Users/nliang/esp/esp-idf/components/freertos/Kconfig \
	/Users/nliang/esp/esp-idf/components/heap/Kconfig \
	/Users/nliang/esp/esp-idf/components/http_server/Kconfig \
	/Users/nliang/esp/esp-idf/components/libsodium/Kconfig \
	/Users/nliang/esp/esp-idf/components/log/Kconfig \
	/Users/nliang/esp/esp-idf/components/lwip/Kconfig \
	/Users/nliang/esp/esp-idf/components/mbedtls/Kconfig \
	/Users/nliang/esp/esp-idf/components/mdns/Kconfig \
	/Users/nliang/esp/esp-idf/components/openssl/Kconfig \
	/Users/nliang/esp/esp-idf/components/pthread/Kconfig \
	/Users/nliang/esp/esp-idf/components/spi_flash/Kconfig \
	/Users/nliang/esp/esp-idf/components/spiffs/Kconfig \
	/Users/nliang/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/Users/nliang/esp/esp-idf/components/vfs/Kconfig \
	/Users/nliang/esp/esp-idf/components/wear_levelling/Kconfig \
	/Users/nliang/esp/esp-idf/Kconfig.compiler \
	/Users/nliang/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/Users/nliang/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/Users/nliang/esp/blink/main/Kconfig.projbuild \
	/Users/nliang/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/Users/nliang/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
