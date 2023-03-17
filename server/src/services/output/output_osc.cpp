// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "output_osc.h"

#include <sstream>

#define MINIOSC_IMPLEMENTATION
extern "C" {
#include "miniosc/miniosc.h"
};

class OutputOSCServer::Impl {
 public:
  Impl() {
    osc_ = minioscInit(9000, 9005, "127.0.0.1", 0);
  }

  void Send(og::Hand hand, const og::InputPeripheralData& input) {
    mobundle bundle = {0};

    const char* hand_string = hand == og::kHandLeft ? "left" : "right";

    char address[100];

    sprintf_s(address, "/avatar/parameters/%s%sSplay", hand_string, "Thumb");
    minioscBundle(&bundle, address, ",f", input.splay[0]);
    sprintf_s(address, "/avatar/parameters/%s%sSplay", hand_string, "Index");
    minioscBundle(&bundle, address, ",f", input.splay[1]);
    sprintf_s(address, "/avatar/parameters/%s%sSplay", hand_string, "Middle");
    minioscBundle(&bundle, address, ",f", input.splay[2]);
    sprintf_s(address, "/avatar/parameters/%s%sSplay", hand_string, "Ring");
    minioscBundle(&bundle, address, ",f", input.splay[3]);
    sprintf_s(address, "/avatar/parameters/%s%sSplay", hand_string, "Pinky");
    minioscBundle(&bundle, address, ",f", input.splay[4]);


    minioscSendBundle(osc_, &bundle);
  }

  void Stop() {
    minioscClose(osc_);
  }

 private:
  miniosc* osc_;
};

OutputOSCServer::OutputOSCServer() {
  pImpl_ = std::make_unique<Impl>();
}

void OutputOSCServer::Send(og::Hand hand, const og::InputPeripheralData& input) {
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

  //send once every 50 milliseconds
  if(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_send_time_).count() < 50) return;

  pImpl_->Send(hand, input);
  last_send_time_ = now;
}

void OutputOSCServer::Stop() {
  pImpl_->Stop();
}

OutputOSCServer::~OutputOSCServer() {
  Stop();
}