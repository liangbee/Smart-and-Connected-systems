# Simple HTTPD Server

This example modifies the Simple HTTPD server from `esp-idf/examples`. If you do not see this example, try updating your `esp-idf` folder from the GitHub repo. The developers recently updated some of their provided examples. This code should fully run as is.

This example consists of HTTPD server demo with demostration of URI handling:

  - URI *\hello* for GET command returns "Hello World!" message
  - URI *\mac* for GET command returns MAC address of the ESP32
  - URI *\ctrl* for PUT commands toggles onboard LED

## Configure

* Configure your router prior
* Configure the project using "make menuconfig" and goto :
    * Example Configuration ->
        1. WIFI SSID: WIFI network to which ***your PC is also connected to***.
        2. WIFI Password: WIFI password
    * Can also define in code if you choose to

## Run

  - `make flash monitor`
  - Note down the IP assigned to your ESP32. The default port is 80.
  - Example tests:
    * Using curl (Our IP was 192.168.1.100) on a separate command line:
        1. `curl 192.168.1.100:80/hello`
          * tests the GET *\hello* handler
        2. `curl 192.168.1.100:80/mac`
          * tests the GET *\mac* handler
        3. `curl -X PUT -d "0" 192.168.1.100:80/ctrl`
          * turns off the LED
        4. `curl -X PUT -d "1" 192.168.1.100:80/ctrl`
          * turns on the LED

## Notes
* [curl](https://curl.haxx.se) for http requests
* The complier complained the `esp-idf` folder was not a git repo, so make sure it is. (`git clone`, etc.)

## References
* See [HTTPD simple server](https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/simple) for more information
* [Espressif HTTPD Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/protocols/http_server.html)
* [Espressif WiFi](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/wifi/esp_wifi.html#application-examples)
