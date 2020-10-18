//
// Created by petr on 9/24/20.
//

#include "Subscription.h"

pf::events::Subscription::Subscription(pf::events::Subscription::Unsubscriber &&unsubscriber)
    : unsub(unsubscriber) {}

void pf::events::Subscription::unsubscribe() { unsub(); }
