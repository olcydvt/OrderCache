#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Order {
public:
    Order(
        const std::string &ordId,
        const std::string &secId,
        const std::string &side,
        const unsigned int qty,
        const std::string &user,
        const std::string &company)
        : m_orderId(ordId),
          m_securityId(secId),
          m_side(side),
          m_qty(qty),
          m_user(user),
          m_company(company) {
    }

    // consider order id and security id can be integer
    const std::string& orderId() const { return m_orderId; }
    const std::string& securityId() const { return m_securityId; }
    const std::string& side() const { return m_side; }
    const std::string& user() const { return m_user; }
    const std::string& company() const { return m_company; }
    unsigned int qty() const { return m_qty; }
    void qty(const unsigned int qty) { m_qty = qty; }

private:
    std::string m_orderId; // unique order id
    std::string m_securityId; // security identifier
    std::string m_side; // side of the order, eg Buy or Sell
    unsigned int m_qty; // qty for this order
    std::string m_user; // user name who owns this order
    std::string m_company; // company for user
};

class OrderCacheInterface {
public:

    // add order to the cache
    virtual void addOrder(Order order) = 0;

    // remove order with this unique order id from the cache
    virtual void cancelOrder(const std::string &orderId) = 0;

    // remove all orders in the cache for this user
    virtual void cancelOrdersForUser(const std::string &user) = 0;

    // remove all orders in the cache for this security with qty >= minQty
    virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) = 0;

    // return the total qty that can match for the security id
    virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;

    // return all orders in cache in a vector
    virtual std::vector<Order> getAllOrders() const = 0;
};

class OrderCache : public OrderCacheInterface {
public:
    OrderCache() {
        m_orders.reserve(1000000);
        m_orderIds.reserve(1000000);
        m_ordersBySecId.reserve(1000000);
    }

    void addOrder(Order order) override;

    void cancelOrder(const std::string &orderId) override;

    void cancelOrdersForUser(const std::string &user) override;

    void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override;

    unsigned int getMatchingSizeForSecurity(const std::string &securityId) override;

    std::vector<Order> getAllOrders() const override;

private:
    using userId = std::string;
    using orderId = std::string;
    using secId = std::string;
    using orderEntryIterator = std::vector<Order>::iterator;

    std::vector<Order> m_orders;
    std::unordered_map<std::string, std::vector<size_t> > m_ordersBySecId;

    struct FNV1aHash {
        size_t operator()(std::string_view key) const noexcept {
            constexpr size_t prime = 0x100000001B3;
            constexpr size_t offset = 0xCBF29CE484222325;
            size_t hash = offset;

            for (char c: key) {
                hash ^= static_cast<size_t>(c);
                hash *= prime;
            }
            return hash;
        }
    };

    std::unordered_set<std::string, FNV1aHash> m_orderIds;
};
