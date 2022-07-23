#pragma once

#include <cstddef>

class StringBuffer {
private:
  char *start_;
  const char *end_;

public:
  StringBuffer(char *buf, size_t len);

  bool is_empty() const;

  void drop_leading_control_chars();
  void drop_leading_non_control_chars();
  void drop_line();
  void drop_token();

  bool parse_token(const char *token, size_t token_length);
  void parse_float(float &out);
  void parse_float3(float out[3]);
};
