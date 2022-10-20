#include "mbed-trace/mbed_trace.h"
#include "mbed.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "wifi_helper.h"
#include <cstdio>

class SocketDemo {
  static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
  static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

  static constexpr size_t REMOTE_PORT = 5555;
  static constexpr int SECONDS = 1000000;

public:
  SocketDemo() : _net(NetworkInterface::get_default_instance()) {}

  ~SocketDemo() {
    if (_net) {
      _net->disconnect();
    }
  }

  void run() {
    if (!_net) {
      printf("Error! No network interface found.\r\n");
      return;
    }

    printf("Connecting to the network...\r\n");

    wifi_connect();
    print_network_info();
    socket_open();
    address_initialize();

    printf("Opening connection to remote port %d\r\n", REMOTE_PORT);

    socket_connect();
    sensorLoop();

    printf("Demo concluded successfully \r\n");
  }

private:
  bool resolve_hostname() {
    const char hostname[] = MBED_CONF_APP_HOSTNAME;

    /* get the host address */
    printf("\nResolve hostname %s\r\n", hostname);
    nsapi_size_or_error_t result = _net->gethostbyname(hostname, _address);
    if (result != 0) {
      printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
      return false;
    }

    printf("%s address is %s\r\n", hostname,
           (_address->get_ip_address() ? _address->get_ip_address() : "None"));

    return true;
  }

  void wifi_scan() {
    WiFiInterface *wifi = _net->wifiInterface();

    WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

    /* scan call returns number of access points found */
    int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

    if (result <= 0) {
      printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
      return;
    }

    printf("%d networks available:\r\n", result);

    for (int i = 0; i < result; i++) {
      printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx "
             "RSSI: %hhd Ch: %hhd\r\n",
             ap[i].get_ssid(), get_security_string(ap[i].get_security()),
             ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
             ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
             ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("\r\n");
  }

  void print_network_info() {
    /* print the network info */
    SocketAddress a;
    _net->get_ip_address(&a);
    printf("IP address: %s\r\n",
           a.get_ip_address() ? a.get_ip_address() : "None");
    _net->get_netmask(&a);
    printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    _net->get_gateway(&a);
    printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
  }

  void wifi_connect() {
    nsapi_size_or_error_t result;
    result = _net->connect();
    while (result != 0) {
      printf("Error! _net->connect() returned: %d\r\n", result);
      wait_us(SECONDS);
      result = _net->connect();
    }
  }

  void address_initialize() {
    while (!resolve_hostname()) {
      wait_us(SECONDS);
    }

    _address->set_port(REMOTE_PORT);
  }

  void socket_open() {
    nsapi_size_or_error_t result;
    result = _socket->open(_net);
    while (result != 0) {
      printf("Error! _socket.open() returned: %d\r\n", result);
      socket_restart();
      wait_us(5 * SECONDS);
      result = _socket->open(_net);
    }
  }

  void socket_connect() {
    nsapi_size_or_error_t result;
    result = _socket->connect(*_address);
    while (result != 0) {
      printf("Error! _socket.connect() returned: %d\r\n", result);
      socket_open();
      wait_us(5 * SECONDS);
      result = _socket->connect(*_address);
    }
  }

  void socket_send(char *json, int len) {
    nsapi_size_or_error_t result;
    result = _socket->send(json, len);
    while (0 >= result) {
      printf("Error seding: %d\n", result);
      socket_connect();
      wait_us(5 * SECONDS);
      result = _socket->send(json, len);
    }
  }

  void socket_restart() {
    _socket->close();
    _socket = new TCPSocket();
  }

  void sensorLoop() {
    int sample_num = 0;
    int16_t pDataXYZ[3] = {0};
    float pGyroDataXYZ[3] = {0};
    BSP_GYRO_Init();
    BSP_ACCELERO_Init();
    char *json = new char[1024];
    int SCALE_MULTIPLIER = 1;

    while (1) {
      ++sample_num;
      BSP_ACCELERO_AccGetXYZ(pDataXYZ);
      BSP_GYRO_GetXYZ(pGyroDataXYZ);
      float x1 = pDataXYZ[0] * SCALE_MULTIPLIER,
            y1 = pDataXYZ[1] * SCALE_MULTIPLIER,
            z1 = pDataXYZ[2] * SCALE_MULTIPLIER,
            x2 = pGyroDataXYZ[0] * SCALE_MULTIPLIER,
            y2 = pGyroDataXYZ[1] * SCALE_MULTIPLIER,
            z2 = pGyroDataXYZ[2] * SCALE_MULTIPLIER;
      int len = sprintf(json,
                        "{\"acc_x\":%f,\"acc_y\":%f,\"acc_z\":%f,\"gyro_x\":%f,"
                        "\"gyro_y\":%f,\"gyro_z\":%f,\"s\":%d}",
                        (float)((int)(x1 * 10000)) / 10000,
                        (float)((int)(y1 * 10000)) / 10000,
                        (float)((int)(z1 * 10000)) / 10000,
                        (float)((int)(x2 * 10000)) / 10000,
                        (float)((int)(y2 * 10000)) / 10000,
                        (float)((int)(z2 * 10000)) / 10000, sample_num);

      socket_send(json, len);
      wait_us(SECONDS / 10);
    }
  }

private:
  NetworkInterface *_net;
  TCPSocket *_socket = new TCPSocket();
  SocketAddress *_address = new SocketAddress();
};

int main() {
  printf("\r\nStarting socket demo\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
  mbed_trace_init();
#endif

  SocketDemo *example = new SocketDemo();
  MBED_ASSERT(example);
  example->run();

  return 0;
}
