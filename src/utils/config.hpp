#ifndef MEDIA_SERVER_CONFIG_H
#define MEDIA_SERVER_CONFIG_H

#include "json.hpp"
#include "logger.hpp"

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <sstream>

using json = nlohmann::json;

#define WEBRTC_HTTPS_PORT  8000
#define WEBRTC_UDP_PORT    7000
#define RTMP_DEF_PORT      1935
#define HTTPFLV_DEF_PORT   8080
#define HTTPAPI_DEF_PORT   8090
#define WEBSOCKET_DEF_PORT 9000
#define HLS_MPEGTS_DEF_DURATION 5000 //ms
#define HLS_DEF_PATH "./hls"

#define CONFIG_DATA_BUFFER (30*1000)

class WebSocketConfg
{
public:
    WebSocketConfg() {};
    ~WebSocketConfg() {};

public:
    std::string dump() {
        std::stringstream ss;
        ss << "websocket config:\r\n";
        ss << "  enable: " << websocket_enable << "\r\n";
        ss << "  port: " << websocket_port << "\r\n";

        return ss.str();
    }

public:
    bool websocket_enable   = false;
    uint16_t websocket_port = WEBSOCKET_DEF_PORT;
};

class WebrtcConfig
{
public:
    WebrtcConfig() {};
    ~WebrtcConfig() {};

public:
    std::string dump() {
        std::stringstream ss;
        ss << "webrtc config:\r\n";
        ss << "  enable: " << webrtc_enable << "\r\n";
        ss << "  https port: " << https_port << "\r\n";
        ss << "  tls key: " << tls_key << "\r\n";
        ss << "  tls cert: " << tls_cert << "\r\n";
        ss << "  webrtc udp: " << udp_port << "\r\n";
        ss << "  candidate ip: " << candidate_ip << "\r\n";
        ss << "  rtmp2rtc: " << rtmp2rtc_enable << "\r\n";
        ss << "  rtc2rtmp: " << rtc2rtmp_enable << "\r\n";

        return ss.str();
    }

public:
    bool webrtc_enable = false;
    uint16_t https_port = WEBRTC_HTTPS_PORT;
    std::string tls_key;
    std::string tls_cert;
    uint16_t udp_port = WEBRTC_UDP_PORT;
    std::string candidate_ip;
    bool rtmp2rtc_enable = false;
    bool rtc2rtmp_enable = false;
};

class RtmpRelayConfig
{
public:
    RtmpRelayConfig(){};
    ~RtmpRelayConfig(){};

public:
    std::string dump() {
        std::stringstream ss;

        ss << "  rtmp relay:" << this->enable << "\r\n";
        ss << "  rtmp host:" << this->relay_host << "\r\n";

        return ss.str();
    }

public:
    bool enable = false;
    std::string relay_host;
};

class RtmpConfig
{
public:
    RtmpConfig(){};
    ~RtmpConfig(){};
public:
    std::string dump() {
        std::stringstream ss;

        ss << "rtmp config:\r\n";
        ss << "  enable: " << rtmp_enable << "\r\n";
        ss << "  port: " << listen_port << "\r\n";
        ss << "  gop cache: " << gop_cache << "\r\n";
        ss << rtmp_relay.dump();

        return ss.str();
    }

public:
    bool rtmp_enable = false;
    uint16_t listen_port = RTMP_DEF_PORT;
    bool gop_cache = true;

    RtmpRelayConfig rtmp_relay;
};

class HttpflvConfig
{
public:
    HttpflvConfig() {};
    ~HttpflvConfig() {};
public:
    std::string dump() {
        std::stringstream ss;

        ss << "httpflv config:\r\n";
        ss << "  enable: " << httpflv_enable << "\r\n";
        ss << "  ssl:" << ssl_enable << "\r\n";
        ss << "  cert_file:" << cert_file << "\r\n";
        ss << "  key_file:" << key_file << "\r\n";
        ss << "  port: " << listen_port << "\r\n";

        return ss.str();
    }

public:
    bool httpflv_enable = false;
    bool ssl_enable = false;
    std::string cert_file;
    std::string key_file;
    uint16_t listen_port = HTTPFLV_DEF_PORT;
};

class HlsConfig
{
public:
    HlsConfig() {};
    ~HlsConfig() {};
public:
    std::string dump() {
        std::stringstream ss;

        ss << "hls config:\r\n";
        ss << "  enable: " << hls_enable << "\r\n";
        ss << "  mpegts duration: " << ts_duration << "\r\n";
        ss << "  hls path: " << hls_path << "\r\n";

        return ss.str();
    }

public:
    bool hls_enable = false;
    int ts_duration = HLS_MPEGTS_DEF_DURATION;
    std::string hls_path = HLS_DEF_PATH;
};

class HttpApiConfig
{
public:
    HttpApiConfig() {};
    ~HttpApiConfig() {};

public:
    std::string dump() {
        std::stringstream ss;

        ss << "http api config:\r\n";
        ss << "  enable: " << httpapi_enable << "\r\n";
        ss << "  ssl:" << ssl_enable << "\r\n";
        ss << "  cert_file:" << cert_file << "\r\n";
        ss << "  key_file:" << key_file << "\r\n";
        ss << "  port: " << listen_port << "\r\n";

        return ss.str();
    }

public:
    bool httpapi_enable  = false;
    bool ssl_enable = false;
    std::string cert_file;
    std::string key_file;
    uint16_t listen_port = HTTPAPI_DEF_PORT;
};

class Config
{
public:
    int load(const std::string& conf_file);
    static Config* Instance();

    std::string dump() {
        std::stringstream ss;

        ss << "log file path:" << log_path_ << "\r\n";
        ss << "log file level:" << log_level_ << "\r\n";
        ss << rtmp_config_.dump();
        ss << httpflv_config_.dump();
        ss << hls_config_.dump();
        ss << httpapi_config_.dump();
        ss << webrtc_config_.dump();
        ss << websocket_config_.dump();

        return ss.str();
    }

public:
    bool rtmp_is_enable();
    uint16_t rtmp_listen_port();
    bool rtmp_gop_cache();
    bool rtmp_relay_is_enable();
    std::string rtmp_relay_host();

public:
    bool httpflv_is_enable();
    bool httpflv_ssl_enable();
    std::string httpflv_cert_file();
    std::string httpflv_key_file();
    uint16_t httpflv_port();

public:
    bool httpapi_is_enable();
    bool httpapi_ssl_enable();
    std::string httpapi_cert_file();
    std::string httpapi_key_file();
    uint16_t httpapi_port();

public:
    bool hls_is_enable();
    std::string hls_path();
    int mpegts_duration();

public:
    bool webrtc_is_enable();
    uint16_t webrtc_https_port();
    std::string tls_key();
    std::string tls_cert();
    uint16_t webrtc_udp_port();
    std::string candidate_ip();
    bool rtmp2rtc_is_enable();
    bool rtc2rtmp_is_enable();

public:
    bool websocket_is_enable();
    uint16_t websocket_port();

public:
    std::string log_filename() { return log_path_; }
    enum LOGGER_LEVEL log_level() { return log_level_; }

private:
    int init(uint8_t* data, size_t len);
    int init_log(json& json_object);
    int init_rtmp(json& json_object);
    int init_httpflv(json& json_object);
    int init_hls(json& json_object);
    int init_webrtc(json& json_object);
    int init_websocket(json& json_object);
    int init_httpapi(json& json_object);


private:
    std::string log_level_str_;//"error","info", "warning"
    enum LOGGER_LEVEL log_level_;
    std::string log_path_;

private:
    RtmpConfig rtmp_config_;
    HttpflvConfig httpflv_config_;
    HlsConfig hls_config_;
    WebrtcConfig webrtc_config_;
    WebSocketConfg websocket_config_;
    HttpApiConfig httpapi_config_;
};

#endif
