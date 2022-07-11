#include "flv_websocket.hpp"
#include "media_stream_manager.hpp"
#include "flv_pub.hpp"
#include "flv_mux.hpp"
#include "logger.hpp"
#include "utils/byte_stream.hpp"
#include "format/flv/flv_demux.hpp"
#include "net/websocket/ws_session.hpp"
#include <sstream>
#include <assert.h>
#ifdef ENABLE_FFMPEG
#include "transcode/transcode.hpp"
#endif

av_outputer::av_outputer()
{
}

av_outputer::~av_outputer()
{
    release();
}

void av_outputer::release() {
#ifdef ENABLE_FFMPEG
    if (trans_) {
        trans_->stop();
        delete trans_;
        trans_ = nullptr;
    }
#endif
}

int av_outputer::output_packet(MEDIA_PACKET_PTR pkt_ptr) {
    int ret = 0;
    auto stm_mgr = media_stream_manager::Instance();
    if ((pkt_ptr->av_type_ == MEDIA_AUDIO_TYPE) && (pkt_ptr->codec_type_ == MEDIA_CODEC_OPUS)) {
#ifdef ENABLE_FFMPEG
        if (trans_ == nullptr) {
            trans_ = new transcode();
            trans_->set_output_audio_fmt("libfdk_aac");
            trans_->set_output_format(MEDIA_FORMAT_FLV);
            trans_->set_output_audio_samplerate(44100);
            trans_->start();
        }
        trans_->send_transcode(pkt_ptr, 0);
        while(true) {
            MEDIA_PACKET_PTR ret_pkt_ptr = trans_->recv_transcode();
            if (!ret_pkt_ptr) {
                break;
            }
            ret_pkt_ptr->key_        = pkt_ptr->key_;
            ret_pkt_ptr->app_        = pkt_ptr->app_;
            if (ret_pkt_ptr->is_seq_hdr_) {
                log_infof("audio seq dts:%ld, pts:%ld",
                        ret_pkt_ptr->dts_, ret_pkt_ptr->pts_);
                log_info_data((uint8_t*)ret_pkt_ptr->buffer_ptr_->data(),
                        ret_pkt_ptr->buffer_ptr_->data_len(),
                        "audio seq data");
            }           
            ret_pkt_ptr->streamname_ = pkt_ptr->streamname_;
            ret = stm_mgr->writer_media_packet(ret_pkt_ptr);
        }
#endif
    } else {
        ret = flv_muxer::add_flv_media_header(pkt_ptr);
        if (ret < 0) {
            return ret;
        }
        pkt_ptr->fmt_type_ = MEDIA_FORMAT_FLV;
        ret = stm_mgr->writer_media_packet(pkt_ptr);
    }

    return ret;
}

flv_websocket::flv_websocket(uv_loop_t* loop,
                        uint16_t port):server_(loop, port, this)
{

}

flv_websocket::flv_websocket(uv_loop_t* loop, uint16_t port,
                const std::string& key_file,
                const std::string& cert_file):server_(loop, port, this, key_file, cert_file)
{
}

flv_websocket::~flv_websocket()
{

}

void flv_websocket::on_accept(websocket_session* session) {
    log_infof("flv in websocket is accept....");
}

void flv_websocket::on_read(websocket_session* session, const char* data, size_t len) {
    if (session->get_uri().empty()) {
        std::string uri = session->path();
        size_t pos = uri.find(".flv");
        if (pos != std::string::npos) {
            uri = uri.substr(0, pos);
        }
        if (uri[0] == '/') {
            uri = uri.substr(1);
        }
        log_infof("websocket uri:%s, path:%s", uri.c_str(), session->path().c_str());

        session->set_uri(uri);
        media_stream_manager::Instance()->add_publisher(uri);
    }
    
    if (session->outputer_ == nullptr) {
        session->outputer_ = new av_outputer();
    }

    if (session->demuxer_ == nullptr) {
        session->demuxer_ = new flv_demuxer(session->outputer_);
    }

    MEDIA_PACKET_PTR pkt_ptr = std::make_shared<MEDIA_PACKET>();
    pkt_ptr->buffer_ptr_->append_data(data, len);
    pkt_ptr->key_ = session->get_uri();
    pkt_ptr->fmt_type_ = MEDIA_FORMAT_FLV;
    session->demuxer_->input_packet(pkt_ptr);
}

void flv_websocket::on_close(websocket_session* session) {
    log_errorf("flv in websocket is closed, uri:%s", session->get_uri().c_str());
    if (!session->get_uri().empty()) {
        media_stream_manager::Instance()->remove_publisher(session->get_uri());
    }
    outputer_.release();
}
