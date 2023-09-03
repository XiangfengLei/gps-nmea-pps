#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/pps.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "ImuCamera.h"
#include "rtkpps.h"

#define LIB_VERISON "V0.0.1"

int fd = -1;
struct pps_kparams pps_kpara;

typedef struct
{
    pthread_t thread_pps;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct timeval ts;
    gps_t gps_time;
    struct timeval sys_time;
    uint8_t thread_start;

} pps_t;

#define TIME_DIFF(s, e) (((int64_t)e.tv_sec - (int64_t)s.tv_sec) * 1000000 + (int64_t)e.tv_usec - (int64_t)s.tv_usec)
#define TIMENS_TO_TIMEMS(s, ns) (s * 1000 + ns / 1000000)

static pps_t *pps = NULL;
static gps_t old_gps_time;
void *pps_set_system_time_thread(void *arg);
int8_t successflag = 0;

int pps_open(void)
{
    int ret;

    printf("rtk pps version: %s\n", LIB_VERISON);
    fd = open("/dev/pps0", O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        printf("<%s %d> open /dev/pps0 failed!!!, need install pps-gpio.ko.\n", __func__, __LINE__);
        return fd;
    }

    ret = ioctl(fd, PPS_GETPARAMS, &pps_kpara);
    if (ret < 0)
    {
        printf("<%s %d> get param failed\n", __func__, __LINE__);
    }

    uint64_t assert_time = TIMENS_TO_TIMEMS(pps_kpara.assert_off_tu.sec, pps_kpara.assert_off_tu.nsec);
    uint64_t clear_time = TIMENS_TO_TIMEMS(pps_kpara.clear_off_tu.sec, pps_kpara.clear_off_tu.nsec);
    printf("<%s %d> version22=%d, mode =%d, offset0=%lu, offset1=%lu\n", __func__, __LINE__, pps_kpara.api_version, pps_kpara.mode, assert_time, clear_time);

    pps = (pps_t *)malloc(sizeof(pps_t));

    memset(pps, 0, sizeof(pps_t));
    pps->thread_start = 1;
    pthread_create(&pps->thread_pps, NULL, pps_set_system_time_thread, pps);
    return fd;
}

void pps_close(void)
{
    if (fd < 0)
    {
        return;
    }
    close(fd);

    pps->thread_start = 0;
    pthread_join(pps->thread_pps, NULL);
    if (pps)
    {
        free(pps);
    }
    fd = -1;
}

int pps_read(struct timeval *ts)
{
    struct pps_fdata fdata;
    struct pollfd fds;
    int nfds;
    int ret = -1;

    if (fd < 0)
    {
        return -1;
    }

    fds.fd = fd;
    fds.events = POLLIN;
    nfds = poll(&fds, 1, -1);
    if (nfds < 0)
    {
        printf("<%s %d> poll err\n", __func__, __LINE__);
        return -1;
    }

    if (fds.revents | POLLIN)
    {
        ret = ioctl(fd, PPS_FETCH, &fdata);
        if (ret < 0)
        {
            printf("<%s %d> get fdata failed\n", __func__, __LINE__);
        }
        else
        {
            uint64_t timeout = TIMENS_TO_TIMEMS(fdata.timeout.sec, fdata.timeout.nsec);
            uint64_t assert_time = TIMENS_TO_TIMEMS(fdata.info.assert_tu.sec, fdata.info.assert_tu.nsec);
            uint64_t clear_time = TIMENS_TO_TIMEMS(fdata.info.clear_tu.sec, fdata.info.clear_tu.nsec);
            printf("<%s %d> timeout=%lu, assert_time=%u %lu,  clear_time=%u %lu\n", __func__, __LINE__,
                   timeout, fdata.info.assert_sequence, assert_time, fdata.info.clear_sequence, clear_time);
            ts->tv_sec = fdata.info.assert_tu.sec;
            ts->tv_usec = fdata.info.assert_tu.nsec / 1000;
            ret = 0;
        }
    }

    return ret;
}

int set_systme_time(utc_t utc, int64_t offset_us)
{
    struct tm _tm;
    struct timeval tv;
    time_t timep;

    memset(&_tm, 0, sizeof(struct tm));
    _tm.tm_sec = utc.utc_second;
    _tm.tm_min = utc.utc_minute;
    _tm.tm_hour = utc.utc_hour;
    _tm.tm_mday = utc.utc_day;
    _tm.tm_mon = utc.utc_month - 1;
    _tm.tm_year = utc.utc_year - 1900;

    tv.tv_sec = mktime(&_tm);
    tv.tv_usec = offset_us;
    if (settimeofday(&tv, (struct timezone *)0) < 0)
    {
        printf("Set system datetime error!\n");
        return -1;
    }
    return 0;
}

struct timeval utc_to_timeval(utc_t utc)
{
    struct tm _tm;
    struct timeval tv;

    //Mon 03 Jul 2023 06:40:25 PM CST
    memset(&_tm, 0, sizeof(struct tm));
    _tm.tm_sec = utc.utc_second;
    _tm.tm_min = utc.utc_minute;
    _tm.tm_hour = utc.utc_hour;
    _tm.tm_mday = utc.utc_day;
    _tm.tm_mon = utc.utc_month - 1;
    _tm.tm_year = utc.utc_year - 1900;
    tv.tv_sec = mktime(&_tm);
    tv.tv_usec = 0;

    return tv;
}

void *pps_set_system_time_thread(void *arg)
{
    int64_t time_diff = 0;

    //	rtk_data_t pps_mcu;
    //	int rtk_count = 0;

    while (pps->thread_start)
    {
        if (pps_read(&pps->ts) == 0)
        {
            pthread_mutex_lock(&pps->mutex);
            pthread_cond_wait(&pps->cond, &pps->mutex);
            pthread_mutex_unlock(&pps->mutex);
#if 0	
				//理论上应该等待mcu串口数据，但获得报文数据时肯定已经获得mcu数据了，如果没有获得mcu数据，那么判断为接收失败	
				pps_mcu = mcu_uart_get_rtkdata();
				if(pps_mcu.TimeStamp.sec != 0 || pps_mcu.TimeStamp.usec != 0)
				{
					struct timeval tmp;

					tmp.tv_sec = pps_mcu.TimeStamp.sec;
					tmp.tv_usec = pps_mcu.TimeStamp.usec;
					rtk_count = 0;

					time_diff = abs(TIME_DIFF(tmp, pps->ts));
					//pps mcu打印的时间戳和soc打印的时间戳进行对比，理论上应该只差4ms左右
					if(time_diff < 10*1000)
					{
						pps->ts.tv_sec = pps_mcu.TimeStamp.sec;
						pps->ts.tv_usec = pps_mcu.TimeStamp.usec;
					}else{
						printf("<%s %d> mcu pps timestamp is error time_diff=%ld!!!\n",  __func__, __LINE__, time_diff);
					}
				}else{
					printf("WARNNING:<%s %d>  it is failed that get pps time from mcu!!!\n",  __func__, __LINE__);
					rtk_count++;
				}

				//mcu可能会偶发没有收到数据，收不到数据就不进行校准, 如果3次之后还收不到，那么再进行SOC的不精确校准
				if(rtk_count < 3)
				{
					continue;
				}
#endif
            time_diff = TIME_DIFF(pps->ts, pps->gps_time.stamp);
            //检查脉冲信号的时间戳和收到报文的时间，匹配脉冲和报文
            if (time_diff < 0 && time_diff > 200 * 1000)
            {
                printf("<%s %d> pps set systime failed time_diff=%ld!!!\n", __func__, __LINE__, time_diff);
                continue;
            }

            struct timeval utcval = utc_to_timeval(pps->gps_time.utc_time);
            printf("<%s %d> utc sec=%lu, usec=%lu\n", __func__, __LINE__, utcval.tv_sec, utcval.tv_usec);

            gettimeofday(&pps->sys_time, NULL);
            time_diff = TIME_DIFF(utcval, pps->sys_time) - TIME_DIFF(pps->ts, pps->sys_time);
            //检查gps时间和当前系统时间的差值
            if (abs(time_diff) < 1000)
            {
                printf("<%s %d> gps and sytem time diff=%ld!!!\n", __func__, __LINE__, time_diff);
                //	continue;
            }
            else
            {
                successflag = 0;
            }
            //脉冲信号时间和当前系统时间的差值
            time_diff = TIME_DIFF(pps->ts, pps->sys_time);
            if (set_systme_time(pps->gps_time.utc_time, time_diff) < 0)
            {
                printf("<%s %d> pps set systime failed!!!\n", __func__, __LINE__);
            }
            else
            {
                printf("<%s %d> pps set systime successful!!!\n", __func__, __LINE__);
                pthread_mutex_lock(&pps->mutex);
                successflag = 1;
                pthread_mutex_unlock(&pps->mutex);
#if 1
                if (ImuCameraUpdateRealTime() == 0)
                {
                    pthread_mutex_lock(&pps->mutex);
                    successflag = 1;
                    pthread_mutex_unlock(&pps->mutex);
                }
#endif
                continue;
            }
        }
    }

    return NULL;
}

int8_t pps_push_gps_time(gps_t t)
{
    if (fd < 0)
    {
        return -1;
    }

    if (t.utc_time.utc_msecond != 0)
    {
        return -1;
    }

    printf("<%s %d> push gps time...\n", __func__, __LINE__);

    pthread_mutex_lock(&pps->mutex);
    pps->gps_time = t;
    pthread_cond_signal(&pps->cond);
    pthread_mutex_unlock(&pps->mutex);

    return 0;
}

int8_t pps_get_successfully_flag(void)
{
    int8_t ret;

    pthread_mutex_lock(&pps->mutex);
    ret = successflag;
    pthread_mutex_unlock(&pps->mutex);

    return ret;
}

/*
int main(int argc, char**argv)
{
	pps_open(argv[1]);

	struct timeval tv;
	struct timeval ts;

	while(1)
	{
		usleep(10000);	

		
		//printf("<%s %d>.... %ld\n", __func__, __LINE__, TIME_DIFF(ts, tv));
	
	}

	pps_close();
	return 0;
}
*/
