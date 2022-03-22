#ifndef MEDIA_SERVER_H
#define MEDIA_SERVER_H

class rtmp2rtc_writer;
class hls_writer;
class httpflv_server;
class websocket_server;
class rtmp_server;
class httpapi_server;
class rtmp_relay_manager;
#include <stdint.h>
#include <stddef.h>
#include <iostream>
#include <boost/asio.hpp>

class MediaServer
{
public:
    friend void on_play_callback(const std::string& key, void* data);

public:
    void Run(const std::string& cfg_file);
    static boost::asio::io_context& get_io_ctx() { return MediaServer::io_context; }

private:
    void create_webrtc();
    void create_rtmp();
    void create_httpflv();
    void create_hls();
    void create_websocket_flv();
    void create_httpapi();

    void release_all();

private:
    static boost::asio::io_context io_context;
    static boost::asio::io_context hls_io_context;

private:
    websocket_server* ws_p = nullptr;
    rtmp2rtc_writer* r2r_output = nullptr;

private:
    std::shared_ptr<rtmp_server> rtmp_ptr;

private:
    std::shared_ptr<httpflv_server> httpflv_ptr;
    std::shared_ptr<httpapi_server> httpapi_ptr;
private:
    hls_writer* hls_output = nullptr;

private:
    websocket_server* ws_flv_p = nullptr;
    rtmp_relay_manager* relay_mgr_p = nullptr;
};
#endif

