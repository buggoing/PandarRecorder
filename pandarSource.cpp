#include <string>
#include "pandarSource.h"
using namespace std;

#ifdef _MSC_VER

int gettimeofday(struct timeval *tp, void *)
{
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);
    long long t = (static_cast<long long>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    t -= 116444736000000000LL;
    t /= 10; // microseconds
    tp->tv_sec = static_cast<long>(t / 1000000UL);
    tp->tv_usec = static_cast<long>(t % 1000000UL);
    return 0;
}

static BOOL g_first_time = 1;
static LARGE_INTEGER g_counts_per_sec;
int clock_gettime(int dummy, struct timespec *ct)
{
    LARGE_INTEGER count;

    if (g_first_time)
    {
        g_first_time = 0;

        if (0 == QueryPerformanceFrequency(&g_counts_per_sec))
        {
            g_counts_per_sec.QuadPart = 0;
        }
    }

    if ((NULL == ct) || (g_counts_per_sec.QuadPart <= 0) ||
        (0 == QueryPerformanceCounter(&count)))
    {
        return -1;
    }
    ct->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
    ct->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * BILLION) / g_counts_per_sec.QuadPart;
    return 0;
}
#endif

const unsigned short LidarPacketHeader_DUAL[21] = {
  0xffff, 0xffff, 0xffff, 0x0a00, 0x0135, 0xc0fe, 0x0008, 0x0045, 0x4c05, 0xbb04, 0x0040, 0x1180, 0x9d1b, 0xa8c0, 0xa114, 0xffff, 0xffff, 0x901f, 0x901f, 0x3805, 0x0000};

const unsigned short LidarPacketHeader[21] = {
  0xffff, 0xffff, 0xffff, 0x0a00, 0x0135, 0xc0fe, 0x0008, 0x0045, 0xf404, 0xa601, 0x0040, 0x1180, 0xa933, 0xa8c0, 0xa114, 0xffff, 0xffff, 0x901f, 0x901f, 0xe004, 0x0000};
//--------------------------------------------------------------------------------
const unsigned short PositionPacketHeader[21] = {
  0xffff, 0xffff, 0xffff, 0x0a00,
  0x0135, 0xc0fe, 0x0008, 0x0045,
  0x1c02, 0x0300, 0x0040, 0x1180,
  0x2438, 0xa8c0, 0x0200, 0xffff, // checksum 0xa9b4 //source ip 0xa8c0, 0xc801 is 192.168.1.200
  0xffff, 0x7420, 0x7420, 0x0802, 0x0000};

  
class UdpServer
{
public:
  UdpServer(int port, PandarSource *source)
      : m_sock(-1),
        guest(source),
        Port(port),
        shouldClose(false)
  {
	   THREAD_T_INIT(receiverProcess);
#ifdef _MSC_VER
    WSADATA wsaData;
    WORD version = MAKEWORD(2, 2);
    int ret = WSAStartup(version, &wsaData); //win sock start up
    if (ret)
    {
      std::cerr << "Initilize winsock error !" << std::endl;
      return;
    }
#endif
    if ((m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
      return;
    }
    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));
    //Prepare the sockaddr_in structure
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

#ifdef _MSC_VER
    bool bReuseaddr = TRUE;
    setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&bReuseaddr, sizeof(bReuseaddr));
    int optVal = 200 * 1024; // 200k socket buffer
    setsockopt(m_sock, SOL_SOCKET, SO_RCVBUF, (char *)&optVal, sizeof(optVal));
#endif

    if (bind(m_sock, (sockaddr *)&addr, sizeof(sockaddr)) == SOCKET_ERROR)
    {
#ifdef _MSC_VER
      printf("Bind failed with error code %d\n", WSAGetLastError());
      closesocket(m_sock);
#else
      close(m_sock);
#endif
      return;
    }
    start_receive();
  }
  ~UdpServer()
  {
    cancel_receiver();
  }

  void Thread()
  {
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 20 * 1000; // 20ms
    int size = 0;
    fd_set rfd;
    shouldClose = 0;
    while (!shouldClose)
    {
      size = 0;
      FD_ZERO(&rfd);

      FD_SET(m_sock, &rfd);
      size = select(m_sock + 1, &rfd, NULL, NULL, &timeout);
      if (size == SOCKET_ERROR)
      {
        //error
        printf("select()/n");
        break;
      }
      else if (size == 0)
      {
        // timeout
        continue;
      }
      else
      {
        if (FD_ISSET(m_sock, &rfd))
        {
          int nRecEcho = recv(m_sock, (char *)this->buffer, 1500, 0);
          if (nRecEcho < 0)
          {
            printf("recv error/n");
            break;
          }
          guest->PushToList(this->buffer, nRecEcho);
        }
      }
    }
  }

  void start_receive()
  {
    if (THREAD_IS_VALID(receiverProcess))
    {
      this->cancel_receiver();
    }
    pthread_create(&receiverProcess, NULL, &_Thread, this);
  }

  void cancel_receiver()
  {
    if (THREAD_IS_VALID(receiverProcess))
    {
      shouldClose = 1;
      pthread_join(receiverProcess, NULL);
      pthread_detach(receiverProcess);
    }

    if (m_sock > 0)
    {

#ifdef _MSC_VER
      closesocket(m_sock);
      WSACleanup();
#else
      close(m_sock);
#endif
      m_sock = -1;
    }
  }

private:
  PandarSource *guest;
  int Port;
  SOCKET m_sock;
  unsigned char buffer[1500];
  bool shouldClose;
  pthread_t receiverProcess;
};

static void* _WritePacketToFile(void *arg)
{
  PandarSource *pandarSource = (PandarSource *)arg;
  pandarSource->WritePacketToFile();
  return NULL;
}

static void* _Thread(void *arg)
{
  UdpServer *server = (UdpServer *)arg;
  server->Thread();
  return NULL;
}

static void* _GetFromList(void *arg)
{
  PandarSource *pandarSource = (PandarSource *)arg;
  pandarSource->GetFromList();
  return NULL;
}

void PandarSource::stop()
{
  if (this->server)
  {
    delete server;
    this->server = 0;
  }
  if (this->gps_server)
  {
    delete gps_server;
    this->gps_server = 0;
  }

  if (THREAD_IS_VALID(this->processPacket))
  {
    this->ProcessPacketExit = 1;
    pthread_join(processPacket, NULL);
    pthread_detach(processPacket);
  }

  THREAD_T_INIT(this->processPacket);
  this->server = 0;
  this->gps_server = 0;
}

void PandarSource::start(unsigned int port, unsigned int gpsport)
{
  stop();
  pthread_create(&processPacket, NULL, &_GetFromList, this);
  this->server = new UdpServer(port, this);
  if (port != gpsport)
    this->gps_server = new UdpServer(gpsport, this);
}

void PandarSource::PushToList(unsigned char packet[], int len)
{
  pthread_mutex_lock(&this->mutexReceivedPacketsCounter);
  this->ratePacketCounter++;
  pthread_mutex_unlock(&this->mutexReceivedPacketsCounter);

  UDPPacket udpPacket(packet, len);
  pthread_mutex_lock(&this->mutexPushPacket);
  this->packetList.push_back(udpPacket);
  pthread_mutex_unlock(&this->mutexPushPacket);
  sem_post(&this->semaphoreForPushPacket);
}

unsigned int PandarSource::getAndUpdatePacketCounter()
{
  unsigned int counter;
  pthread_mutex_lock(&this->mutexReceivedPacketsCounter);
  counter = this->ratePacketCounter;
  this->ratePacketCounter = 0;
  pthread_mutex_unlock(&this->mutexReceivedPacketsCounter);
  return counter;
}

uint64_t PandarSource::getSavedPacketCounter()
{
  uint64_t counter;
  counter = this->savedPacketsCounter;
  return counter;
}

void PandarSource::GetFromList()
{
  ProcessPacketExit = 0;
  while (!ProcessPacketExit)
  {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
      cout << "get time error" << endl;
    }

    ts.tv_nsec += 4 * 1000 * 1000;
    if (sem_timedwait(&this->semaphoreForPushPacket, &ts) == -1)
    {
      continue;
    }

    UDPPacket buffer;
    pthread_mutex_lock(&this->mutexPushPacket);
    buffer = this->packetList.front();
    this->packetList.pop_front();
    pthread_mutex_unlock(&this->mutexPushPacket);

    if (isRecording)
    {
      pthread_mutex_lock(&this->mutexWritePacket);
      this->packetsForWriter.push_back(buffer);
      pthread_mutex_unlock(&this->mutexWritePacket);

      sem_post(&this->semaphoreForWriteFile);
    }
  }
}

void PandarSource::StartRecording(const std::string &filename)
{
  if (this->OutputFilename != filename)
  {
    if (this->pcapFile)
    {
      pcap_dump_close(this->pcapDump);
      pcap_close(this->pcapFile);
      this->pcapFile = 0;
      this->pcapDump = 0;
      this->OutputFilename.clear();
    }
  }
  if (this->pcapFile == 0)
  {
    this->pcapFile = pcap_open_dead(DLT_EN10MB, 65535);
    this->pcapDump = pcap_dump_open(this->pcapFile, filename.c_str());

    if (!this->pcapDump)
    {
      pcap_geterr(this->pcapFile);
      pcap_close(this->pcapFile);
      this->pcapFile = 0;
      std::cout << "Failed to open packet file: " << filename << std::endl;
      return;
    }

    this->OutputFilename = filename;
  }

  this->packetsForWriter.clear();
  this->isRecording = true;

  sem_init(&this->semaphoreForWriteFile, 0, 0);
  pthread_create(&writerProcess, NULL, &_WritePacketToFile, this);
}

void PandarSource::StopRecording()
{
  this->isRecording = false;
  if (THREAD_IS_VALID(writerProcess))
  {
    pthread_join(writerProcess, NULL);
    pthread_detach(writerProcess);
  }

  THREAD_T_INIT(this->writerProcess);
  this->packetsForWriter.clear();
  pthread_mutex_lock(&this->mutexSavedPacketsCounter);
  this->savedPacketsCounter = 0;
  pthread_mutex_unlock(&this->mutexSavedPacketsCounter);
  sem_init(&this->semaphoreForWriteFile, 0, 0);
}

void PandarSource::WritePacketToFile()
{
  while (this->isRecording)
  {
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
      std::cout << "get time error" << std::endl;
    }

    ts.tv_nsec += 4 * 1000 * 1000; //4 millseconds
    if (sem_timedwait(&this->semaphoreForWriteFile, &ts) == -1)
    {
      continue;
    }
    pthread_mutex_lock(&this->mutexWritePacket);
    UDPPacket packet = this->packetsForWriter.front();
    this->packetsForWriter.pop_front();
    pthread_mutex_unlock(&this->mutexWritePacket);
    this->WriteOnePacket(packet.buff, packet.len);
  }
}

bool PandarSource::WriteOnePacket(const unsigned char *data, unsigned int dataLength)
{
  if (!this->pcapFile)
  {
    return false;
  }

  const unsigned short *headerData;
  struct pcap_pkthdr header;

  std::vector<unsigned char> packetBuffer;
  if (dataLength == HS_LIDAR_L40_PACKET_SIZE)
  {
    headerData = LidarPacketHeader;
    header.caplen = dataLength + 42;
    header.len = dataLength + 42;
    packetBuffer.resize(dataLength + 42);
  }
  else if (dataLength == HS_LIDAR_L40_DUAL_VERSION_PACKET_SIZE)
  {
    headerData = LidarPacketHeader_DUAL;
    header.caplen = dataLength + 42;
    header.len = dataLength + 42;
    packetBuffer.resize(dataLength + 42);
  }
  else if (dataLength == HS_LIDAR_L40_GPS_PACKET_SIZE)
  {
    headerData = PositionPacketHeader;
    header.caplen = 554;
    header.len = 554;
    packetBuffer.resize(554);
  }
  else
  {
    return false;
  }

  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  header.ts = currentTime;
  memcpy(&(packetBuffer[0]), headerData, 42);
  memcpy(&(packetBuffer[0]) + 42, data, dataLength);
  pcap_dump((u_char *)this->pcapDump, &header, &(packetBuffer[0]));
  pthread_mutex_lock(&this->mutexSavedPacketsCounter);
  this->savedPacketsCounter++;
  pthread_mutex_unlock(&this->mutexSavedPacketsCounter);
  return true;
}

PandarSource::PandarSource()
{
  sem_init(&this->semaphoreForPushPacket, 0, 0);
  sem_init(&this->semaphoreForWriteFile, 0, 0);
  THREAD_T_INIT(this->processPacket);
  THREAD_T_INIT(this->writerProcess);  
  THREAD_T_INIT(this->receiverProcess);
  pthread_mutex_init(&this->mutexPushPacket, NULL);
  pthread_mutex_init(&this->mutexWritePacket, NULL);
  pthread_mutex_init(&this->mutexReceivedPacketsCounter, NULL);
  pthread_mutex_init(&this->mutexSavedPacketsCounter, NULL);
  this->server = 0;
  this->gps_server = 0;
  this->ProcessPacketExit = 0;
  this->isRecording = false;
  this->pcapFile = 0;
  this->pcapDump = 0;
  this->ratePacketCounter = 0;
  this->savedPacketsCounter = 0;

}
PandarSource::~PandarSource()
{
  this->stop();
}