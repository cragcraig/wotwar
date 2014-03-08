#include "wotwar.h"

#include "connection.h"
#include <iostream>
#include <asio.hpp>
#include <cstdio>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <cstring>

CONNECTION::CONNECTION(asio::io_service& rio_service, const char * host, int port) : io_service(rio_service), csocket(rio_service), pacceptor(NULL), state(false), port(port), eerror(false), msg_phead(0), msg_ptail(0), network(NULL)
{
    string host_name(host);
    int final_port;
    connect(host_name, port);
    rcv(final_port);
    csocket.close();
    connect(host_name, final_port);
}

CONNECTION::CONNECTION(asio::io_service& rio_service, int port) : io_service(rio_service), csocket(rio_service), pacceptor(NULL), state(false), port(port), eerror(false), msg_phead(0), msg_ptail(0), network(NULL)
{
    accept(port);
}

CONNECTION::CONNECTION(asio::io_service& rio_service, int port, NETWORK * host) : io_service(rio_service), csocket(rio_service), pacceptor(NULL), state(false), port(port), eerror(false)
{
    accept_intermediate(port, host);
}

CONNECTION::~CONNECTION()
{
    io_service.stop();
    if (pacceptor) delete pacceptor;
}

bool CONNECTION::is_connected(void)
{
    return state;
}

bool CONNECTION::is_error(void)
{
    return eerror;
}

bool CONNECTION::connect(string& host, int port)
{
    try {
        tcp::resolver resolver(io_service);
        char portbuf[64];
        snprintf(portbuf, 64, "%d", port);
        tcp::resolver::query query(host, string(portbuf));
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        // try to connect
        asio::error_code error = asio::error::host_not_found;
        while (error && endpoint_iterator != end) {
            csocket.close();
            csocket.connect(*endpoint_iterator++, error);
        }
        // error
        if (error) {
            printf("network fail: unable to connect to host on port %d\n", port);
            global_popup("Unable to connect to host.");
            eerror = true;
            return false;
        }

    } catch (std::exception& e) {
        std::cerr << std::string("network fail: ") << e.what() << std::endl;
        std::cout << std::string("network fail: ") << e.what() << std::endl;
        global_popup(e.what());
        eerror = true;
        return false;
    }
    printf("network: connected to host on port %d\n", port);

    state = true;
    return true;
}

void CONNECTION::accept(int port)
{
    try {
        if (pacceptor) delete pacceptor;
        pacceptor = new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), port));
        pacceptor->listen();
        pacceptor->async_accept(csocket, boost::bind(&CONNECTION::handle_accept, this, asio::placeholders::error));
    } catch (std::exception& e) {
        std::cerr << std::string("network fail: ") << e.what() << std::endl;
        std::cout << std::string("network fail: ") << e.what() << std::endl;
        global_popup(e.what());
        eerror = true;
    }
}

void CONNECTION::accept_intermediate(int port, NETWORK * host)
{
    try {
        if (pacceptor) delete pacceptor;
        pacceptor = new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), port));
        pacceptor->set_option(asio::ip::tcp::acceptor::reuse_address(true));
        pacceptor->listen();
        pacceptor->async_accept(csocket, boost::bind(&NETWORK::handle_accept, host, asio::placeholders::error));
        printf("network: listening for connections on port %d\n", port);
    } catch (std::exception& e) {
        std::cerr << std::string("network fail: ") << e.what() << std::endl;
        std::cout << std::string("network fail: ") << e.what() << std::endl;
        global_popup(e.what());
        eerror = true;
    }
}

void CONNECTION::handle_accept(const asio::error_code& error)
{
    if (!error) state = true;
}

asio::ip::tcp::socket * CONNECTION::socket(void)
{
    return &csocket;
}

int CONNECTION::rcv(char * buf, int n)
{
    asio::error_code connection_error;
    int r = asio::read(csocket, asio::buffer(buf, n), asio::transfer_all(), connection_error);
    if (connection_error) connection_broke();
    return r;
}

void CONNECTION::snd(const char * buf, int n)
{
    asio::error_code connection_error;
    asio::write(csocket, asio::buffer(buf, n), asio::transfer_all(), connection_error);
    // error out
    if (connection_error) connection_broke();
}

int CONNECTION::rcv(int& num)
{
    char buf[16];
    asio::error_code connection_error;
    int r = asio::read(csocket, asio::buffer(buf, sizeof(buf)), asio::transfer_all(), connection_error);
    if (connection_error) connection_broke();
    sscanf(buf, "%d", &num);
    return r;
}

void CONNECTION::snd(int num)
{
    asio::error_code connection_error;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", num);
    asio::write(csocket, asio::buffer(buf, sizeof(buf)), asio::transfer_all(), connection_error);
    // error out
    if (connection_error) connection_broke();
}

void CONNECTION::async_snd(const char * buf, int n)
{
    char * msg_buf = new char[n];
    memcpy(msg_buf, buf, n);

    asio::async_write(csocket, asio::buffer(msg_buf, n), asio::transfer_all(), boost::bind(&CONNECTION::handle_finish_async_snd, this, msg_buf, asio::placeholders::error));
}

void CONNECTION::handle_finish_async_snd(char * buf, const asio::error_code& error)
{
    if (error) {
        eerror = true;
        return;
    }

    delete [] buf;
}

void CONNECTION::connection_broke(void)
{
    state = false;
    csocket.close();
    global_popup("Player disconnected.");
}

bool CONNECTION::send_file(std::string filename)
{
    asio::error_code connection_error;
    FILE * fp = fopen(filename.c_str(), "r");
    if (!fp) {
        printf("fail: the file to be transferred has gone missing!?!\n");
        return false;
    }

    // obtain file size:
    fseek(fp, 0, SEEK_END);
    int f_size = ftell(fp);
    rewind(fp);

    // load file
    char * fbuf = new char[f_size];
    fread(fbuf, 1, f_size, fp);
    fclose(fp);

    // send filename
    char sizebuf[HEADER_PACKET_SIZE];
    snprintf(sizebuf, HEADER_PACKET_SIZE, "%s", filename.c_str());
    asio::write(csocket, asio::buffer(sizebuf, HEADER_PACKET_SIZE), asio::transfer_all(), connection_error);
    if (connection_error) {
        printf("fail: transmission/connection error\n");
        fclose(fp);
        return false;
    }

    // send size
    snprintf(sizebuf, HEADER_PACKET_SIZE, "%d", f_size);
    asio::write(csocket, asio::buffer(sizebuf, HEADER_PACKET_SIZE), asio::transfer_all(), connection_error);
    if (connection_error) {
        printf("fail: transmission/connection error\n");
        fclose(fp);
        return false;
    }

    // send file
    asio::write(csocket, asio::buffer(fbuf, f_size), asio::transfer_all(), connection_error);
    if (connection_error) {
        printf("fail: transmission/connection error\n");
        fclose(fp);
        return false;
    }

    // close file
    delete [] fbuf;
    return true;
}

std::string CONNECTION::rcv_file(void)
{
    char buf[FILE_PACKET_SIZE];
    int r, len;
    std::string fname;
    asio::error_code connection_error;

    // read filename
    r = asio::read(csocket, asio::buffer(buf, HEADER_PACKET_SIZE), asio::transfer_all(), connection_error);
    fname = std::string(buf);
    if (connection_error) {
        printf("fail: transmission/connection error\n");
        return string("");
    }

    // create file
    printf("rcv_file: %s\n", buf);
    FILE * fp = fopen(buf, "w");
    if (!fp) {
        printf("fail: the map file could not be created\n");
        return string("");
    }

    r = asio::read(csocket, asio::buffer(buf, HEADER_PACKET_SIZE), asio::transfer_all(), connection_error);
    if (connection_error) {
        printf("fail: transmission/connection error\n");
        fclose(fp);
        return string("");
    }

    // get file length
    sscanf(buf, "%d", &len);

    // get file
    char * fbuf = new char[len];
    r = asio::read(csocket, asio::buffer(fbuf, len));
    if (r != len) {
        printf("fail: error receiving file\n");
        fclose(fp);
        return string("");
    }

    // write file
    fwrite(fbuf, 1, len, fp);

    fclose(fp);
    delete [] fbuf;
    return fname;
}

bool CONNECTION::msg_empty(void)
{
    return (msg_phead == msg_ptail);
}

void CONNECTION::msg_pop(void)
{
    if (msg_phead == msg_ptail) return;
    if (msg_stored[msg_ptail].type == MSGTYPE_CHAT) delete [] msg_stored[msg_ptail].msg.chat.str;
    msg_ptail++;
    if (msg_ptail >= MESSAGE_BUFFER_LEN) msg_ptail = 0;
}

void CONNECTION::msg_push(void)
{
    if ((msg_phead + 1)%MESSAGE_BUFFER_LEN == msg_ptail) panic("CONNECTION: msg_push(): msg_stored[] buffer overrun!");
    msg_phead++;
    if (msg_phead >= MESSAGE_BUFFER_LEN) msg_phead = 0;
}

MESSAGE* CONNECTION::msg_head(void)
{
    return msg_stored + msg_phead;
}

MESSAGE* CONNECTION::msg_tail(void)
{
    return msg_stored + msg_ptail;
}

void CONNECTION::msg_listen(NETWORK * forward_network)
{
    if (forward_network) network = forward_network;
    asio::async_read(csocket, asio::buffer(msg_head(), sizeof(struct MESSAGE)), asio::transfer_all(), boost::bind(&CONNECTION::handle_listen, this, msg_head(), asio::placeholders::error));
}

void CONNECTION::handle_listen(MESSAGE * msg, const asio::error_code& error)
{
    if (error) {
        eerror = true;
        connection_broke();
        return;
    }

    // forward to network if requested
    if (network && msg->type != MSGTYPE_NONE) network->broadcast((char*)msg, sizeof(msg), this);

    // listen for another message
    if (msg->type == MSGTYPE_CHAT) {
        msg->msg.chat.str = new char[msg->msg.chat.len];
        asio::async_read(csocket, asio::buffer(msg->msg.chat.str, msg->msg.chat.len), asio::transfer_all(), boost::bind(&CONNECTION::handle_listen_chat, this, msg_head(), asio::placeholders::error));
    } else {
        msg_push();
        msg_listen();
    }
}

void CONNECTION::handle_listen_chat(MESSAGE * msg, const asio::error_code& error)
{
    if (error) {
        eerror = true;
        connection_broke();
        return;
    }

    // forward to network if requested
    if (network) network->broadcast(msg->msg.chat.str, msg->msg.chat.len, this);

    msg_push();
    msg_listen();
}
