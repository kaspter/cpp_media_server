#include "rtmp_server.hpp"
#include "ws_server.hpp"
#include "httpflv_server.hpp"
#include "rtmp_server.hpp"
#include "ws_server.hpp"
#include "httpflv_server.hpp"
#include "net/webrtc/webrtc_pub.hpp"
#include "net/webrtc/rtc_dtls.hpp"
#include "net/webrtc/srtp_session.hpp"
#include "net/webrtc/rtmp2rtc.hpp"
#include "net/hls/hls_writer.hpp"
#include "net/httpapi/httpapi_server.hpp"
#include "net/rtmp/rtmp_relay_mgr.hpp"
#include "utils/byte_crypto.hpp"
#include "utils/config.hpp"
#include "utils/av/media_stream_manager.hpp"
#include "net/webrtc/webrtc_pub.hpp"
#include "net/webrtc/rtc_dtls.hpp"
#include "net/webrtc/srtp_session.hpp"
#include "net/webrtc/rtmp2rtc.hpp"
#include "net/hls/hls_writer.hpp"
#include "net/http/http_common.hpp"
#include "net/websocket/wsimple/flv_websocket.hpp"
#include "net/websocket/wsimple/protoo_server.hpp"
#include "utils/byte_crypto.hpp"
#include "utils/config.hpp"
#include "utils/av/media_stream_manager.hpp"
#include "logger.hpp"
#include "media_server.hpp"
#include <stdint.h>
#include <stddef.h>
#include <iostream>
#ifndef WIN32
#include <unistd.h>
#endif

uv_loop_t* MediaServer::loop_ = uv_default_loop();
uv_loop_t* MediaServer::hls_loop_ = uv_loop_new();

void on_play_callback(const std::string& key, void* data) {
    if (!Config::rtmp_is_enable()) {
        return;
    }
    std::string host = Config::rtmp_relay_host();
    if (host.empty()) {
        return;
    }
    rtmp_relay_manager* relay_mgr_p = (rtmp_relay_manager*)data;
    if (relay_mgr_p) {
        log_infof("request a new stream:%s, host:%s", key.c_str(), host.c_str());
        relay_mgr_p->add_new_relay(host, key);
    }
}

void MediaServer::create_webrtc() {
    if (!Config::webrtc_is_enable() || Config::tls_key().empty() || Config::tls_cert().empty()) {
        log_infof("webrtc is disable...");
        return;
    }

    byte_crypto::init();
    rtc_dtls::dtls_init(Config::tls_key(), Config::tls_cert());
    srtp_session::init();
    init_single_udp_server(loop_, Config::candidate_ip(), Config::webrtc_udp_port());
    init_webrtc_stream_manager_callback();

    if (Config::rtmp2rtc_is_enable()) {
        r2r_output = new rtmp2rtc_writer();
        media_stream_manager::Instance()->set_rtc_writer(r2r_output);
    }
    
    if (Config::wss_is_enable()) {
        ws_webrtc_server = new protoo_server(loop_,
                                                    Config::webrtc_https_port(),
                                                    Config::tls_key(),
                                                    Config::tls_cert());
    } else {
        ws_webrtc_server = new protoo_server(loop_, Config::webrtc_https_port());
    }
    
    log_infof("webrtc is starting, websocket port:%d, wss enable:%s rtmp2rtc:%s, rtc2rtmp:%s",
            Config::webrtc_https_port(),
            Config::wss_is_enable() ? "true" : "false",
            Config::rtmp2rtc_is_enable() ? "true" : "false",
            Config::rtc2rtmp_is_enable() ? "true" : "false");
    return;
}

void MediaServer::create_rtmp() {
    if (!Config::rtmp_is_enable()) {
        log_infof("rtmp is disable...");
        return;
    }
    rtmp_ptr = std::make_shared<rtmp_server>(loop_, Config::rtmp_listen_port());
    log_infof("rtmp server is starting, listen port:%d", Config::rtmp_listen_port());

    if (Config::rtmp_relay_is_enable() && !Config::rtmp_relay_host().empty()) {
        relay_mgr_p = new rtmp_relay_manager(loop_);
        media_stream_manager::Instance()->set_play_callback(on_play_callback, (void*)relay_mgr_p);
    }
    return;
}

void MediaServer::create_httpflv() {
    if (!Config::httpflv_is_enable()) {
        log_infof("httpflv is disable...");
        return;
    }

    log_infof("httpflv server is starting, listen port:%d, ssl enable:%s, key file:%s, cert file:%s", 
            Config::httpflv_port(),
            Config::httpflv_ssl_enable() ? "true":"false",
            Config::httpflv_key_file().c_str(),
            Config::httpflv_cert_file().c_str());
    if (Config::httpflv_ssl_enable() && !Config::httpflv_cert_file().empty() && !Config::httpflv_key_file().empty()) {
        httpflv_ptr = std::make_shared<httpflv_server>(loop_,
                                                        Config::httpflv_port(),
                                                        Config::httpflv_key_file(),
                                                        Config::httpflv_cert_file());
    } else {
        httpflv_ptr = std::make_shared<httpflv_server>(loop_, Config::httpflv_port());
    }

    return;
}

void MediaServer::create_httpapi() {
    if (!Config::httpapi_is_enable()) {
        log_infof("httpapi is disable...");
        return;
    }
    if (Config::httpapi_ssl_enable() && !Config::httpapi_cert_file().empty() && !Config::httpapi_key_file().empty()) {
        httpapi_ptr = std::make_shared<httpapi_server>(loop_,
                                                        Config::httpapi_port(),
                                                        Config::httpapi_key_file(),
                                                        Config::httpapi_cert_file());
    } else {
        httpapi_ptr = std::make_shared<httpapi_server>(loop_, Config::httpapi_port());
    }

    log_infof("httpapi server is starting, listen port:%d", Config::httpapi_port());
    return;
}

void MediaServer::create_hls() {
    if (!Config::hls_is_enable()) {
        log_infof("hls is disable...");
        return;
    }
    hls_output = new hls_writer(hls_loop_, Config::hls_path(), true);//enable hls
    media_stream_manager::Instance()->set_hls_writer(hls_output);
    hls_output->run();

    log_infof("hls server is starting, hls path:%s", Config::hls_path().c_str());
    return;
}

void MediaServer::create_websocket_flv() {
    if (!Config::websocket_is_enable()) {
        log_infof("websocket flv is disable...");
        return;
    }

    if (Config::websocket_wss_enable() &&
        !Config::websocket_key_file().empty() &&
        !Config::websocket_cert_file().empty()) {
        ws_flv_server = new flv_websocket(loop_,
                                                Config::websocket_port(),
                                                Config::websocket_key_file(),
                                                Config::websocket_cert_file());
    } else {
        ws_flv_server = new flv_websocket(loop_, Config::websocket_port());
    }

    log_infof("websocket flv is starting, listen port:%d", Config::websocket_port());
    return;
}

void MediaServer::Run(const std::string& cfg_file) {
    if (cfg_file.empty()) {
        std::cerr << "configure file is needed.\r\n";
        return;
    }

    int ret = Config::load(cfg_file.c_str());
    if (ret < 0) {
        std::cout << "load config file: " << cfg_file << ", error:" << ret << "\r\n";
        return;
    }

    Logger::get_instance()->set_filename(Config::log_filename());
    Logger::get_instance()->set_level(Config::log_level());
    #ifdef __APPLE__
    //enable cosole in mac os
    Logger::get_instance()->enable_console();
    #endif
    log_infof("configuration file:%s", Config::dump().c_str());
    
    try {
        create_webrtc();
        create_rtmp();
        create_httpflv();
        create_hls();
        create_httpapi();
        create_websocket_flv();

        uv_run(loop_, UV_RUN_DEFAULT);
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }

    release_all();
    return;
}

void MediaServer::release_all() {
    if (ws_p) {
        delete ws_p;
        ws_p = nullptr;
    }

    if (ws_flv_server) {
        delete ws_flv_server;
        ws_flv_server = nullptr;
    }

    if (r2r_output) {
        delete r2r_output;
        r2r_output = nullptr;
    }


    if (hls_output) {
        delete hls_output;
        hls_output = nullptr;
    }

    return;
}

uv_loop_t* get_global_io_context() {
    return MediaServer::get_loop();
}

int main(int argn, char** argv) {
    std::string cfg_file;
#ifdef WIN32
    if(argn > 2 && !_stricmp(argv[1],"-c") ) {
        cfg_file = argv[2];
    }
#else
    int opt = 0;
    while ((opt = getopt(argn, argv, "c:")) != -1) {
      switch (opt) {
        case 'c':
          cfg_file = std::string(optarg);
          break;
        default:
          std::cerr << "Usage: " << argv[0]
               << " [-c config-file] "
               << std::endl;
          return -1;
      }
    }
#endif
    MediaServer server;
    server.Run(cfg_file);

    return 0;
}
