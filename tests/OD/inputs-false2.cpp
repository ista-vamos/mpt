#include <cassert>

#include "monitor.h"
#include "events.h"
#include "inputs.h"

int x;
uint64_t addr = (uint64_t)&x;

#define NUM_STREAMS 2
#define MAX_NUM_EVS 8
const size_t lens[] = {8, 6};
TraceEvent streams[][MAX_NUM_EVS] = {
    {Event_InputL(1, addr, 1), Event_InputL(2, addr, 2), Event_InputL(3, addr, 3),

     Event_OutputL(1, addr, 1), Event_OutputL(2, addr, 2), Event_OutputL(3, addr, 3),

     Event_OutputL(3, addr, 4), Event_Write(4, addr, 4)},
    {
        Event_InputL(1, addr, 1),
        Event_InputL(2, addr, 2),
        Event_InputL(3, addr, 3),

        Event_OutputL(1, addr, 1),
        Event_OutputL(2, addr, 2),
        Event_OutputL(3, addr, 3),
    },
};

bool InputStream::hasEvent() const {
  const size_t pos = data<const size_t>(1);
  const size_t len = data<const size_t>(2);
  return pos < len;
}

bool InputStream::isDone() const {
  const size_t pos = data<const size_t>(1);
  const size_t len = data<const size_t>(2);
  return pos >= len;
}

Event *InputStream::getEvent() {
  assert(hasEvent() && "getEvent() when there is no event");

  TraceEvent *events = data<TraceEvent *>(0);
  size_t &pos = data<size_t>(1);

  return &events[pos++];
}

Inputs::Inputs(char **, size_t) {
  size_t &returned = data<size_t>(0);
  returned = 0;
}

bool Inputs::done() const {
  const size_t returned = data<const size_t>(0);
  return returned >= NUM_STREAMS;
}

InputStream *Inputs::getNewInputStream() {
  size_t &returned = data<size_t>(0);
  if (returned >= NUM_STREAMS)
    return nullptr;

  auto *stream = new InputStream(_streams.size());
  _streams.emplace_back(stream);

  stream->data<TraceEvent *>(0) = streams[returned];
  stream->data<size_t>(1) = 0;
  stream->data<size_t>(2) = lens[returned];

  ++returned;

  return stream;
}
