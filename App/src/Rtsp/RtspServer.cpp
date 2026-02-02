#include <algorithm>
#include <assert.h>
#include <stdio.h>

#include "RtspServer.h"
#include "New.h"
#include "SysSocket.h"
#include "SysNet.h"

RtspServer* RtspServer::createNew(UsageEnvironment* env, Ipv4Address& addr)
{
    //return new RtspServer(env, addr);
    return New<RtspServer>::allocate(env, addr);
}

RtspServer::RtspServer(UsageEnvironment* env, const Ipv4Address& addr) :
    TcpServer(env, addr)
{
    mTriggerEvent = TriggerEvent::createNew(this);
    mTriggerEvent->setTriggerCallback(triggerCallback);

    mMutex = Mutex::createNew();
}

RtspServer::~RtspServer()
{
    Delete::release(mTriggerEvent);
    Delete::release(mMutex);
}

void RtspServer::handleNewConnection(int connfd)
{
    RtspConnection* conn = RtspConnection::createNew(this, connfd);
    conn->setDisconnectionCallback(disconnectionCallback, this);
    mConnections.insert(std::make_pair(connfd, conn));
}

void RtspServer::disconnectionCallback(void* arg, int sockfd)
{
    RtspServer* rtspServer = (RtspServer*)arg;
    rtspServer->handleDisconnection(sockfd);
}

void RtspServer::handleDisconnection(int sockfd)
{
    MutexLockGuard mutexLockGuard(mMutex);
    mDisconnectionlist.push_back(sockfd);
    //mEnv->scheduler()->addTriggerEvent(mTriggerEvent);
    mEnv->scheduler()->removeTriggerEvent(mTriggerEvent);
}

bool RtspServer::addMeidaSession(MediaSession* mediaSession)
{
    if(mMediaSessions.find(mediaSession->name()) != mMediaSessions.end())
        return false;

    mMediaSessions.insert(std::make_pair(mediaSession->name(), mediaSession));

    return true;
}

MediaSession* RtspServer::loopupMediaSession(std::string name)
{
    std::map<std::string, MediaSession*>::iterator it = mMediaSessions.find(name);
    if(it == mMediaSessions.end())
        return NULL;
    
    return it->second;
}

std::string RtspServer::getUrl(MediaSession* session)
{
    char url[200];
    char ip[64];
	SysNet_get_ip("eth0", PF_INET, ip, 64);
    snprintf(url, sizeof(url), "rtsp://%s:%d/%s", ip,mAddr.getPort(), session->name().c_str());

    return std::string(url);
}

void RtspServer::triggerCallback(void* arg)
{
    RtspServer* rtspServer = (RtspServer*)arg;
    rtspServer->handleDisconnectionList();
}

void RtspServer::handleDisconnectionList()
{
    MutexLockGuard mutexLockGuard(mMutex);

    for(std::vector<int>::iterator it = mDisconnectionlist.begin(); it != mDisconnectionlist.end(); ++it)
    {
        int sockfd = *it;
        std::map<int, RtspConnection*>::iterator _it = mConnections.find(sockfd);
        assert(_it != mConnections.end());
        //delete _it->second;
        Delete::release(_it->second);
        mConnections.erase(sockfd);
    }

    mDisconnectionlist.clear();
}