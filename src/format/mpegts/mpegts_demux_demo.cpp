#include "logger.hpp"
#include "mpegts_demux.hpp"
#include "av_format_interface.hpp"
#include <string>
#include <memory>
#include <sstream>

#define TS_MAX 188

class media_data_get : public av_format_callback {
public:
    media_data_get() {};
    virtual ~media_data_get() {};

public:
    virtual int output_packet(MEDIA_PACKET_PTR pkt_ptr) {
        log_info_data((uint8_t*)pkt_ptr->data(), pkt_ptr->size(), pkt_ptr->dump().c_str());
        return 0;
    }
};

int main(int argn, char** argv) {
    unsigned char data[TS_MAX];
    mpegts_demux demux_obj;
    media_data_get cb;
    
    FILE* file_p;
    if (argn < 2) {
        log_errorf("please input ts name.\r\n");
        return 0;
    }

    Logger::get_instance()->set_filename("mpegts_demux.log");
    const char* file_name = argv[1];
    log_infof("input ts name:%s.\r\n", file_name);

    file_p = fopen(file_name, "r");
    fseek(file_p, 0L, SEEK_END); /* 定位到文件末尾 */
    size_t flen = ftell(file_p); /* 得到文件大小 */
    fseek(file_p, 0L, SEEK_SET); /* 定位到文件开头 */

    do {
        DATA_BUFFER_PTR data_ptr = std::make_shared<data_buffer>();
        fread(data, TS_MAX, 1, file_p);
        data_ptr->append_data((char*)data, TS_MAX);
        demux_obj.decode(data_ptr, &cb);
        flen -= TS_MAX;
    } while(flen > 0);

    log_infof("read file eof....");
    return 1;
}
