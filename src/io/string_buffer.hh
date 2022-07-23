#pragma once

#include "../fast_float/fast_float.h"
#include <cstddef>

class StringBuffer {
private:
  char *start_;
  const char *end_;

public:
  StringBuffer(char *buf, size_t len) {
    start_ = buf;
    end_ = start_ + len;
  }

  bool is_empty() const { return start_ == end_; }

  void drop_leading_control_chars() {
    while ((start_ < end_) && (*start_) <= ' ') {
      start_++;
    }
  }

  void drop_leading_non_control_chars() {
    while ((start_ < end_) && (*start_) > ' ') {
      start_++;
    }
  }

  void drop_line() {
    while (start_ < end_ && *start_ != '\n') {
      start_++;
    }
  }

  bool parse_token(const char *token, size_t token_length) {
    drop_leading_control_chars();
    if (end_ - start_ < token_length + 1) {
      return false;
    }
    if (memcmp(start_, token, token_length) != 0) {
      return false;
    }
    if (start_[token_length] > ' ') {
      return false;
    }
    start_ += token_length + 1;
    return true;
  }

  void drop_token() {
    drop_leading_non_control_chars();
    drop_leading_control_chars();
  }

  void parse_float(float &out) {
    drop_leading_control_chars();
    /* Skip '+' */
    if (start_ < end_ && *start_ == '+') {
      start_++;
    }
    auto res = fast_float::from_chars(start_, end_, out);
    if (res.ec == std::errc::invalid_argument ||
        res.ec == std::errc::result_out_of_range) {
      out = 0.0f;
    }
    start_ = const_cast<char *>(res.ptr);
  }

  void parse_float3(float out[3]) {
    for (int i = 0; i < 3; i++) {
      parse_float(out[i]);
    }
  }
};
