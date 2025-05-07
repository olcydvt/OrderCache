// Todo: your implementation of the OrderCache...
#include "OrderCache.h"
#include <algorithm>

void OrderCache::addOrder(Order order) {
    auto [it, inserted] = m_orderIds.emplace(order.orderId());
    if (!inserted) {
        return;
    }

    const std::string &side = order.side();
    if (order.qty() <= 0 ||
        order.securityId().empty() ||
        order.user().empty() ||
        order.company().empty() ||
        order.orderId().empty() ||
        (side != "Buy" && side != "Sell")) {
        m_orderIds.erase(it);
        return;
    }

    const std::string &secId = order.securityId();
    m_ordersBySecId[secId].push_back(m_orders.size());
    m_orders.emplace_back(std::move(order));
}

void OrderCache::cancelOrder(const std::string &orderId) {
    auto orderItr = std::find_if(m_orders.begin(), m_orders.end(),
                                 [&orderId](const Order &o) { return o.orderId() == orderId; });

    if (orderItr == m_orders.end()) {
        return; // Order not found
    }
    m_orderIds.erase(orderId);
    size_t orderPos = orderItr - m_orders.begin();
    const std::string &secId = orderItr->securityId();

    auto &secVec = m_ordersBySecId[secId];
    secVec.erase(std::remove(secVec.begin(), secVec.end(), orderPos), secVec.end());

    if (orderPos != m_orders.size() - 1) {
        std::iter_swap(orderItr, m_orders.end() - 1);

        size_t swappedOrderPos = orderPos;
        const std::string &swappedSecId = m_orders[orderPos].securityId();
        auto &swappedSecIndices = m_ordersBySecId[swappedSecId];

        auto swappedIt = std::find(swappedSecIndices.begin(),
                                   swappedSecIndices.end(),
                                   m_orders.size() - 1);
        *swappedIt = swappedOrderPos;
    }

    m_orders.pop_back();
}

void OrderCache::cancelOrdersForUser(const std::string &user) {
    auto it = m_orders.begin();
    while (it != m_orders.end()) {
        if (it->user() == user) {
            const size_t orderPos = it - m_orders.begin();
            const std::string &secId = it->securityId();
            m_orderIds.erase(it->orderId());
            auto &secOrders = m_ordersBySecId[secId];
            secOrders.erase(
                std::remove(secOrders.begin(), secOrders.end(), orderPos),
                secOrders.end()
            );

            if (orderPos != m_orders.size() - 1) {
                std::iter_swap(it, m_orders.end() - 1);

                const size_t swappedPos = orderPos;
                const std::string &swappedSecId = m_orders[swappedPos].securityId();
                auto &swappedSecOrders = m_ordersBySecId[swappedSecId];

                auto swappedIt = std::find(swappedSecOrders.begin(),
                                           swappedSecOrders.end(),
                                           m_orders.size() - 1);
                *swappedIt = swappedPos;
            } else {
                m_orders.pop_back();
                return;
            }
            m_orders.pop_back();
        } else {
            ++it;
        }
    }
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) {
    if (minQty == 0) {
        return;
    }
    auto secIt = m_ordersBySecId.find(securityId);
    if (secIt == m_ordersBySecId.end()) return;

    auto &indices = secIt->second;
    indices.erase(
        std::remove_if(indices.begin(), indices.end(),
                       [&](size_t idx) {
                           Order &order = m_orders[idx];
                           if (order.qty() >= minQty) {
                               m_orderIds.erase(order.orderId());

                               if (idx != m_orders.size() - 1) {
                                   std::swap(order, m_orders.back());

                                   const std::string &swappedSecId = m_orders[idx].securityId();
                                   auto &swappedIndices = m_ordersBySecId[swappedSecId];
                                   auto swappedIt = std::find(swappedIndices.begin(),
                                                              swappedIndices.end(),
                                                              m_orders.size() - 1);
                                   *swappedIt = idx;
                               }

                               m_orders.pop_back();
                               return true;
                           }
                           return false;
                       }),
        indices.end()
    );

    if (indices.empty()) {
        m_ordersBySecId.erase(secIt);
    }
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string &securityId) {
    auto secIt = m_ordersBySecId.find(securityId);
    if (secIt == m_ordersBySecId.end()) return 0;

    using OrderRef = std::reference_wrapper<Order>;
    std::vector<OrderRef> buys, sells;
    buys.reserve(secIt->second.size());
    sells.reserve(secIt->second.size());

    for (size_t idx: secIt->second) {
        Order &order = m_orders[idx];
        if (order.side() == "Buy") {
            buys.emplace_back(order);
        } else {
            sells.emplace_back(order);
        }
    }

    unsigned int totalMatched = 0;
    for (OrderRef buyRef: buys) {
        Order &buy = buyRef.get();
        if (buy.qty() == 0) continue;

        for (OrderRef sellRef: sells) {
            Order &sell = sellRef.get();
            if (sell.qty() == 0 || buy.company() == sell.company()) continue;

            unsigned int matchQty = std::min(buy.qty(), sell.qty());
            totalMatched += matchQty;
            buy.qty(buy.qty() - matchQty);
            sell.qty(sell.qty() - matchQty);

            if (buy.qty() == 0) break;
        }
    }
    return totalMatched;
}

std::vector<Order> OrderCache::getAllOrders() const {
    return m_orders;
}
