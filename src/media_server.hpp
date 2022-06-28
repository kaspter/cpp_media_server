#ifndef MEDIA_SERVER_H
#define MEDIA_SERVER_H

#include <uv.h>

class flv_websocket;
class protoo_server;
class websocket_server;
class rtmp2rtc_writer;
class rtmp_server;
class httpflv_server;
class httpapi_server;
class hls_writer;
class rtmp_relay_manager;

class MediaServer
{
public:
    void Run(const std::string& cfg_file);
    static uv_loop_t* get_loop() { return MediaServer::loop_; }

private:
    void create_webrtc();
    void create_rtmp();
    void create_httpflv();
    void create_hls();
    void create_websocket_flv();
    void create_httpapi();

    void release_all();

private:
    static uv_loop_t* loop_;
    static uv_loop_t* hls_loop_;

private:
    websocket_server* ws_p = nullptr;
    rtmp2rtc_writer* r2r_output  = nullptr;
    hls_writer* hls_output  = nullptr;

private:
    std::shared_ptr<rtmp_server> rtmp_ptr;

private:
    std::shared_ptr<httpflv_server> httpflv_ptr;

private:
    std::shared_ptr<httpapi_server> httpapi_ptr;

private:
    flv_websocket* ws_flv_server  = nullptr;
    protoo_server* ws_webrtc_server = nullptr;
    rtmp_relay_manager* relay_mgr_p = nullptr;
};

#endif

