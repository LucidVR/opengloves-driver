#include "lucidgloves_named_pipe_discovery.h"

#include <regex>

#include "named_pipe/named_pipe_win.h"

class LucidglovesNamedPipeDiscovery::Impl {
 public:
  Impl(bool is_right_hand, std::function<void()> on_client_connected_callback, std::function<void(const og::InputData&)> on_data_callback)
      : on_client_connected_callback_(std::move(on_client_connected_callback)), on_data_callback_(std::move(on_data_callback)) {
    std::string base_name = R"(\\.\pipe\vrapplication\input\glove\$version\)" + std::string(is_right_hand ? "right" : "left");

    named_pipes_.emplace_back(std::make_unique<NamedPipeListener<og::InputData>>(
        std::regex_replace(base_name, std::regex("\\$version"), "v1"),
        [&](const NamedPipeListenerEvent& event) {
          switch (event.type) {
            case NamedPipeListenerEventType::ClientConnected:
              on_client_connected_callback_();
              break;
          }
        },
        [&](og::InputData* data) {

        }));
  };

 private:
  std::vector<std::unique_ptr<INamedPipeListener>> named_pipes_;
  std::function<void()> on_client_connected_callback_;
  std::function<void(const og::InputData&)> on_data_callback_;
};

LucidglovesNamedPipeDiscovery::LucidglovesNamedPipeDiscovery() {}

void LucidglovesNamedPipeDiscovery::StartDiscovery(std::function<void(std::unique_ptr<og::Device>)> callback) {}

void LucidglovesNamedPipeDiscovery::StopDiscovery() {}

LucidglovesNamedPipeDiscovery::~LucidglovesNamedPipeDiscovery() {
  StopDiscovery();
}