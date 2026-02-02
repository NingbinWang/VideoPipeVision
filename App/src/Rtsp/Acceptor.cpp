#include "Acceptor.h"
#include "TcpSocket.h"
#include "Logger.h"
#include "New.h"

Acceptor* Acceptor::createNew(UsageEnvironment* env, const Ipv4Address& addr)
{
    //return new Acceptor(env, addr);
    return New<Acceptor>::allocate(env, addr);
}

Acceptor::Acceptor(UsageEnvironment* env, const Ipv4Address& addr) :
    mEnv(env),
    mAddr(addr),
    mSocket(mSocket.creat_socket()),
    mNewConnectionCallback(NULL)
{
    mSocket.setReuseAddr(1);
    mSocket.bind(mAddr);
    mAcceptIOEvent = IOEvent::createNew(mSocket.fd(), this);
    mAcceptIOEvent->setReadCallback(readCallback);
    mAcceptIOEvent->enableReadHandling();
}

Acceptor::~Acceptor()
{
    if(mListenning)
        mEnv->scheduler()->removeIOEvent(mAcceptIOEvent);
    Delete::release(mAcceptIOEvent);
}

void Acceptor::listen()
{
    mListenning = true;
    mSocket.listen(1024);
    mEnv->scheduler()->addIOEvent(mAcceptIOEvent);
}

void Acceptor::setNewConnectionCallback(NewConnectionCallback cb, void* arg)
{
    mNewConnectionCallback = cb;
    mArg = arg;
}

void Acceptor::readCallback(void* arg)
{
    Acceptor* acceptor = (Acceptor*)arg;
    acceptor->handleRead();
}

void Acceptor::handleRead()
{
    int connfd = mSocket.accept();
    LOG_DEBUG("client connect: %d\n", connfd);
    if(mNewConnectionCallback)
        mNewConnectionCallback(mArg, connfd);
}
