#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include "uart_serial.h"

bool OpenUart(int &fd, const std::string &dev_path)
{
    const char *tmp_path = dev_path.c_str();

    fd = open(tmp_path, O_RDWR | O_NONBLOCK | O_NDELAY | O_NOCTTY);
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return false;
    }
    else
    {
        printf("open %s success!\n", tmp_path);
    }

    if (isatty(STDIN_FILENO) == 0)
    {
        printf("standard input is not a terminal device\n");
    }
    else
    {
        printf("isatty success!\n");
    }

    return true;
}

void CloseUart(int &fd)
{
    close(fd);
    fd = -1;
}

int UartSend(const int &fd, const char *data, const int &len)
{
    return write(fd, data, len);
}

int UartRecv(const int &fd, char *data, const int &len, const int &timeout_ms)
{
    int ret = -1;
    fd_set rfds;
    struct timeval tv;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = timeout_ms * 1000; /* ms */

    ret = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (ret > 0)
    {
        if (FD_ISSET(fd, &rfds) > 0)
        {
            return read(fd, data, len);
        }
    }

    return 0;
}

bool SetUart(int fd, const SerialParams &params)
{
    int speed = params.baud_rate;
    int bits = params.bits;
    char event = params.event;
    int stop = params.stop;

    struct termios newtio, oldtio;
    if (tcgetattr(fd, &oldtio) != 0)
    {
        printf("tcgetattr fail. %d\n", tcgetattr(fd, &oldtio));
        return false;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;
    switch (bits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }
    switch (event)
    {
    case 'o':
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'e':
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'n':
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    default:
        break;
    }

    switch (speed)
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }

    if (stop == 1)
        newtio.c_cflag &= ~CSTOPB;
    else if (stop == 2)
        newtio.c_cflag |= CSTOPB;

    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);

    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        printf("tcsetattr set error\n");
        return false;
    }

    printf("set uart success!\n");

    return true;
}

size_t ReadLine(const int &fd, std::string &buffer, size_t size, const std::string &eol)
{
    size_t eol_len = eol.length();
    char *buffer_ = static_cast<char *>(alloca(size * sizeof(uint8_t)));
    size_t read_so_far = 0;
    while (true)
    {
        size_t bytes_read = UartRecv(fd, buffer_ + read_so_far, 1, 10);
        read_so_far += bytes_read;
        if (bytes_read == 0)
        {
            break; // Timeout occured on reading 1 byte
        }
        if (std::string(reinterpret_cast<const char *>(buffer_ + read_so_far - eol_len), eol_len) == eol)
        {
            break; // EOL found
        }
        if (read_so_far == size)
        {
            break; // Reached the maximum read length
        }
    }
    buffer.append(reinterpret_cast<const char *>(buffer_), read_so_far);
    return read_so_far;
}

std::string ReadLine(const int &fd, uint32_t size, const std::string &eol)
{
    std::string buffer;
    ReadLine(fd, buffer, size, eol);
    return buffer;
}