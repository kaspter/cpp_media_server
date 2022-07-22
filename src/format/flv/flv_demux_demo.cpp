#include "flv_demux.hpp"
#include "media_packet.hpp"
#include "logger.hpp"
#include <string>
#include <stdio.h>
#include <sstream>

class av_outputer : public av_format_callback
{
public:
    virtual int output_packet(MEDIA_PACKET_PTR pkt_ptr) override {
        log_info_data((uint8_t*)pkt_ptr->buffer_ptr_->data(), pkt_ptr->buffer_ptr_->data_len(), pkt_ptr.dump().c_str());
        return 0;
    }
};

int main(int argn, char** argv) {
    if (argn < 2) {
        log_infof("please input: ./flv_demux_demo ./*.flv.\r\n");
        return -1;
    }
    Logger::get_instance()->set_filename("flv_demux.log");
    
    std::string filename(argv[1]);
    log_infof("input flv file:%s", filename.c_str());

    FILE* fh_p = fopen(filename.c_str(), "r");
    if (!fh_p) {
        log_errorf("fail to open filename:%s\r\n", filename.c_str());
        return -1;
    }

    av_outputer outputer;
    flv_demuxer demuxer(&outputer);

    char buffer[2048];
    int n = 0;

    do
    {
        n = fread(buffer, 1, sizeof(buffer), fh_p);
        MEDIA_PACKET_PTR pkt_ptr = std::make_shared<MEDIA_PACKET>();
        pkt_ptr->fmt_type_ = MEDIA_FORMAT_FLV;
        pkt_ptr->key_ = "live/1000";
        pkt_ptr->buffer_ptr_->append_data(buffer, n);

        demuxer.input_packet(pkt_ptr);
    } while (n > 0);

    fclose(fh_p);

    return 0;
}