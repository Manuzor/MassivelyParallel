#pragma once

template<typename ArgsType>
class Event
{
public:
  using EventArgsType = ArgsType;
  using ListenerType = std::function<void(EventArgsType&)>;

  friend void Trigger(Event& event, EventArgsType& args)
  {
    if(event.m_bIsTriggering)
    {
      MP_ReportError("Event trigger cycles are not allowed.");
      return;
    }

    event.m_bIsTriggering = true;
    for (auto& listener : event.m_Listeners)
      if(listener)
        listener(args);
    event.m_bIsTriggering = false;
  }

  friend void Add(Event& event, ListenerType listener)
  {
    event.m_Listeners.push_back(listener);
  }

  friend bool Remove(Event& event, ListenerType listener)
  {
    MP_Assert(!event.m_bIsTriggering, "Cannot remove a listener while the event is triggering.");

    using namespace std;

    auto findResult = find(begin(event.m_Listeners), end(event.m_Listeners), listener);
    if(findResult == end(event.m_Listeners)) // Listener not found.
      return false;

    event.m_Listeners.erase(findResult);
    return true;
  }

private:
  bool m_bIsTriggering = false;
  std::vector<ListenerType> m_Listeners;
};
