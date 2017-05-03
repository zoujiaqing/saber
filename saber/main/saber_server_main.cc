// Copyright (c) 2017 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <voyager/core/eventloop.h>
#include <voyager/util/string_util.h>
#include <skywalker/node.h>
#include <saber/util/logging.h>
#include "saber/server/saber_server.h"
#include "saber/server/saber_cell.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    LOG_ERROR("Usage: %s id:ip:port:port id:ip:port:port,...\n", argv[0]);
    return -1;
  }
  char path[1024];
  if (getcwd(path, sizeof(path)) == nullptr) {
    LOG_ERROR("getcwd failed.\n");
    return -1;
  }

  saber::ServerOptions server_options;

  server_options.server_thread_size = 4;
  server_options.log_storage_path = std::string(path);

  std::vector<std::string> server;
  std::vector<std::string> servers;

  voyager::SplitStringUsing(std::string(argv[1]), ":", &server);

  saber::ServerMessage server_message;
  server_message.server_id = atoi(server[0].data());
  server_message.server_ip = server[1];
  server_message.client_port = atoi(server[2].data());
  server_message.paxos_port = atoi(server[3].data());


  voyager::SplitStringUsing(std::string(argv[2]), ",", &servers);
  for (auto& s : servers) {
    server.clear();
    voyager::SplitStringUsing(s, ":", &server);
    
    server_message.server_id = atoi(server[0].data());
    server_message.server_ip = server[1];
    server_message.client_port = atoi(server[2].data());
    server_message.paxos_port = atoi(server[3].data());

    server_options.all_server_messages.push_back(server_message);
  }

  voyager::EventLoop loop;
  saber::SaberServer saber_server(&loop, server_options);

  bool res = saber_server.Start();
  if (res) {
    LOG_INFO("SaberServer start successfully!\n");
    loop.Loop();
  } else {
    LOG_ERROR("SaberServer start failed!\n");
  }
  return 0;
}