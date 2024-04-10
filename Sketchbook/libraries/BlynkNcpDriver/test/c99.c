
#include "BlynkRpcClient.h"
#include "BlynkRpcUartFraming.h"

int rpc_uart_available(void) {
  return 0;
}
int rpc_uart_read(void) {
  return -1;
}
size_t rpc_uart_write(uint8_t data) {
  return 0;
}
void rpc_uart_flush(void) {
}
uint32_t rpc_system_millis(void) {
  return 0;
}

int main(int argc, char* argv[]){
  return 0;
}

