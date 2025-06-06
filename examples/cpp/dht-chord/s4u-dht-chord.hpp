/* Copyright (c) 2016-2025. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef S4U_CHORD_HPP
#define S4U_CHORD_HPP
#include "simgrid/s4u.hpp"
#include <string>
#include <xbt/random.hpp>

namespace sg4 = simgrid::s4u;

constexpr double MAX_SIMULATION_TIME              = 1000;
constexpr double PERIODIC_STABILIZE_DELAY         = 20;
constexpr double PERIODIC_FIX_FINGERS_DELAY       = 120;
constexpr double PERIODIC_CHECK_PREDECESSOR_DELAY = 120;
constexpr double PERIODIC_LOOKUP_DELAY            = 10;
constexpr double SLEEP_DELAY                      = 4.9999;

/* Types of tasks exchanged between nodes. */
enum class MessageType {
  FIND_SUCCESSOR,
  FIND_SUCCESSOR_ANSWER,
  GET_PREDECESSOR,
  GET_PREDECESSOR_ANSWER,
  NOTIFY,
  SUCCESSOR_LEAVING,
  PREDECESSOR_LEAVING,
  PREDECESSOR_ALIVE,
  PREDECESSOR_ALIVE_ANSWER
};

class ChordMessage {
public:
  MessageType type;                                                                    // type of message
  std::string issuer_host_name = sg4::this_actor::get_host()->get_name();              // used for logging
  int request_id     = -1;            // id (used by some types of messages)
  int request_finger = 1;             // finger parameter (used by some types of messages)
  int answer_id      = -1;            // answer (used by some types of messages)
  sg4::Mailbox* answer_to      = nullptr;       // mailbox to send an answer to (if any)

  explicit ChordMessage(MessageType type) : type(type) {}

  static void destroy(void* message);
};

class Node {
  inline static int nb_bits_;
  inline static int nb_keys_;
  inline static int timeout_;

  int known_id_      = -1;
  double start_time_ = -1;
  double deadline_   = -1;
  bool joined_       = false;
  int id_;                           // my id
  int pred_id_ = -1;                 // predecessor id
  simgrid::xbt::random::XbtRandom random_; // random number generator for this node
  sg4::Mailbox* mailbox_;                  // my mailbox
  std::vector<int> fingers_;         // finger table,(fingers[0] is my successor)
  int next_finger_to_fix_;           // index of the next finger to fix in fix_fingers()

  static bool is_in_interval(int id, int start, int end);

public:
  static void set_parameters(int nb_bits, int nb_keys, int timeout);

  explicit Node(std::vector<std::string> args);
  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;
  void join(int known_id);
  void leave();
  void notifyAndQuit();

  void randomLookup();
  void setFinger(int finger_index, int id);
  void fixFingers();
  void printFingerTable();

  void setPredecessor(int predecessor_id);
  void checkPredecessor();
  int remoteGetPredecessor(int ask_to);
  int closestPrecedingFinger(int id);
  int findSuccessor(int id);
  int remoteFindSuccessor(int ask_to, int id);

  void notify(int predecessor_candidate_id);
  void remoteNotify(int notify_id, int predecessor_candidate_id) const;
  void stabilize();
  void handleMessage(ChordMessage* message);

  void operator()();
};

#endif
