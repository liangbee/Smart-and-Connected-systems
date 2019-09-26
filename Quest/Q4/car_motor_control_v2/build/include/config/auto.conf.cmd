deps_config := \
	/Users/linbma/esp/esp-idf/components/app_trace/Kconfig \
	/Users/linbma/esp/esp-idf/components/aws_iot/Kconfig \
	/Users/linbma/esp/esp-idf/components/bt/Kconfig \
	/Users/linbma/esp/esp-idf/components/driver/Kconfig \
	/Users/linbma/esp/esp-idf/components/esp32/Kconfig \
	/Users/linbma/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/Users/linbma/esp/esp-idf/components/esp_http_client/Kconfig \
	/Users/linbma/esp/esp-idf/components/ethernet/Kconfig \
	/Users/linbma/esp/esp-idf/components/fatfs/Kconfig \
	/Users/linbma/esp/esp-idf/components/freertos/Kconfig \
	/Users/linbma/esp/esp-idf/components/heap/Kconfig \
	/Users/linbma/esp/esp-idf/components/http_server/Kconfig \
	/Users/linbma/esp/esp-idf/components/libsodium/Kconfig \
	/Users/linbma/esp/esp-idf/components/log/Kconfig \
	/Users/linbma/esp/esp-idf/components/lwip/Kconfig \
	/Users/linbma/esp/esp-idf/components/mbedtls/Kconfig \
	/Users/linbma/esp/esp-idf/components/mdns/Kconfig \
	/Users/linbma/esp/esp-idf/components/openssl/Kconfig \
	/Users/linbma/esp/esp-idf/components/pthread/Kconfig \
	/Users/linbma/esp/esp-idf/components/spi_flash/Kconfig \
	/Users/linbma/esp/esp-idf/components/spiffs/Kconfig \
	/Users/linbma/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/Users/linbma/esp/esp-idf/components/vfs/Kconfig \
	/Users/linbma/esp/esp-idf/components/wear_levelling/Kconfig \
	/Users/linbma/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/Users/linbma/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/Users/linbma/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/Users/linbma/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
