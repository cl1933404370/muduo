#include <muduo/net/TcpServer.h>

#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

int numThreads = 0;

class EchoServer
{
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr)
  {
    server_.setConnectionCallback(
        boost::bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(numThreads);
  }

  void start()
  {
    server_.start();
  }
  // void stop();

 private:
  TcpConnectionPtr first;
  TcpConnectionPtr second;
  void onConnection(const TcpConnectionPtr& conn)
  {
    printf("conn %s -> %s %s\n",
        conn->peerAddress().toHostPort().c_str(),
        conn->localAddress().toHostPort().c_str(),
        conn->connected() ? "UP" : "DOWN");
    if (!first)
      first = conn;
    else if (!second)
      second = conn;
  }

  void onMessage(const TcpConnectionPtr& conn, ChannelBuffer* buf, Timestamp time)
  {
    string msg(buf->retrieveAsString());
    printf("recv %zu bytes '%s'", msg.size(), msg.c_str());
    if (msg == "exit\n")
    {
      first->shutdown();
      first.reset();
      second->shutdown();
      second.reset();
      loop_->quit();
    }
    if (conn != first && first)
      first->send(msg);
    if (conn != second && second)
      second->send(msg);
    // conn->send(buf);
    // conn->shutdown();
    // loop_->quit();
    /*
    if (conn == second)
    {
      sleep(10);
    }
    if (second && conn == first)
    {
      second->shutdown();
      second.reset();
      first.reset();
      // loop_->quit();
    }
    */
  }

  EventLoop* loop_;
  TcpServer server_;
};

void threadFunc(uint16_t port)
{
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  numThreads = argc - 1;
  EventLoop loop;
  InetAddress listenAddr(2000);
  EchoServer server(&loop, listenAddr);

  server.start();

  loop.loop();
}
