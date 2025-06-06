/* Copyright (c) 2007-2025. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_KERNEL_ACTIVITY_SLEEP_HPP
#define SIMGRID_KERNEL_ACTIVITY_SLEEP_HPP

#include "src/kernel/activity/ActivityImpl.hpp"

namespace simgrid::kernel::activity {

class XBT_PUBLIC SleepImpl : public ActivityImpl_T<SleepImpl> {
  sg_host_t host_  = nullptr;
  double duration_ = 0;

public:
  SleepImpl& set_host(s4u::Host* host);
  SleepImpl& set_duration(double duration);
  void finish() override;
  SleepImpl* start();
};
} // namespace simgrid::kernel::activity

#endif
