#ifndef EXECUTOR_COMMAND_LIST
#define EXECUTOR_COMMAND_LIST

enum class ExectorCommand
{
  SHUT_DOWN = 0,
  SEND_TO_ROBOT = 1,
  SEND_TO_RCA = 2,
  SEND_TO_SENSOR = 3,
  RECV_FROM_SENSOR = 4,

  INVALID = -1
};

#endif