#ifndef NETWORK_H
#define NETWORK_H

#include "wotwar_classes.h"
#include <string>
#include <vector>
#include <asio.hpp>
#include <boost/thread.hpp>

using asio::ip::tcp;

class NETWORK
{
    public:
        NETWORK(int port);
        ~NETWORK(void);
        int num_connected(void);
        int num_connections(void);
        bool all_connected(void);
        bool any_error(void);
        void wait_player(void);
        bool connect_player(std::string host_name);
        void wait_all(void);
        void send_all(const char * buf, int n);
        bool accept_connections(void);
        void handle_accept(const asio::error_code& error);
        std::string hostname(void);
        CONNECTION * get(int connection_num);

        // async
        void listen_messages(bool enable_forwarding);
        void broadcast(const char * buf, int n, CONNECTION * pl_ignore=NULL);

        bool wait_all_sync(void);
        void send_all_sync(void);

    private:
        asio::io_service io_service;
        asio::io_service::work * pwork;
        boost::thread * net_thread;
        CONNECTION * host;
        int cur_port;
        int base_port;
        vector<CONNECTION*> connections;
        void async_thread(void);
};

#endif
