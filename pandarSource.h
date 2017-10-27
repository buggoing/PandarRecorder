#ifndef __PandarSource_h
#define __PandarSource_h

#include <list>
#include <queue>
#include <pcap.h>
#include <cstring>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include<cstdint>

#ifdef _MSC_VER
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") //Winsock Library
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

#ifdef _MSC_VER
#define BILLION (1E9)
#define CLOCK_REALTIME 0
#define THREAD_T_INIT(t){t.p = NULL;}
#define THREAD_IS_VALID(t) t.p
int gettimeofday(struct timeval *tp, void *);
void initilizeTimer();

#else
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define THREAD_T_INIT(t){t = 0;}
#define THREAD_IS_VALID(t) t
typedef unsigned int SOCKET;
#endif


// Start of Packet & Packet Angle size , SOF 2bytes , Angle 2 bytes
#define HS_LIDAR_L40_DUAL_VERSION_SOP_ANGLE_SIZE (4)
// Unit size = distance(2bytes) + reflectivity(2bytes) for each Line
#define HS_LIDAR_L40_DUAL_VERSION_SERIAL_UNIT_SIZE (4)
// Unit num = Line 40
#define HS_LIDAR_L40_DUAL_VERSION_UNIT_NUM (40)
// Every udp packet has 8 blocks
#define HS_LIDAR_L40_DUAL_VERSION_BLOCK_NUM (8)
// Block size = unit num * unit size + SOP + Angle
#define HS_LIDAR_L40_DUAL_VERSION_SERIAL_BLOCK_SIZE (HS_LIDAR_L40_DUAL_VERSION_SERIAL_UNIT_SIZE * HS_LIDAR_L40_DUAL_VERSION_UNIT_NUM + HS_LIDAR_L40_DUAL_VERSION_SOP_ANGLE_SIZE)
// Block tail = timestamp ( 4 bytes ) + factory num (2 bytes)
#define HS_LIDAR_L40_DUAL_VERSION_TIMESTAMP_SIZE (4)
#define HS_LIDAR_L40_DUAL_VERSION_ECHO_SIZE (1)
#define HS_LIDAR_L40_DUAL_VERSION_FACTORY_SIZE (1)
#define HS_LIDAR_L40_DUAL_VERSION_RESERVED_SIZE (8)
#define HS_LIDAR_L40_DUAL_VERSION_ENGINE_VELOCITY (2)
#define HS_LIDAR_L40_DUAL_VERSION_TAIL_SIZE (HS_LIDAR_L40_DUAL_VERSION_TIMESTAMP_SIZE + HS_LIDAR_L40_DUAL_VERSION_ECHO_SIZE + HS_LIDAR_L40_DUAL_VERSION_FACTORY_SIZE + HS_LIDAR_L40_DUAL_VERSION_RESERVED_SIZE + HS_LIDAR_L40_DUAL_VERSION_ENGINE_VELOCITY)
// total packet size
#define HS_LIDAR_L40_DUAL_VERSION_PACKET_SIZE (HS_LIDAR_L40_DUAL_VERSION_SERIAL_BLOCK_SIZE * HS_LIDAR_L40_DUAL_VERSION_BLOCK_NUM + HS_LIDAR_L40_DUAL_VERSION_TAIL_SIZE)

// Start of Packet & Packet Angle size , SOF 2bytes , Angle 2 bytes
#define HS_LIDAR_L40_SOP_ANGLE_SIZE (4)
// Unit size = distance(2bytes) + reflectivity(2bytes) for each Line
#define HS_LIDAR_L40_SERIAL_UNIT_SIZE (5)
// Unit num = Line 40
#define HS_LIDAR_L40_UNIT_NUM (40)
// Every udp packet has 8 blocks
#define HS_LIDAR_L40_BLOCK_NUM (6)
// Block size = unit num * unit size + SOP + Angle
#define HS_LIDAR_L40_SERIAL_BLOCK_SIZE (HS_LIDAR_L40_SERIAL_UNIT_SIZE * HS_LIDAR_L40_UNIT_NUM + HS_LIDAR_L40_SOP_ANGLE_SIZE)
// Block tail = timestamp ( 4 bytes ) + factory num (2 bytes)
#define HS_LIDAR_L40_TIMESTAMP_SIZE (4)
#define HS_LIDAR_L40_FACTORY_SIZE (2)
#define HS_LIDAR_L40_RESERVED_SIZE (8)
#define HS_LIDAR_L40_ENGINE_VELOCITY (2)
#define HS_LIDAR_L40_TAIL_SIZE (HS_LIDAR_L40_TIMESTAMP_SIZE + HS_LIDAR_L40_FACTORY_SIZE + HS_LIDAR_L40_RESERVED_SIZE + HS_LIDAR_L40_ENGINE_VELOCITY)
// total packet size
#define HS_LIDAR_L40_PACKET_SIZE (HS_LIDAR_L40_SERIAL_BLOCK_SIZE * HS_LIDAR_L40_BLOCK_NUM + HS_LIDAR_L40_TAIL_SIZE)

//real packets num is around 300
#define HS_LIDAR_L40_MAX_PACKETS_NUM (1000)

#define HS_LIDAR_L40_GPS_PACKET_SIZE (512)
#define HS_LIDAR_L40_GPS_PACKET_FLAG_SIZE (2)
#define HS_LIDAR_L40_GPS_PACKET_YEAR_SIZE (2)
#define HS_LIDAR_L40_GPS_PACKET_MONTH_SIZE (2)
#define HS_LIDAR_L40_GPS_PACKET_DAY_SIZE (2)
#define HS_LIDAR_L40_GPS_PACKET_HOUR_SIZE (2)
#define HS_LIDAR_L40_GPS_PACKET_MINUTE_SIZE (2)
#define HS_LIDAR_L40_GPS_PACKET_SECOND_SIZE (2)
#define HS_LIDAR_L40_GPS_ITEM_NUM (7)

#define DEFAULT_UDP_LISTEN_PORT 8080
#define DEFAULT_GPS_LISTEN_PORT 8308
#define PACKET_SIZE 1282
#define MAX_UDP_PORT 65536

class UdpServer;

class UDPPacket
{
  public:
    UDPPacket(unsigned char data[], int size)
    {
        memcpy(buff, data, size);
        len = size;
    }
    UDPPacket()
    {
        len = 0;
    }
    ~UDPPacket()
    {
        len = 0;
    }

    unsigned char buff[1500];
    int len;
};

class PandarSource
{
  public:
    PandarSource();
    virtual ~PandarSource();
    //the following two funcs equal to producer and consumer
    void PushToList(unsigned char *packet, int len);
    void GetFromList();
    void start(unsigned int, unsigned int);
    void stop();
    void StartRecording(const std::string &filename);
    void StopRecording();
    bool getIsRecording();
    void WritePacketToFile();
    bool WriteOnePacket(const unsigned char *data, unsigned int dataLength);
    unsigned int getAndUpdatePacketCounter();
    uint64_t getSavedPacketCounter();

  protected:
    std::string OutputFilename;
    pcap_t *pcapFile;
    pcap_dumper_t *pcapDump;

  private:
    UdpServer *server, *gps_server;
    std::list<UDPPacket> packetList;
    std::list<UDPPacket> packetsForWriter;
    sem_t semaphoreForPushPacket, semaphoreForWriteFile;

    pthread_mutex_t mutexPushPacket, mutexWritePacket, mutexReceivedPacketsCounter, mutexSavedPacketsCounter;
    pthread_t processPacket;
    pthread_t receiverProcess;
    pthread_t writerProcess;

    int ProcessPacketExit;
    unsigned int ratePacketCounter;
    uint64_t savedPacketsCounter;
    bool isRecording;
};

static void* _Thread(void *arg);

#endif