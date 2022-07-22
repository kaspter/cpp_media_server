#include "flv_demux.hpp"
#include "flv_mux.hpp"
#include "logger.hpp"
#include <iostream>
#include <memory>
#include <sstream>

std::shared_ptr<flv_demuxer> demuxer_ptr;
std::shared_ptr<flv_muxer> muxer_ptr;
std::string src_file;
std::string dst_file;

class media_callback : public av_format_callback
{
public:
    virtual int output_packet(MEDIA_PACKET_PTR pkt_ptr) {
        log_infof("demux %s", pkt_ptr.dump().c_str());
        muxer_ptr->input_packet(pkt_ptr);
        return 0;
    }
};

class file_output_callback : public av_format_callback
{
public:
    virtual int output_packet(MEDIA_PACKET_PTR pkt_ptr) {
        std::string val = "mux " + pkt_ptr->dump();
        log_info_data((uint8_t*)pkt_ptr->buffer_ptr_->data(), pkt_ptr->buffer_ptr_->data_len(), val.c_str());

        FILE* fh = fopen(dst_file.c_str(), "ab+");
        if (fh) {
            fwrite(pkt_ptr->data(), pkt_ptr->size(), 1, fh);
            fclose(fh);
        }
        return 0;
    }
};

int main(int argn, char** argv)
{
    if (argn < 3) {
        std::cout << "please input right parameters" << std::endl;
        return -1;
    }

    src_file = argv[1];
    dst_file = argv[2];

    media_callback media_cb;
    file_output_callback file_cb;

    demuxer_ptr = std::make_shared<flv_demuxer>(&media_cb);
    muxer_ptr   = std::make_shared<flv_muxer>(true, true, &file_cb);
    
    Logger::get_instance()->set_filename("flv_mux.log");

    log_infof("input file:%s, output file:%s", src_file.c_str(), dst_file.c_str());
    FILE* fh = fopen(src_file.c_str(), "r");
    char data[4096];
    int n;

    if (fh) {
        do
        {
            n = fread(data, 1, sizeof(data), fh);
            log_infof("file read n:%d", n);
            if (n > 0) {
                MEDIA_PACKET_PTR pkt_ptr = std::make_shared<MEDIA_PACKET>();
                pkt_ptr->buffer_ptr_->append_data(data, n);
                demuxer_ptr->input_packet(pkt_ptr);
            }
        } while (n > 0);
        
        fclose(fh);
    }
    return 0;
}