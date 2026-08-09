#pragma once

class OtaService {
public:
  void begin(const char* hostname);
  void handle();

private:
  bool configured = false;
};
