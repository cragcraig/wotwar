#ifndef CONNECTION_H
#define CONNECTION_H

#include "wotwar_classes.h"
#include <cstdio>
#include <string>
#include <asio.hpp>
#include <boost/thread.hpp>
#include <asio/error_code.hpp>

using asio::ip::tcp;

class CONNECTION
{
    public:
        CONNECTION(asio::io_service& rio_service, const char * host, int port);
        CONNECTION(asio::io_service& rio_service, int port);
        CONNECTION(asio::io_service& rio_service, int port, NETWORK * host);
        ~CONNECTION();
        bool is_connected(void);
        bool is_error(void);
        bool send_file(std::string filename);
        std::string rcv_file(void);
        asio::ip::tcp::socket * socket(void);
        void accept_intermediate(int port, NETWORK * host);

        // store/read messages
        bool msg_empty(void);
        void msg_pop(void);
        void msg_push(void);
        MESSAGE* msg_head(void);
        MESSAGE* msg_tail(void);
        void msg_listen(NETWORK * forward_network=NULL);

        // meat and potatoes
        int rcv(char * buf, int n);
        void snd(const char * buf, int n);
        int rcv(int& num); // rcv an int (blocking)
        void snd(int num); // send an int (blocking)
        void async_snd(const char * buf, int n);

    private:
        bool connect(string& host, int port);
        void accept(int port);
        void handle_accept(const asio::error_code& error);
        void connection_broke(void);
        asio::io_service& io_service;
        asio::ip::tcp::socket csocket;
        asio::ip::tcp::acceptor * pacceptor;
        int port;
        bool state;
        bool eerror;

        // messages
        int msg_phead;
        int msg_ptail;
        MESSAGE msg_stored[MESSAGE_BUFFER_LEN];
        NETWORK * network;
        void handle_listen(MESSAGE * msg, const asio::error_code& error);
        void handle_listen_chat(MESSAGE * msg, const asio::error_code& error);
        void handle_finish_async_snd(char * buf, const asio::error_code& error);
};

#endif
