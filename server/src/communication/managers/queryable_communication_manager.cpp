/**
 * A communication manager that manages a resources that can be queried and writes to/from the device in strings using an encoding scheme. Ie.
 * Bluetooth, Serial, etc. But **NOT** ipc methods like named pipes.
 */

#include "communication_manager.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

CommunicationManager::CommunicationManager(
    std::unique_ptr<ICommunicationService> communication_service, std::unique_ptr<IEncodingService> encoding_service) {
  communication_service_ = std::move(communication_service);
  encoding_service_ = std::move(encoding_service);
}

void CommunicationManager::BeginListener(std::function<void(og::Input)> callback) {
  if (thread_active_) {
    logger.Log(kLoggerLevel_Warning, "Did not start communication listener as the listener was already active.");
    return;
  }

  thread_active_ = true;

  communication_thread_ = std::thread(&CommunicationManager::CommunicationThread, this);
}

void CommunicationManager::CommunicationThread() {
  while (thread_active_) {
    std::string recevied_string;
    communication_service_->ReceiveNextPacket(recevied_string);

    const Input input = encoding_service_->DecodePacket(recevied_string);
    callback_(input);

    // now write information we might have
    queued_write_string += "\n";
    communication_service_->RawWrite(queued_write_string);

    if (queued_write_string != "\n")  // log any data we've sent to the device
      logger.Log(kLoggerLevel_Info, "Wrote data to device: %s", queued_write_string.c_str());

    queued_write_string.clear();
  }
}

CommunicationManager::~CommunicationManager() {
  if (thread_active_.exchange(false)) {
    logger.Log(kLoggerLevel_Info, "Attempting to cleanup communication thread...");
    communication_thread_.join();

    logger.Log(kLoggerLevel_Info, "Successfully cleaned up communication thread");
  }
}