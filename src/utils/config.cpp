#include "config.hpp"
#include "json.hpp"
#include <stdio.h>
#include <iostream>
#include "logger.hpp"
using json = nlohmann::json;

Config* Config::Instance(){
    static Config s_config;
    return &s_config;
}

#define CONFIG_DATA_BUFFER (30*1000)
int Config::load(const std::string& conf_file) {
    FILE* fh_p = fopen(conf_file.c_str(), "r");
    if (!fh_p) {
        std::cout << "open file:" << conf_file << " error\r\n";
        return -1;
    }
    uint8_t buffer_[CONFIG_DATA_BUFFER];
    size_t n = fread(buffer_, 1, CONFIG_DATA_BUFFER, fh_p);
    fclose(fh_p);

    if (n <= 0) {
        std::cout << "read file error:" << n << "\r\n";
        return -2;
    }

    return init(buffer_, n);
}

int Config::init_log(json& json_object) {
    auto log_level_iter = json_object.find("log_level");
    if (log_level_iter == json_object.end()) {
        log_level_str_ = "info";
    } else {
        log_level_str_ = log_level_iter->get<std::string>();
    }

    auto log_path_iter = json_object.find("log_dir");
    if (log_path_iter == json_object.end()) {
        log_path_ = "server.log";
    } else {
        log_path_ = log_path_iter->get<std::string>();
    }

    if(log_level_str_ == "debug") {
        log_level_ = LOGGER_DEBUG_LEVEL;
    } else if (log_level_str_ == "info") {
        log_level_ = LOGGER_INFO_LEVEL;
    } else if (log_level_str_ == "warn") {
        log_level_ = LOGGER_WARN_LEVEL;
    } else if (log_level_str_ == "error") {
        log_level_ = LOGGER_ERROR_LEVEL;
    } else {
        log_level_ = LOGGER_INFO_LEVEL;
    }
    return 0;
}

int Config::init_rtmp(json& json_object) {

    auto enable_iter = json_object.find("enable");
    if (enable_iter == json_object.end()) {
        rtmp_config_.rtmp_enable = false;
        return 0;
    }
    rtmp_config_.rtmp_enable = enable_iter->get<bool>();

    auto port_iter = json_object.find("listen");
    if (port_iter != json_object.end()) {
        rtmp_config_.listen_port = (uint16_t)port_iter->get<int>();
    }

    auto gop_cache_iter = json_object.find("gop_cache");
    if (gop_cache_iter != json_object.end()) {
        std::string gop_cache_str = gop_cache_iter->get<std::string>();
        rtmp_config_.gop_cache = (gop_cache_str == "enable") ? true : false;
    }

    auto rtmp_relay_iter = json_object.find("rtmp_relay");
    if (rtmp_relay_iter != json_object.end()) {
        rtmp_config_.rtmp_relay.enable     = (*rtmp_relay_iter)["enable"];
        rtmp_config_.rtmp_relay.relay_host = (*rtmp_relay_iter)["host"];
    }

    return 0;
}

int Config::init_httpflv(json& json_object) {

    auto enable_iter = json_object.find("enable");
    if (enable_iter == json_object.end()) {
        httpflv_config_.httpflv_enable = false;
        return 0;
    }
    httpflv_config_.httpflv_enable = enable_iter->get<bool>();

    auto ssl_iter = json_object.find("ssl");
    if (ssl_iter == json_object.end()) {
        httpflv_config_.ssl_enable = false;
        return 0;
    }
    httpflv_config_.ssl_enable = ssl_iter->get<bool>();

    auto cert_iter = json_object.find("cert_file");
    if (cert_iter == json_object.end()) {
        httpflv_config_.cert_file = "";
        return 0;
    }
    httpflv_config_.cert_file = cert_iter->get<std::string>();

    auto key_iter = json_object.find("key_file");
    if (key_iter == json_object.end()) {
        httpflv_config_.key_file = "";
        return 0;
    }
    httpflv_config_.key_file = key_iter->get<std::string>();


    auto port_iter = json_object.find("listen");
    if (port_iter != json_object.end()) {
        httpflv_config_.listen_port = (uint16_t)port_iter->get<int>();
    }

    return 0;
}

int Config::init_httpapi(json& json_object) {
    //httpapi_config_
    auto enable_iter = json_object.find("enable");
    if (enable_iter == json_object.end()) {
        httpapi_config_.httpapi_enable = false;
        return 0;
    }
    httpapi_config_.httpapi_enable = enable_iter->get<bool>();

    auto ssl_iter = json_object.find("ssl");
    if (ssl_iter == json_object.end()) {
        httpapi_config_.ssl_enable = false;
        return 0;
    }
    httpapi_config_.ssl_enable = ssl_iter->get<bool>();

    auto cert_iter = json_object.find("cert_file");
    if (cert_iter == json_object.end()) {
        httpapi_config_.cert_file = "";
        return 0;
    }
    httpapi_config_.cert_file = cert_iter->get<std::string>();

    auto key_iter = json_object.find("key_file");
    if (key_iter == json_object.end()) {
        httpapi_config_.key_file = "";
        return 0;
    }
    httpapi_config_.key_file = key_iter->get<std::string>();

    auto port_iter = json_object.find("listen");
    if (port_iter != json_object.end()) {
        httpapi_config_.listen_port = (uint16_t)port_iter->get<int>();
    }
    return 0;
}

int Config::init_hls(json& json_object) {
    auto enable_iter = json_object.find("enable");
    if (enable_iter == json_object.end()) {
        hls_config_.hls_enable = false;
        return 0;
    }
    hls_config_.hls_enable = enable_iter->get<bool>();

    auto ts_duration_iter = json_object.find("ts_duration");
    if (ts_duration_iter != json_object.end()) {
        hls_config_.ts_duration = ts_duration_iter->get<int>();
    }

    auto hls_path_iter = json_object.find("hls_path");
    if (hls_path_iter != json_object.end()) {
        hls_config_.hls_path = hls_path_iter->get<std::string>();
    }

    return 0;
}

int Config::init_websocket(json& json_object) {
    auto enable_iter = json_object.find("enable");
    if (enable_iter == json_object.end()) {
        websocket_config_.websocket_enable = false;
        return 0;
    }
    websocket_config_.websocket_enable = enable_iter->get<bool>();

    auto port_iter = json_object.find("listen");
    if (port_iter != json_object.end()) {
        websocket_config_.websocket_port = (uint16_t)port_iter->get<int>();
    }

    return 0;
}

int Config::init_webrtc(json& json_object) {
    auto enable_iter = json_object.find("enable");
    if (enable_iter == json_object.end()) {
        webrtc_config_.webrtc_enable = false;
        return 0;
    }
    webrtc_config_.webrtc_enable = enable_iter->get<bool>();

    auto port_iter = json_object.find("listen");
    if (port_iter != json_object.end()) {
        webrtc_config_.https_port = (uint16_t)port_iter->get<int>();
    }

    auto tls_key_iter = json_object.find("tls_key");
    if (tls_key_iter != json_object.end()) {
        webrtc_config_.tls_key = tls_key_iter->get<std::string>();
    }

    auto tls_cert_iter = json_object.find("tls_cert");
    if (tls_cert_iter != json_object.end()) {
        webrtc_config_.tls_cert = tls_cert_iter->get<std::string>();
    }


    auto udp_port_iter = json_object.find("udp_port");
    if (udp_port_iter != json_object.end()) {
        webrtc_config_.udp_port = (uint16_t)udp_port_iter->get<int>();
    }


    auto candidat_ip_iter = json_object.find("candidate_ip");
    if (candidat_ip_iter != json_object.end()) {
        webrtc_config_.candidate_ip = candidat_ip_iter->get<std::string>();
    } else {
        std::cout << "candidate ip is empty\r\n";
    }

    auto rtc2rtmp_iter = json_object.find("rtc2rtmp");
    if (rtc2rtmp_iter == json_object.end()) {
        webrtc_config_.rtc2rtmp_enable = false;
    } else {
        webrtc_config_.rtc2rtmp_enable = rtc2rtmp_iter->get<bool>();
    }
 
    auto rtmp2rtc_iter = json_object.find("rtmp2rtc");
    if (rtmp2rtc_iter == json_object.end()) {
        webrtc_config_.rtmp2rtc_enable = false;
    } else {
        webrtc_config_.rtmp2rtc_enable = rtmp2rtc_iter->get<bool>();
    }
    return 0;
}

int Config::init(uint8_t* data, size_t len) {
    int ret = 0;

    try {
        auto data_json = json::parse(data);

        init_log(data_json);

        auto rtmp_iter = data_json.find("rtmp");
        if (rtmp_iter != data_json.end()) {
            ret = init_rtmp(*rtmp_iter);
            if (ret < 0) {
                std::cout << "init rtmp config error" << "\r\n";
                return ret;
            }
        }

        auto httpflv_iter = data_json.find("httpflv");
        if (httpflv_iter != data_json.end()) {
            ret = init_httpflv(*httpflv_iter);
            if (ret < 0) {
                std::cout << "init httpflv config error" << "\r\n";
                return ret;
            }
        }

        auto httpapi_iter = data_json.find("httpapi");
        if (httpapi_iter != data_json.end()) {
            ret = init_httpapi(*httpapi_iter);
            if (ret < 0) {
                std::cout << "init httpapi config error" << "\r\n";
                return ret;
            }
        }

        auto hls_iter = data_json.find("hls");
        if (hls_iter != data_json.end()) {
            ret = init_hls(*hls_iter);
            if (ret < 0) {
                std::cout << "init hls config error" << "\r\n";
                return ret;
            }
        }

        auto webrtc_iter = data_json.find("webrtc");
        if (webrtc_iter != data_json.end()) {
            ret = init_webrtc(*webrtc_iter);
            if (ret < 0) {
                std::cout << "init webrtc config error" << "\r\n";
                return ret;
            }
        }
        
        auto websocket_iter = data_json.find("websocket");
        if (websocket_iter != data_json.end()) {
            ret = init_websocket(*websocket_iter);
            if (ret < 0) {
                std::cout << "init websocket config error" << "\r\n";
                return ret;
            }
        }
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        throw(e);
    }
    return 0;
}

bool Config::websocket_is_enable() {
    return websocket_config_.websocket_enable;
}

uint16_t Config::websocket_port() {
    return websocket_config_.websocket_port;
}

bool Config::rtmp_is_enable() {
    return rtmp_config_.rtmp_enable;
}

uint16_t Config::rtmp_listen_port() {
    return rtmp_config_.listen_port;
}

bool Config::rtmp_gop_cache() {
    return rtmp_config_.gop_cache;
}

bool Config::rtmp_relay_is_enable() {
    return rtmp_config_.rtmp_relay.enable;
}

std::string Config::rtmp_relay_host() {
    return rtmp_config_.rtmp_relay.relay_host;
}

bool Config::httpapi_is_enable() {
    return httpapi_config_.httpapi_enable;
}

bool Config::httpapi_ssl_enable() {
    return httpapi_config_.ssl_enable;
}

std::string Config::httpapi_cert_file() {
    return httpapi_config_.cert_file;
}

std::string Config::httpapi_key_file() {
    return httpapi_config_.key_file;
}

uint16_t Config::httpapi_port() {
    return httpapi_config_.listen_port;
}

bool Config::httpflv_is_enable() {
    return httpflv_config_.httpflv_enable;
}

uint16_t Config::httpflv_port() {
    return httpflv_config_.listen_port;
}

bool Config::httpflv_ssl_enable() {
    return httpflv_config_.ssl_enable;
}

std::string Config::httpflv_cert_file() {
    return httpflv_config_.cert_file;
}

std::string Config::httpflv_key_file() {
    return httpflv_config_.key_file;
}

bool Config::hls_is_enable() {
    return hls_config_.hls_enable;
}

std::string Config::hls_path() {
    return hls_config_.hls_path;
}

int Config::mpegts_duration() {
    return hls_config_.ts_duration;
}

bool Config::webrtc_is_enable() {
    return webrtc_config_.webrtc_enable;
}

uint16_t Config::webrtc_https_port() {
    return webrtc_config_.https_port;
}

std::string Config::tls_key() {
    return webrtc_config_.tls_key;
}

std::string Config::tls_cert() {
    return webrtc_config_.tls_cert;
}

uint16_t Config::webrtc_udp_port() {
    return webrtc_config_.udp_port;
}

std::string Config::candidate_ip() {
    return webrtc_config_.candidate_ip;
}

bool Config::rtmp2rtc_is_enable() {
    return webrtc_config_.rtmp2rtc_enable;
}

bool Config::rtc2rtmp_is_enable() {
    return webrtc_config_.rtc2rtmp_enable;
}
