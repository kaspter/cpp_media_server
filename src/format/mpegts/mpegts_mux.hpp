#ifndef MPEGTS_MUX_HPP
#define MPEGTS_MUX_HPP
#include "mpegts_pub.hpp"
#include "data_buffer.hpp"
#include "utils/av/av.hpp"
#include "format/av_format_interface.hpp"

uint8_t* get_h264_aud_data(size_t& len);
uint8_t* get_h265_aud_data(size_t& len);

class mpegts_mux
{
public:
    mpegts_mux(av_format_callback* cb);
    ~mpegts_mux();

public:
    int input_packet(MEDIA_PACKET_PTR pkt_ptr);
    int write_pat();
    int write_pmt();
    void set_video_flag(bool flag) { has_video_ = flag;}
    bool has_video() { return has_video_; }
    void set_audio_flag(bool flag) { has_audio_ = flag;}
    bool has_audio() { return has_audio_; }

    void set_video_codec(MEDIA_CODEC_TYPE codec) { video_codec_type_ = codec; }
    void set_audio_codec(MEDIA_CODEC_TYPE codec) { audio_codec_type_ = codec; }
    MEDIA_CODEC_TYPE get_video_codec() { return video_codec_type_; }
    MEDIA_CODEC_TYPE get_audio_codec() { return audio_codec_type_; }

private:
    int generate_pat();
    int generate_pmt();
    int write_pes(MEDIA_PACKET_PTR pkt_ptr);
    int write_pes_header(int64_t data_size,
                    bool is_video, int64_t dts, int64_t pts);
    int write_ts(uint8_t* data, uint8_t flag, int64_t ts);
    int write_pcr(uint8_t* data, int64_t ts);
    int adaptation_bufinit(uint8_t* data, uint8_t data_size, uint8_t remain_bytes);

private:
    void ts_callback(uint8_t* data, MEDIA_PACKET_PTR pkt_ptr=nullptr);

private:
    av_format_callback* cb_ = nullptr;

private:
    uint32_t pmt_count_     = 1;
    uint8_t pat_data_[TS_PACKET_SIZE];
    uint8_t pmt_data_[TS_PACKET_SIZE];
    uint8_t pes_header_[TS_PACKET_SIZE];

private://about pat
    uint16_t transport_stream_id_ = 1;
    uint8_t  pat_ver_ = 0;
    uint16_t program_number_ = 1;

private://about pmt
    bool has_video_ = true;
    bool has_audio_ = true;
    MEDIA_CODEC_TYPE video_codec_type_ = MEDIA_CODEC_H264;
    MEDIA_CODEC_TYPE audio_codec_type_ = MEDIA_CODEC_AAC;

    uint16_t pmt_pid_    = 0x1001;
    uint16_t pmt_pn      = 0x01;
    uint8_t  pmt_ver_    = 0;
    uint16_t pcr_id_     = 0x100;//256
    uint16_t pminfo_len_ = 0;
    
    uint16_t video_stream_type_ = STREAM_TYPE_VIDEO_H264;
    uint16_t audio_stream_type_ = STREAM_TYPE_AUDIO_AAC;
    uint16_t video_pid_ = 0x100;//256
    uint16_t audio_pid_ = 0x101;//257
    uint8_t video_sid_ = 0;
    uint8_t audio_sid_ = 0;

    int64_t pes_header_size_ = 0;
    uint8_t video_cc_ = 0;
    uint8_t audio_cc_ = 0;
};

#endif