#include "input_force_feedback_named_pipe.h"

#ifdef WIN32
#include "named_pipe/named_pipe_win.h"
#endif

static og::Logger& logger = og::Logger::GetInstance();

class InputForceFeedbackNamedPipe::Impl {
 public:
  Impl(og::Hand hand, std::function<void(const ForceFeedbackCurlData&)> on_data_callback) : on_data_callback_(std::move(on_data_callback)) {
    const std::string pipe_name = R"(\\.\pipe\vrapplication\ffb\curl\)" + std::string(hand == og::kHandLeft ? "left" : "right");

#ifdef WIN32
    pipe_listener_ = std::make_unique<NamedPipeListener<ForceFeedbackCurlData>>(
        pipe_name,
        [&](const NamedPipeListenerEvent& event) {
          if (event.type == NamedPipeListenerEventType::ClientConnected)
            logger.Log(og::kLoggerLevel_Info, "Force feedback pipe connected for %s hand", hand == og::kHandLeft ? "left" : "right");
        },
        [&](const ForceFeedbackCurlData* data) { on_data_callback_(*data); });
#endif
  }

  void StartListening() {
#ifdef WIN32
    pipe_listener_->StartListening();
#endif
  }

 private:
  std::function<void(const ForceFeedbackCurlData&)> on_data_callback_;

#ifdef WIN32
  std::unique_ptr<INamedPipeListener> pipe_listener_;
#endif
};

InputForceFeedbackNamedPipe::InputForceFeedbackNamedPipe(og::Hand hand, std::function<void(const ForceFeedbackCurlData&)> on_data_callback)
    : pImpl_(std::make_unique<Impl>(hand, std::move(on_data_callback))){};

void InputForceFeedbackNamedPipe::StartListener() {
  pImpl_->StartListening();
}

InputForceFeedbackNamedPipe::~InputForceFeedbackNamedPipe() = default;