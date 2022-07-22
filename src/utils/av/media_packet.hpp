#ifndef MEDIA_PACKET_HPP
#define MEDIA_PACKET_HPP

#include "av.hpp"
#include "data_buffer.hpp"

#include <stdint.h>
#include <string>
#include <memory>
#include <sstream>

class MEDIA_PACKET
{
public:
    MEDIA_PACKET()
    {
        buffer_ptr_ = std::make_shared<data_buffer>();
    }
    MEDIA_PACKET(size_t len)
    {
        buffer_ptr_ = std::make_shared<data_buffer>(len);
    }
    ~MEDIA_PACKET()
    {
    }

    std::shared_ptr<MEDIA_PACKET> copy() {
        std::shared_ptr<MEDIA_PACKET> pkt_ptr = std::make_shared<MEDIA_PACKET>(this->buffer_ptr_->data_len() + 1024);
        pkt_ptr->copy_properties(*this);
        pkt_ptr->buffer_ptr_->append_data(this->buffer_ptr_->data(), this->buffer_ptr_->data_len());
        return pkt_ptr;
    }

    void copy_properties(const MEDIA_PACKET& pkt) {
        this->av_type_      = pkt.av_type_;
        this->codec_type_   = pkt.codec_type_;
        this->fmt_type_     = pkt.fmt_type_;
        this->dts_          = pkt.dts_;
        this->pts_          = pkt.pts_;
        this->is_key_frame_ = pkt.is_key_frame_;
        this->is_seq_hdr_   = pkt.is_seq_hdr_;

        this->key_        = pkt.key_;
        this->vhost_      = pkt.vhost_;
        this->app_        = pkt.app_;
        this->streamname_ = pkt.streamname_;
        this->streamid_   = pkt.streamid_;
        this->typeid_     = pkt.typeid_;
    }

    void copy_properties(const std::shared_ptr<MEDIA_PACKET> pkt_ptr) {
        copy_properties(*pkt_ptr);
    }

    std::string dump() {
        std::stringstream ss;
        ss << "type:" << avtype_tostring(av_type_) << ", codec:" << codectype_tostring(codec_type_)
           << ", format:" << formattype_tostring(fmt_type_) << ", dts:" << dts_ << ", pts:" << pts_
           << (is_key_frame_?" keyframe":"") << (is_seq_hdr_?" seqframe":"")
           << ", key:" << key_ << ", app:" << app_ << ", stream name:" << streamname_
           << ", length:" << buffer_ptr_->data_len();
        return ss.str();
    }
    
    char* data() {return buffer_ptr_->data();}
    size_t size() const {return buffer_ptr_->data_len();}
public:
    // 媒体类型：音频/视频
    MEDIA_PKT_TYPE av_type_      = MEDIA_UNKOWN_TYPE;
    // 编码类型：opus,aac,vp8,vp9
    MEDIA_CODEC_TYPE codec_type_ = MEDIA_CODEC_UNKOWN;
    // 封装格式：ts,flv, raw
    MEDIA_FORMAT_TYPE fmt_type_  = MEDIA_FORMAT_UNKOWN;
    int64_t dts_ = 0;
    int64_t pts_ = 0;
    bool is_key_frame_ = false;
    bool is_seq_hdr_   = false;
    std::shared_ptr<data_buffer> buffer_ptr_;

//rtmp info:
public:
    std::string key_;//stream_key: vhost(option)_app_streamname
    std::string vhost_;
    std::string app_;
    std::string streamname_;
    uint32_t streamid_ = 0;
    uint8_t typeid_ = 0;
};

typedef std::shared_ptr<MEDIA_PACKET> MEDIA_PACKET_PTR;

class av_writer_base
{
public:
    virtual int write_packet(MEDIA_PACKET_PTR) = 0;
    // stream key: app/streamname
    virtual std::string get_key() = 0;
    // this writer's id
    virtual std::string get_writerid() = 0;
    
    virtual void close_writer() = 0;

    virtual bool is_inited() = 0;
    virtual void set_init_flag(bool flag) = 0;
};

#endif//MEDIA_PACKET_HPP