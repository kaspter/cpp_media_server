#ifndef RTC_PUBLISH_HPP
#define RTC_PUBLISH_HPP
#include "rtc_media_info.hpp"
#include "net/rtprtcp/rtp_packet.hpp"
#include "net/rtprtcp/rtcp_sr.hpp"
#include "utils/timer.hpp"
#include "utils/timeex.hpp"
#include "rtp_stream.hpp"
#include "rtc_stream_pub.hpp"
#include <vector>
#include <stdint.h>
#include <stddef.h>
#include <string>

class rtc_base_session;
class room_callback_interface;

class rtc_publisher : public timer_interface, public rtc_stream_callback
{
public:
    rtc_publisher(room_callback_interface* room, rtc_base_session* session, const MEDIA_RTC_INFO& media_info);
    virtual ~rtc_publisher();

public:
    void on_handle_rtppacket(rtp_packet* pkt);
    void on_handle_rtcp_sr(rtcp_sr_packet* sr_pkt);

public:
    std::string get_media_type();
    uint32_t get_rtp_ssrc() {return rtp_ssrc_;}
    uint32_t get_rtx_ssrc() {return rtx_ssrc_;}
    int get_clockrate();
    uint8_t get_rtp_payloadtype();
    uint8_t get_rtx_payloadtype();
    bool has_rtx();

public://implement timer_interface
    virtual void on_timer() override;

public://implement rtc_stream_callback
    virtual void stream_send_rtcp(uint8_t* data, size_t len) override;
    virtual void stream_send_rtp(uint8_t* data, size_t len) override;

private:
    room_callback_interface* room_ = nullptr;
    rtc_base_session* session_ = nullptr;

private:
    MEDIA_RTC_INFO media_info_;
    std::string media_type_;
    uint32_t rtp_ssrc_       = 0;
    uint32_t rtx_ssrc_       = 0;
    int clock_rate_          = 0;
    uint8_t payloadtype_     = 0;
    uint8_t rtx_payloadtype_ = 0;
    bool has_rtx_            = false;

private://for rtcp sr
    NTP_TIMESTAMP ntp_;//from rtcp sr
    uint32_t sr_ssrc_      = 0;
    int64_t rtp_timestamp_ = 0;
    int64_t sr_local_ms_   = 0;
    uint32_t pkt_count_    = 0;
    uint32_t bytes_count_  = 0;

private:
    rtp_stream* rtp_handler_ = nullptr;
};

#endif