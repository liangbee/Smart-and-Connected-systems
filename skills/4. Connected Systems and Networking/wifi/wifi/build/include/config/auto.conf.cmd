deps_config := \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/app_trace/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/aws_iot/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/bt/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/driver/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/esp32/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/esp_http_client/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/ethernet/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/fatfs/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/freertos/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/heap/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/http_server/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/libsodium/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/log/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/lwip/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/mbedtls/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/mdns/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/mqtt/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/nvs_flash/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/openssl/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/pthread/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/spi_flash/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/spiffs/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/vfs/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/wear_levelling/Kconfig \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/Users/emily/Desktop/ec444/esp/simple-server/main/Kconfig.projbuild \
	/Users/emily/Desktop/ec444/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/Users/emily/Desktop/ec444/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
