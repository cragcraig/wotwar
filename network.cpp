#include "wotwar.h"

#include "connection.h"
#include "network.h"
#include <iostream>
#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

NETWORK::NETWORK(int port) : cur_port(port), base_port(port), host(NULL)
{
    // create a separate thread to handle async network I/O
    connections.reserve(8);
    pwork = new asio::io_service::work(io_service);
    net_thread = new boost::thread(boost::bind(&NETWORK::async_thread, this));
}

NETWORK::~NETWORK(void)
{
    io_service.stop();
    net_thread->join();

    // free all connections
    for (int i=0; i<connections.size(); i++)
        if (connections[i]) delete connections[i];

    delete pwork;
    delete net_thread;
    if (host) delete host;
}

void NETWORK::async_thread(void)
{
    try {
        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cout << e.what() << std::endl;
        panic("NETWORK: exception occured");
    }
}

int NETWORK::num_connected(void)
{
    int r = 0;
    for (int i=0; i<connections.size(); i++) {
        if (connections[i]->is_connected()) r++;
    }
    return r;
}

bool NETWORK::any_error(void)
{
    for (int i=0; i<connections.size(); i++) {
        if (connections[i]->is_error()) return true;
    }
    return false;
}

int NETWORK::num_connections(void)
{
    return connections.size();
}

bool NETWORK::all_connected(void)
{
    return (connections.size() == num_connected());
}

void NETWORK::wait_player(void)
{
    connections.push_back(new CONNECTION(io_service, cur_port));
}

void NETWORK::wait_all(void)
{
    char buf[3];
    for (int i=0; i<connections.size(); i++) {
        connections[i]->rcv(buf, 1);
    }
}

void NETWORK::send_all(const char * buf, int n)
{
    for (int i=0; i<connections.size(); i++) {
        connections[i]->snd(buf, n);
    }
}

bool NETWORK::wait_all_sync(void)
{
    for (int i=0; i<connections.size(); i++) {
        while (connections[i]->msg_empty()) {
            if (key[KEY_ESC] || !all_connected()) return false; // fail if ESC is pressed or anyone disconnects
        }
        connections[i]->msg_pop();
    }
    return true;
}

void NETWORK::send_all_sync(void)
{
    MESSAGE msg;
    msg.type = MSGTYPE_NONE;
    msg.size = sizeof(msg);

    broadcast((char*)&msg, sizeof(msg));
}

bool NETWORK::connect_player(std::string host_name)
{
    CONNECTION * con = new CONNECTION(io_service, host_name.c_str(), cur_port);
    connections.push_back(con);
    return con->is_connected();
}

CONNECTION * NETWORK::get(int connection_num)
{
    return connections[connection_num];
}

bool NETWORK::accept_connections(void)
{
    if (host) return false;
    host = new CONNECTION(io_service, base_port, this);
    cur_port = base_port + 1;
    return !host->is_error();
}

void NETWORK::handle_accept(const asio::error_code& error)
{
    if (!error) {
        host->snd(cur_port);
        host->socket()->close();
        wait_player();
        cur_port++;
    }
    host->accept_intermediate(base_port, this);
}

std::string NETWORK::hostname(void)
{
    /*if (!host) return string("<error>");
    return host->socket()->local_endpoint().address().to_string();*/

    try {
        asio::error_code error;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(asio::ip::host_name(), "");
        tcp::resolver::iterator iter = resolver.resolve(query);
        error = asio::error::host_not_found;
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;
        tcp::endpoint endp;
        std::string r;
        while (endpoint_iterator != end) {
            endp = *endpoint_iterator++;
            r += string(" ") + endp.address().to_string();
        }
        return r;

    } catch (std::exception& e) {
        std::cerr << std::string("network fail: ") << e.what() << std::endl;
        std::cout << std::string("network fail: ") << e.what() << std::endl;
        global_popup(e.what());
        return string("<error>");
    }
}

void NETWORK::broadcast(const char * buf, int n, CONNECTION * pl_ignore)
{
    for (int i=0; i<connections.size(); i++) {
        if (connections[i] != pl_ignore) {
            connections[i]->async_snd(buf, n);
        }
    }
}

void NETWORK::listen_messages(bool enable_forwarding)
{
    for (int i=0; i<connections.size(); i++) {
        connections[i]->msg_listen(enable_forwarding ? this : NULL);
    }
}
