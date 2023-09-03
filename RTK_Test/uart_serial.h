#ifndef UART_SERIAL_H
#define UART_SERIAL_H

#include <string>

struct SerialParams
{
    int baud_rate = 9600;
    int bits = 8;
    char event = 'N';
    int stop = 1;
    SerialParams(const int &baud_rate,
                 const int &bits,
                 const char &event,
                 const int &stop) : baud_rate(baud_rate),
                                    bits(bits),
                                    event(event),
                                    stop(stop) {}
};

bool OpenUart(int &fd, const std::string &dev_path);
bool SetUart(int fd, const SerialParams &params);
void CloseUart(int &fd);
int UartSend(const int &fd, const char *data, const int &len);
int UartRecv(const int &fd, char *data, const int &len, const int &timeout_ms);
size_t ReadLine(const int &fd, std::string &buffer, size_t size, const std::string &eol);
std::string ReadLine(const int &fd, uint32_t size = 65535, const std::string &eol = "\n");

#endif