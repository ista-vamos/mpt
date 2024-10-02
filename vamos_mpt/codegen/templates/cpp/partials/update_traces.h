template <typename WorkbagT, typename TracesT, typename StreamsT>
void update_traces(Inputs &inputs, WorkbagT &workbag, TracesT &traces,
                   StreamsT &online_traces) {
  // get new input streams
  if (auto *stream = inputs.getNewInputStream()) {
    // std::cout << "NEW stream " << stream->id() << "\n";
    auto *trace = new Trace<TraceEvent>(stream->id());
    traces.emplace_back(trace);
    stream->setTrace(trace);
    online_traces.push_back(stream);

    add_new_cfgs(workbag, traces, trace);
  }

  // copy events from input streams to traces
  std::set<InputStream *> remove_online_traces;
  for (auto *stream : online_traces) {
    if (stream->hasEvent()) {
      auto *event = static_cast<TraceEvent *>(stream->getEvent());
      auto *trace = static_cast<Trace<TraceEvent> *>(stream->trace());
      trace->append(event);

      // std::cout << "[Stream " << stream->id() << "] event: " << *event << "\n";

      if (stream->isDone()) {
        // std::cout << "Stream " << stream->id() << " DONE\n";
        remove_online_traces.insert(stream);
        trace->append(TraceEvent(Kind::END)); //, trace->size()));
        trace->setDone();
      }
    }
  }
  // remove finished traces
  if (remove_online_traces.size() > 0) {
    std::vector<InputStream *> tmp;
    tmp.reserve(online_traces.size() - remove_online_traces.size());
    for (auto *stream : online_traces) {
      if (remove_online_traces.count(stream) == 0)
        tmp.push_back(stream);
    }
    online_traces.swap(tmp);
  }
}
