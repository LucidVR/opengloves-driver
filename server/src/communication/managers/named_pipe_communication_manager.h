// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include "communication_manager.h"

#include "opengloves_interface.h"

#include <memory>

class NamedPipeCommunicationManager : public ICommunicationManager {
 public:
  NamedPipeCommunicationManager(og::Hand hand, std::function<void()> on_client_connected);

  void BeginListener(std::function<void(const og::Input&)> callback) override;

  void WriteOutput(const og::Output& output) override;

  ~NamedPipeCommunicationManager() override;

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  std::function<void()> on_client_connected_;
  std::function<void(const og::Input&)> on_data_callback_;
};