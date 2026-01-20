
#pragma once
#include <functional>
#include <string>
using result_cb_t = std::function<void(std::string, std::string)>;
using onconfirm_generator_t = std::function<std::function<void()>(result_cb_t)>;
