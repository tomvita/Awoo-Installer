#pragma once
#include <string>
#include <switch.h>

class Gdrive {
  private:
    std::string m_id;
    std::string m_username;
    std::string m_password;

  public:
    template <typename T1, typename T2, typename T3>
    Gdrive(T1 &&id, T2 &&username, T3 &&password)
        : m_id(std::string(id)), m_username(std::string(username)), m_password(std::string(password)) {}
    ~Gdrive();

    Result Buffer(u8 *dst, off_t offset, size_t size);
};
