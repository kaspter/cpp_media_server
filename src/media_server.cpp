#include "rtmp_server.hpp"
#include "ws_server.hpp"
#include "httpflv_server.hpp"
#include "net/webrtc/webrtc_pub.hpp"
#include "net/webrtc/rtc_dtls.hpp"
#include "net/webrtc/srtp_session.hpp"
#include "net/webrtc/rtmp2rtc.hpp"
#include "net/hls/hls_writer.hpp"
#include "net/http/http_common.hpp"
#include "net/rtmp/rtmp_relay_mgr.hpp"
#include "net/httpapi/httpapi_server.hpp"
#include "utils/byte_crypto.hpp"
#include "utils/config.hpp"
#include "utils/av/media_stream_manager.hpp"
#include "logger.hpp"
#include "media_server.hpp"
#include <stdint.h>
#include <stddef.h>
#include <iostream>

boost::asio::io_context MediaServer::io_context;
boost::asio::io_context MediaServer::hls_io_context;
static auto config = Config::Instance();

void on_play_callback(const std::string& key, void* data) {
    if (!config->rtmp_is_enable()) {
        return;
    }
    std::string host = config->rtmp_relay_host();
    if (host.empty()) {
        return;
    }

    MediaServer* server = (MediaServer*)data;
    if (server->relay_mgr_p == nullptr) {
        return;
    }

    log_infof("request a new stream:%s, host:%s", key.c_str(), host.c_str());
    server->relay_mgr_p->add_new_relay(host, key);
    return;
}

void MediaServer::create_webrtc() {
    if (!config->webrtc_is_enable()) {
        log_infof("webrtc is disable...");
        return;
    }

    byte_crypto::init();
    rtc_dtls::dtls_init(config->tls_key(), config->tls_cert());
    srtp_session::init();
    init_single_udp_server(MediaServer::io_context, config->candidate_ip(), config->webrtc_udp_port());
    init_webrtc_stream_manager_callback();

    if (config->rtmp2rtc_is_enable()) {
        this->r2r_output = new rtmp2rtc_writer();
        media_stream_manager::set_rtc_writer(this->r2r_output);
    }
    this->ws_p = new websocket_server(MediaServer::io_context, config->webrtc_https_port(), WEBSOCKET_IMPLEMENT_PROTOO_TYPE);
    log_infof("webrtc is starting, https port:%d, rtmp2rtc:%s, rtc2rtmp:%s",
            config->webrtc_https_port(),
            config->rtmp2rtc_is_enable() ? "true" : "false",
            config->rtc2rtmp_is_enable() ? "true" : "false");
    return;
}

void MediaServer::create_rtmp() {
    if (!config->rtmp_is_enable()) {
        log_infof("rtmp is disable...");
        return;
    }
    this->rtmp_ptr = std::make_shared<rtmp_server>(MediaServer::io_context, config->rtmp_listen_port());
    log_infof("rtmp server is starting, listen port:%d", config->rtmp_listen_port());

    if (config->rtmp_relay_is_enable() && !config->rtmp_relay_host().empty()) {
        MediaServer::relay_mgr_p = new rtmp_relay_manager(MediaServer::io_context);
        media_stream_manager::set_play_callback(on_play_callback, this);
    }
    return;
}

void MediaServer::create_httpflv() {
    if (!config->httpflv_is_enable()) {
        log_infof("httpflv is disable...");
        return;
    }

    if (config->httpflv_ssl_enable() && !config->httpflv_cert_file().empty() && !config->httpflv_key_file().empty()) {
        MediaServer::httpflv_ptr = std::make_shared<httpflv_server>(MediaServer::io_context, config->httpflv_port(), config->httpflv_cert_file(), config->httpflv_key_file());
    } else {
        MediaServer::httpflv_ptr = std::make_shared<httpflv_server>(MediaServer::io_context, config->httpflv_port());
    }

    log_infof("httpflv server is starting, listen port:%d", config->httpflv_port());
    return;
}

void MediaServer::create_httpapi() {
    if (!config->httpapi_is_enable()) {
        log_infof("httpapi is disable...");
        return;
    }
    if (config->httpapi_ssl_enable() && !config->httpapi_cert_file().empty() && !config->httpapi_key_file().empty()) {
        MediaServer::httpapi_ptr = std::make_shared<httpapi_server>(MediaServer::io_context, config->httpapi_port(), config->httpapi_cert_file(), config->httpapi_key_file());
    } else {
        MediaServer::httpapi_ptr = std::make_shared<httpapi_server>(MediaServer::io_context, config->httpapi_port());
    }

    log_infof("httpapi server is starting, listen port:%d", config->httpapi_port());
    return;
}

void MediaServer::create_hls() {
    if (!config->hls_is_enable()) {
        log_infof("hls is disable...");
        return;
    }
    this->hls_output = new hls_writer(MediaServer::hls_io_context, config->hls_path(), true);//enable hls
    media_stream_manager::set_hls_writer(hls_output);
    this->hls_output->run();

    log_infof("hls server is starting, hls path:%s", config->hls_path().c_str());
    return;
}

void MediaServer::create_websocket_flv() {
    if (!config->websocket_is_enable()) {
        log_infof("websocket flv is disable...");
        return;
    }

    this->ws_flv_p = new websocket_server(MediaServer::io_context, config->websocket_port(), WEBSOCKET_IMPLEMENT_FLV_TYPE);

    log_infof("websocket flv is starting, listen port:%d", config->websocket_port());
    return;
}

void MediaServer::Run(const std::string& cfg_file) {
    if (cfg_file.empty()) {
        std::cerr << "configure file is needed.\r\n";
        return;
    }

    int ret = config->load(cfg_file.c_str());
    if (ret < 0) {
        std::cout << "load config file: " << cfg_file << ", error:" << ret << "\r\n";
        return;
    }

    Logger::get_instance()->set_filename(config->log_filename());
    Logger::get_instance()->set_level(config->log_level());
    #ifdef __APPLE__
    //enable cosole in mac os
    Logger::get_instance()->enable_console();
    #endif
    log_infof("configuration file:%s", config->dump().c_str());
    
    boost::asio::io_service::work work(io_context);
    try {
        create_webrtc();
        create_rtmp();
        create_httpflv();
        create_hls();
        create_httpapi();
        create_websocket_flv();
        io_context.run();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    release_all();
}

void MediaServer::release_all() {
    if (this->ws_p) {
        delete this->ws_p;
        this->ws_p = nullptr;
    }

    if (this->ws_flv_p) {
        delete this->ws_flv_p;
        this->ws_flv_p = nullptr;
    }

    if (this->r2r_output) {
        delete this->r2r_output;
        this->r2r_output = nullptr;
    }


    if (this->hls_output) {
        delete this->hls_output;
        this->hls_output = nullptr;
    }

    return;
}

boost::asio::io_context& get_global_io_context() {
    return MediaServer::get_io_ctx();
}

int main(int argn, char** argv) {
    std::string cfg_file;

    int opt = 0;
    while ((opt = getopt(argn, argv, "c:")) != -1) {
      switch (opt) {
        case 'c':
          cfg_file = std::string(optarg);
          break;
        default:
          std::cerr << "Usage: " << argv[0] << " [-c config-file] " << std::endl;
          return -1;
      }
    }

    MediaServer server;
    server.Run(cfg_file);

    return 0;
}
