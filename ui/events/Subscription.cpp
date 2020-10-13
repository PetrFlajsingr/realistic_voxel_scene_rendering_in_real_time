//
// Created by petr on 9/24/20.
//

#include "Subscription.h"

pf::events::Subscription::Subscription(pf::events::Subscription::Unsubscriber &&unsub)
    : unsub(unsub) {}

void pf::events::Subscription::unsubscribe() { unsub(); }
