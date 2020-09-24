//
// Created by petr on 9/24/20.
//

#include "subscription.h"

pf::events::subscription::subscription(pf::events::subscription::unsubscriber &&unsub)
    : unsub(unsub) {}

void pf::events::subscription::unsubscribe() {
  unsub();
}
