#include "easywsclient.hpp"
#include "OpenProtocol.h"

//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include <endian.h>
#include <unistd.h>
#include <thread>

#define _DEBUG  1

using easywsclient::WebSocket;
static WebSocket::pointer ws = NULL;

void injectGPS(double longitude, double latitude) {
    OpenProtocol::VirtualDevice virtualDevice;
    virtualDevice.type = htobe32(uint32_t(OpenProtocol::HardwareType::Sensor));
    virtualDevice.subType = htobe32(uint32_t(OpenProtocol::SensorType::GPS));
    virtualDevice.interfaces = htobe32(uint32_t(OpenProtocol::GPSFunc::InjectLL));

    int64_t longitude64 = htobe64(*((uint64_t*)&longitude));
    int64_t latitude64 = htobe64(*((uint64_t*)&latitude));

    int size = sizeof(virtualDevice)+sizeof(longitude64)+sizeof(latitude64);
    uint8_t *buffer = (uint8_t *)malloc(size);
    memcpy(buffer,&virtualDevice,sizeof(virtualDevice));
    memcpy(buffer+sizeof(virtualDevice),&longitude64,sizeof(longitude64));
    memcpy(buffer+sizeof(virtualDevice)+sizeof(longitude64),&latitude64,sizeof(latitude64));
    std::vector<uint8_t> vectorData(buffer,buffer+size);

#if _DEBUG
    printf("send:");
    for(int i=0;i<size;i++) {
        printf("%02X ",buffer[i]);
    }
    printf("\r\n");
#endif
    ws->sendBinary(vectorData);
}

void injectAudio(uint8_t *pcm, uint32_t audio_size) {
    OpenProtocol::VirtualDevice virtualDevice;
    virtualDevice.type = htobe32(uint32_t(OpenProtocol::HardwareType::Microphone));
    virtualDevice.subType = htobe32(uint32_t(OpenProtocol::MicrophoneType::Microphone));
    virtualDevice.interfaces = htobe32(uint32_t(OpenProtocol::MicFunc::InjectAudio));

    int size = sizeof(virtualDevice)+audio_size;
    uint8_t *buffer = (uint8_t *)malloc(size);
    memcpy(buffer,&virtualDevice,sizeof(virtualDevice));
    memcpy(buffer+sizeof(virtualDevice),pcm,audio_size);
    std::vector<uint8_t> vectorData(buffer,buffer+size);

#if _DEBUG
    printf("send: %d\r\n",size);
    for(int i=0;i<sizeof(virtualDevice);i++) {
        printf("%02X ",buffer[i]);
    }
    printf("\r\n");
#endif
    ws->sendBinary(vectorData);
}

bool isMicOn = false;
#define BUFFER_SIZE 4096
void readPcmData(const char* filename) {
    FILE *fp = fopen(filename,"rb");
    if (fp==NULL) {
        printf("Failed to open PCM file: %s\r\n",filename);
        return;
    }

    uint8_t buffer[BUFFER_SIZE]={0};
    while (!feof(fp) && isMicOn) {
        int nRead = fread(buffer, 1, BUFFER_SIZE, fp);
        injectAudio(buffer,nRead);
        usleep(20000);
    }
    printf("readPcmData finish\r\n");

    fclose(fp);
}

void handle_message(const std::vector<uint8_t> &message)
{
#if _DEBUG
    printf("recv:");
    for(const uint8_t &byte:message) {
        printf("%02X ",byte);
    }
    printf("\r\n");
#endif

    OpenProtocol::VirtualDevice virtualDevice;
    memcpy(&virtualDevice,message.data(),sizeof(virtualDevice));
    OpenProtocol::HardwareType type = OpenProtocol::HardwareType(htobe32(virtualDevice.type));
    virtualDevice.subType = htobe32(virtualDevice.subType);
    virtualDevice.interfaces = htobe32(virtualDevice.interfaces);

#if _DEBUG
    printf("type:%d, subType:%d, interfaces:%d\r\n",type,virtualDevice.subType,virtualDevice.interfaces);
#endif

    switch (type) {
        case OpenProtocol::HardwareType::Camera:
            if(virtualDevice.subType==int(OpenProtocol::CameraType::Camera)) {
                switch (OpenProtocol::CameraFunc(virtualDevice.interfaces)) {
                    case OpenProtocol::CameraFunc::Open:
                        printf("Camera Open\r\n");
                        break;

                    case OpenProtocol::CameraFunc::Close:
                        printf("Camera Close\r\n");
                        break;

                    default:
                        break;
                }
            }
            break;

        case OpenProtocol::HardwareType::Microphone:
            if(virtualDevice.subType==int(OpenProtocol::MicrophoneType::Microphone)) {
                switch (OpenProtocol::MicFunc(virtualDevice.interfaces)) {
                    case OpenProtocol::MicFunc::OpenMic: {
                        printf("Mic On\r\n");
                        isMicOn = true;
                        std::thread audioThread(readPcmData, "/sdcard/audio.pcm");
                        audioThread.detach();
                    }
                        break;

                    case OpenProtocol::MicFunc::CloseMic: {
                        printf("Mic Off\r\n");
                        isMicOn = false;
                    }
                        break;

                    default:
                        break;
                }
            }
            break;

        default:
            break;
    }
}

int main()
{
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
#endif

    ws = WebSocket::from_url("ws://localhost:4001");
    assert(ws);
    injectGPS(168.456789,89.123456);
    while (ws->getReadyState() != WebSocket::CLOSED) {
      ws->poll();
      ws->dispatchBinary(handle_message);
    }
    printf("ws state: %d\r\n",ws->getReadyState());
    delete ws;
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
