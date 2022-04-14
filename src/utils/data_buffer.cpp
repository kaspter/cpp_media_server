#include "logger.hpp"
#include "data_buffer.hpp"
#include <cstring>

data_buffer::data_buffer(size_t data_size) {
    buffer_      = new char[data_size + PRE_RESERVE_HEADER_SIZE];
    buffer_size_ = data_size;
    start_       = PRE_RESERVE_HEADER_SIZE;
    end_         = PRE_RESERVE_HEADER_SIZE;
    data_len_    = 0;
    memset(buffer_, 0, data_size);
}

data_buffer::data_buffer(const data_buffer& input) {
    buffer_ = NULL;
    this->operator=(input);
}

data_buffer& data_buffer::operator=(const data_buffer& input) {
    sent_flag_     = input.sent_flag_;
    dst_ip_        = input.dst_ip_;
    dst_port_      = input.dst_port_;
    // fixed memroy leak
    if (buffer_) delete [] buffer_;
    buffer_        = new char[input.buffer_size_ + PRE_RESERVE_HEADER_SIZE];
    buffer_size_   = input.buffer_size_;
    data_len_      = input.data_len_;
    start_         = input.start_;
    end_           = input.end_;
    memcpy(buffer_ + start_, input.buffer_ + start_, data_len_);
    // memcpy(buffer_, input.buffer_, data_len_);
    return *this;
}

data_buffer::~data_buffer() {
    if (buffer_) {
        delete[] buffer_;
    }
}

static int get_new_size(int new_len) {
    int ret = new_len;

    if (new_len <= 50*1024) {
        ret = 50*1024;
    } else if ((new_len > 50*1024) && (new_len <= 100*1024)) {
        ret = 100*1024;
    } else if ((new_len > 100*1024) && (new_len <= 200*1024)) {
        ret = 200*1024;
    } else if ((new_len > 200*1024) && (new_len <= 500*1024)) {
        ret = 500*1024;
    } else {
        ret = new_len;
    }

    return ret;
}

int data_buffer::append_data(const char* input_data, size_t input_len) {
    if (!input_data || !input_len) {
        return 0;
    }

    if ((size_t)end_ + input_len > (buffer_size_ - PRE_RESERVE_HEADER_SIZE)) {
        if (data_len_ + input_len >= (buffer_size_ - PRE_RESERVE_HEADER_SIZE)) {
            // 缓存空间不够
            int new_len = get_new_size(data_len_ + (int)input_len + EXTRA_LEN);
            char* new_buffer = new char[new_len];
            memcpy(new_buffer + PRE_RESERVE_HEADER_SIZE, buffer_ + start_, data_len_);
            memcpy(new_buffer + PRE_RESERVE_HEADER_SIZE + data_len_, input_data, input_len);
            delete[] buffer_;
            buffer_      = new_buffer;
            buffer_size_ = new_len;
            data_len_    += input_len;
            start_     = PRE_RESERVE_HEADER_SIZE;
            end_       = start_ + data_len_;
            return data_len_;
        }
        else { // 挪一挪start_, 还是可以放下的
            memmove(buffer_ + PRE_RESERVE_HEADER_SIZE, buffer_ + start_, data_len_);
            memcpy(buffer_ + PRE_RESERVE_HEADER_SIZE + data_len_, input_data, input_len);
            
            data_len_ += input_len;
            start_     = PRE_RESERVE_HEADER_SIZE;
            end_       = start_ + data_len_;
            return data_len_;
        }
    }
    else{ // 尾部空间够，直接加到尾部
        memcpy(buffer_ + end_, input_data, input_len);
        data_len_ += (int)input_len;
        end_ += (int)input_len;
        return data_len_;
    }
}

char* data_buffer::consume_data(int consume_len) {
    if (consume_len > data_len_) {
        log_errorf("consume_len:%lu, data_len_:%d", consume_len, data_len_);
        return nullptr;
    }
    // can be less than 0
    if (consume_len < 0) {
        // can't move to begin
        if ((start_ + consume_len) < 0) {
            return nullptr;
        }
    }
    start_    += consume_len;
    data_len_ -= consume_len;

    return buffer_ + start_;
}

void data_buffer::reset() {
    start_    = PRE_RESERVE_HEADER_SIZE;
    end_      = PRE_RESERVE_HEADER_SIZE;
    data_len_ = 0;
}

char* data_buffer::data() {
    return buffer_ + start_;
}

size_t data_buffer::data_len() {
    return data_len_;
}

bool data_buffer::require(size_t len) {
    return len <= data_len_;
}