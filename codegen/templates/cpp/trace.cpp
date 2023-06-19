#include <cassert>

#include "events.h"
#include "monitor.h"
#include "mstring.h"

#ifdef DBG
#include <iomanip>
#include <iostream>

static const char *color_green = "\033[0;32m";
static const char *color_red = "\033[0;31m";
static const char *color_reset = "\033[0m";

std::ostream &operator<<(std::ostream &s, const TraceEvent &ev) {
  s << "TraceEvent(" << color_green << std::setw(7) << std::left
    << kindToStr((Kind)ev.kind()) << color_reset << ", " << color_red
    << std::setw(2) << std::right << ev.id() << color_reset;
  const auto &params = paramsToStr((Kind)ev.kind());
  if (!params.empty()) {
    s << ", " << params;
  }
  << ")";

  return s;
}

#endif
