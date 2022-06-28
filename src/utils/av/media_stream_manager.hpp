#ifndef RTMP_MEDIA_STREAM_HPP
#define RTMP_MEDIA_STREAM_HPP

#include "media_packet.hpp"
#include "gop_cache.hpp"
#include <stdint.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <list>

class rtmp_server_session;
class rtmp_request;

typedef void (*PLAY_CALLBACK)(const std::string& key, void* data);

// writer_id -> writer
typedef std::unordered_map<std::string, av_writer_base*> WRITER_MAP;
class media_stream
{
public:
    std::string stream_key_;//app/streamname
    bool publisher_exist_ = false;
    gop_cache cache_;
    WRITER_MAP writer_map_;//(session_key, av_writer_base*)
};

typedef std::shared_ptr<media_stream> MEDIA_STREAM_PTR;

class stream_manager_callbackI
{
public:
    virtual void on_publish(const std::string& app, const std::string& streamname) = 0;
    virtual void on_unpublish(const std::string& app, const std::string& streamname) = 0;
};

class media_stream_manager
{
public:
    static media_stream_manager* Instance();
    int add_player(av_writer_base* writer_p);
    void remove_player(av_writer_base* writer_p);

    MEDIA_STREAM_PTR add_publisher(const std::string& stream_key);
    void remove_publisher(const std::string& stream_key);

    void set_hls_writer(av_writer_base* writer);
    void set_rtc_writer(av_writer_base* writer);

    void set_play_callback(PLAY_CALLBACK cb, void* data);
    PLAY_CALLBACK get_play_callback();
    
public:
    int writer_media_packet(MEDIA_PACKET_PTR pkt_ptr);

public:
    void add_stream_callback(stream_manager_callbackI* cb) {
        cb_vec_.push_back(cb);
    }

private:
    std::unordered_map<std::string, MEDIA_STREAM_PTR> media_streams_map_;//key("app/stream"), MEDIA_STREAM_PTR
    std::vector<stream_manager_callbackI*> cb_vec_;

private:
    av_writer_base* hls_writer_ = nullptr;
    av_writer_base* r2r_writer_ = nullptr;//rtmp to webrtc

private:
    PLAY_CALLBACK play_cb_ = nullptr;
    void* play_data_ = nullptr;
};

#endif //RTMP_MEDIA_STREAM_HPP