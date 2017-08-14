#include "saber/server/connection_monitor.h"
#include "saber/util/logging.h"
#include "saber/util/mutexlock.h"

namespace saber {

ConnectionMonitor::ConnectionMonitor(int max_all_connections,
                                     int max_ip_connections)
    : kMaxAllConnections(max_all_connections),
      kMaxIpConnections(max_ip_connections) {}

ConnectionMonitor::~ConnectionMonitor() {}

bool ConnectionMonitor::OnConnection(const voyager::TcpConnectionPtr& p) {
  bool result = true;
  const std::string& ip = p->PeerSockAddr().Ip();
  mutex_.Lock();
  if (++counter_ > kMaxAllConnections) {
    result = false;
    LOG_WARN("the all connection size is %d, more than %d, so force close it.",
             counter_, kMaxAllConnections);
  } else {
    auto it = ip_counter_.find(ip);
    if (it != ip_counter_.end()) {
      if (++(it->second) > kMaxIpConnections) {
        LOG_WARN(
            "the connection size of ip=%s is %d, more than %d, "
            "so force close it.",
            ip.c_str(), it->second, kMaxIpConnections);
        result = false;
      }
    } else {
      ip_counter_.insert(std::make_pair(ip, 1));
    }
  }
  mutex_.UnLock();

  if (!result) {
    p->ForceClose();
  }
  return result;
}

void ConnectionMonitor::OnClose(const voyager::TcpConnectionPtr& p) {
  const std::string& ip = p->PeerSockAddr().Ip();
  MutexLock lock(&mutex_);
  auto it = ip_counter_.find(ip);
  if (it != ip_counter_.end()) {
    if (--(it->second) <= 0) {
      ip_counter_.erase(it);
    }
  }
  --counter_;
}

}  // namespace saber