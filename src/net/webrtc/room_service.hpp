#ifndef ROOM_SERVICE_HPP
#define ROOM_SERVICE_HPP
#include "net/websocket/wsimple/protoo_pub.hpp"
#include "utils/av/media_stream_manager.hpp"
#include "utils/timer.hpp"
#include "rtc_session_pub.hpp"
#include "user_info.hpp"
#include "json.hpp"
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;

class rtc_subscriber;
class http_response;

typedef std::unordered_map<std::string, std::shared_ptr<rtc_subscriber>> SUBSCRIBER_MAP;

class webrtc_stream_manager_callback : public stream_manager_callbackI
{
public:
    webrtc_stream_manager_callback(){}
    virtual ~webrtc_stream_manager_callback(){}

public:
    virtual void on_publish(const std::string& app, const std::string& streamname) override;
    virtual void on_unpublish(const std::string& app, const std::string& streamname) override;
};

class room_service : public protoo_event_callback, public room_callback_interface, public timer_interface
{
public:
    room_service(const std::string& roomId);
    virtual ~room_service();

public:
    friend int get_subscriber_statics(const std::string& roomId, const std::string& uid, json& data_json);
    friend int get_room_statics(json& data_json);

public:
    virtual void on_open() override;
    virtual void on_failed() override;
    virtual void on_disconnected() override;
    virtual void on_close() override;
    virtual void on_request(const std::string& id, const std::string& method, const std::string& data,
                        protoo_request_interface* feedback_p, void* ws_session) override;
    virtual void on_response(int err_code, const std::string& err_message, const std::string& id, 
                        const std::string& data) override;
    virtual void on_notification(const std::string& method, const std::string& data) override;

public://implement timer_interface
    virtual void on_timer() override;

public:
    virtual void on_rtppacket_publisher2room(rtc_publisher* publisher, rtp_packet* pkt) override;
    virtual void on_rtppacket_publisher2room(const std::string& publisher_id, const std::string& media_type, rtp_packet* pkt) override;
    virtual void on_request_keyframe(const std::string& pid, const std::string& sid, uint32_t media_ssrc) override;
    virtual void on_unpublish(const std::string& pid) override;
    virtual void on_unsubscribe(const std::string& pid, const std::string& sid) override;
    virtual void on_rtmp_callback(const std::string& roomId, const std::string& uid,
                                const std::string& stream_type, MEDIA_PACKET_PTR pkt_ptr) override;
    virtual void on_update_alive(const std::string& roomId, const std::string& uid, int64_t now_ms) override;
public:
    bool has_rtc_user(const std::string& uid);
    void rtmp_stream_ingest(MEDIA_PACKET_PTR pkt_ptr);
    void remove_live_user(const std::string& roomid, const std::string& uid);
    std::shared_ptr<user_info> get_rtc_user(const std::string& uid);
    std::shared_ptr<live_user_info> get_live_user(const std::string& uid);

public:
    void handle_http_join(const std::string& uid);
    int handle_http_publish(const std::string& uid, const std::string& data, std::string& resp_sdp,
                        std::string& session_id, std::string& err_msg);
    int handle_http_unpublish(const std::string& uid, std::string& err_msg);
    int handle_http_unpublish(const std::string& uid, const std::string& sessionid, std::string& err_msg);
    int handle_http_subscribe(const std::string& my_uid, const std::string& remote_uid, const std::string& data,
                        std::string& resp_sdp, std::string& session_id, std::string& err_msg);
    int handle_http_webrtc_subscribe(std::shared_ptr<user_info> user_ptr, std::shared_ptr<user_info> remote_user_ptr, const std::string& data,
                        std::string& resp_sdp, std::string& session_id, std::string& err_msg);
    int handle_http_live_subscribe(std::shared_ptr<user_info> user_ptr, std::shared_ptr<live_user_info> remote_user_ptr, const std::string& data,
                        std::string& resp_sdp, std::string& session_id, std::string& err_msg);
    int handle_http_unsubscribe(const std::string& my_uid, const std::string& remote_uid, std::string& err_msg);
    int handle_http_unsubscribe(const std::string& my_uid, const std::string& remote_uid, const std::string& sessionid, std::string& err_msg);

private:
    void handle_join(const std::string& id, const std::string& method,
                const std::string& data, protoo_request_interface* feedback_p, void* ws_session);
    void handle_publish(const std::string& id, const std::string& method,
                const std::string& data, protoo_request_interface* feedback_p, void* ws_session);
    void response_publish(const std::string& id, protoo_request_interface* feedback_p,
                int code, const std::string& desc, const std::string& sdp, const std::string& pc_id, void* ws_session);
    void handle_unpublish(const std::string& id, const std::string& method,
                const std::string& data, protoo_request_interface* feedback_p, void* ws_session);
    void handle_subscribe(const std::string& id, const std::string& method,
                const std::string& data, protoo_request_interface* feedback_p, void* ws_session);
    void handle_live_subscribe(const std::string& id, const json& data_json, protoo_request_interface* feedback_p, void* ws_session);
    void handle_webrtc_subscribe(const std::string& id, const json& data_json, protoo_request_interface* feedback_p, void* ws_session);
    void handle_unsubscribe(const std::string& id, const std::string& method,
                const std::string& data, protoo_request_interface* feedback_p, void* ws_session);
    void handle_heartbeat(const std::string& id, const std::string& method,
                const std::string& data, protoo_request_interface* feedback_p, void* ws_session);
private:
    std::shared_ptr<live_user_info> live_user_join(const std::string& roomId, const std::string& uid);
    int live_publish(const std::string& uid,
                    const std::string& publisher_id,
                    bool has_video, bool has_audio,
                    uint32_t video_ssrc, uint32_t audio_ssrc,
                    uint32_t video_mid, uint32_t audio_mid);

private:
    std::shared_ptr<user_info> get_user_info(const std::string& uid);
    std::shared_ptr<live_user_info> get_live_user_info(const std::string& uid);
    std::string get_uid_by_json(const json& json_obj);
    std::vector<publisher_info> get_publishers_info_by_json(const json& publishers_json);
    void insert_subscriber(const std::string& publisher_id, std::shared_ptr<rtc_subscriber> subscriber_ptr);
    void notify_userin_to_others(const std::string& uid, const std::string& user_type);
    void notify_userout_to_others(const std::string& uid);
    void notify_publisher_to_others(const std::string& uid, const std::string& user_type, const std::string& pc_id, const std::vector<publisher_info>& publisher_vec);
    void notify_unpublisher_to_others(const std::string& uid, const std::vector<publisher_info>& publisher_vec);
    void notify_others_publisher_to_me(const std::string& uid, std::shared_ptr<user_info> me);
    
private:
    std::string roomId_;
    std::unordered_map<std::string, std::shared_ptr<user_info>> users_;//key: uid, value: user_info
    std::unordered_map<std::string, std::shared_ptr<live_user_info>> live_users_;//key: uid, value: live_user_info
    std::unordered_map<std::string, SUBSCRIBER_MAP> pid2subscribers_;//key: publisher_id, value: rtc_subscribers
};

std::shared_ptr<room_service> GetorCreate_room_service(const std::string& roomId);
void remove_room_service(const std::string& roomId);
bool room_has_rtc_uid(const std::string& roomId, const std::string& uid);

#endif
