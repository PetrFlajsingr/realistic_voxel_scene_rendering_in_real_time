//
// Created by petr on 9/24/20.
//

#include "Subscription.h"

pf::events::Subscription::Subscription(pf::events::Subscription::Unsubscriber &&unsubscriber)
    : unsub(unsubscriber) {}

void pf::events::Subscription::unsubscribe() { unsub(); }
pf::events::Subscription::Subscription(pf::events::Subscription &&other) noexcept
    : unsub(std::move(other.unsub)) {
  other.unsub = [] {};
}
pf::events::Subscription &
pf::events::Subscription::operator=(pf::events::Subscription &&other) noexcept {
  unsub = other.unsub;
  other.unsub = [] {};
  return *this;
}
