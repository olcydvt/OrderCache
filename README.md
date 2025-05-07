# Order Cache Implementation

## Table of Contents
- [Overview](#overview)
- [Objectives](#objectives)
- [Development Guidelines](#development-guidelines)
  - [Order Class](#order-class)
  - [OrderCacheInterface Implementation](#ordercacheinterface-implementation)
  - [Testing](#testing)
  - [Submission](#submission)
  - [Additional Information](#additional-information)
- [Order Matching Rules](#order-matching-rules-for-getmatchingsizeforsecurity)
- [Order Matching Examples](#order-matching-examples)
  - [Example 1](#example-1)
  - [Example 2](#example-2)
  - [Example 3](#example-3)
- [Performance](#performance)
- [Error Handling](#error-handling)
- [Conclusion](#conclusion)

## Overview

 Implementation an in-memory cache for order objects, facilitating operations like adding, removing, and matching buy and sell orders. This simulates a basic trading platform.

- An "order" is a request to buy or sell a financial security (eg. bond, stock, commodity, etc.).
- Each order is uniquely identified by an order id.
- Each security has a different security id.
- Order matching occurs for orders with the same security id, different side (buy or sell), and different company (company of person who requested the order).

## Features
- Implements an `OrderCache` class, conforming to the `OrderCacheInterface` in `OrderCache.h`.
- Supports operations on financial security orders, uniquely identified by order IDs.
- Ensures order matching rules are followed (detailed below).

## Guidelines
### OrderCacheInterface Implementation

  - `addOrder()`
  - `cancelOrder()`
  - `cancelOrdersForUser()`
  - `cancelOrdersForSecIdWithMinimumQty()`
  - `getMatchingSizeForSecurity()`
  - `getAllOrders()`

### Testing
- Use the provided unit test in `OrderCacheTest.cpp`.

### Additional Information

- Uses up to C++17 standards
-  platform-agnostic. Code can work on Linux and Microsoft Windows.
- Single-threaded

## Order Matching Rules for getMatchingSizeForSecurity()

- Matches orders with the same security ID and opposite types (Buy/Sell).
- An order can match against multiple orders of the opposite type.
- A buy order can match against multiple sell orders, and vice versa.
- Any order quantity already allocated to a match cannot be reused as a match against a different order.
- Some orders may not match entirely or at all.
- Users in the same company cannot match against each other.
- Full details and examples below.

## Order Matching Examples

Three detailed examples are included to guide you through expected functionality and order matching scenarios.

### Example 1

Orders in cache:
```
OrdId1 SecId1 Buy  1000 User1 CompanyA
OrdId2 SecId2 Sell 3000 User2 CompanyB
OrdId3 SecId1 Sell  500 User3 CompanyA
OrdId4 SecId2 Buy   600 User4 CompanyC
OrdId5 SecId2 Buy   100 User5 CompanyB
OrdId6 SecId3 Buy  1000 User6 CompanyD
OrdId7 SecId2 Buy  2000 User7 CompanyE
OrdId8 SecId2 Sell 5000 User8 CompanyE
```

Total quantity matching for securities:
```
SecId1 0
SecId2 2700
SecId3 0
```

**Explanation:**
- **SecId1**
  - SecId1 has 1 Buy order and 1 Sell order
  - Both orders are for users in CompanyA so they are not allowed to match
  - There are no matches for SecId1
- **SecId2**
  - OrdId2 matches quantity 600 against OrdId4
  - OrdId2 matches quantity 2000 against OrdId7
  - OrdId2 has a total matched quantity of 2600
  - OrdId8 matches quantity 100 against OrdId5 only
  - OrdId8 has a remaining qty of 4900
  - OrdId4 had its quantity fully allocated to match OrdId2
  - No remaining qty on OrdId4 for the remaining 4900 of OrdId8
  - Total quantity matched for SecId2 is 2700. (2600 + 100)
  - Note: there are other combinations of matches among the orders which would lead to the same result of 2700 total qty matching
- **SecId3** has only one Buy order, no other orders to match against

### Example 2

Orders in cache:
```
OrdId1  SecId1 Sell  100 User10 Company2
OrdId2  SecId3 Sell  200 User8  Company2
OrdId3  SecId1 Buy   300 User13 Company2
OrdId4  SecId2 Sell  400 User12 Company2
OrdId5  SecId3 Sell  500 User7  Company2
OrdId6  SecId3 Buy   600 User3  Company1
OrdId7  SecId1 Sell  700 User10 Company2
OrdId8  SecId1 Sell  800 User2  Company1
OrdId9  SecId2 Buy   900 User6  Company2
OrdId10 SecId2 Sell 1000 User5  Company1
OrdId11 SecId1 Sell 1100 User13 Company2
OrdId12 SecId2 Buy  1200 User9  Company2
OrdId13 SecId1 Sell 1300 User1  Company1
```

Total quantity matching for securities:
```
SecId1 300
SecId2 1000
SecId3 600
```

**Explanation:**
- **SecId1**
  - Buy order OrdId3 (300 units) can match with Sell order OrdId8 (800 units) because they are from different companies (Company2 and Company1).
  - OrdId3 (300 units) cannot match with OrdId1, OrdId7, or OrdId11 as all belong to the same company (Company2).
  - Total quantity matched for SecId1 is 300.
- **SecId2**
  - Buy order OrdId12 (1200 units) can match with Sell order OrdId10 (1000 units) from different companies (Company2 and Company1).
  - OrdId9 (900 units) cannot match with OrdId4 (400 units) as they belong to the same company (Company2).
  - Total quantity matched for SecId2 is 1000.
- **SecId3**
  - Buy order OrdId6 (600 units) can match with Sell orders OrdId2 (200 units) and OrdId5 (500 units) for a total of 600 units.
  - OrdId6 (600 units) cannot match with OrdId1 as it is from the same company (Company1).
  - Total quantity matched for SecId3 is 600.

### Example 3

Orders in cache:
```
OrdId1  SecId3 Sell  100 User1 Company1
OrdId2  SecId3 Sell  200 User3 Company2
OrdId3  SecId1 Buy   300 User2 Company1
OrdId4  SecId3 Sell  400 User5 Company2
OrdId5  SecId2 Sell  500 User2 Company1
OrdId6  SecId2 Buy   600 User3 Company2
OrdId7  SecId2 Sell  700 User1 Company1
OrdId8  SecId1 Sell  800 User2 Company1
OrdId9  SecId1 Buy   900 User5 Company2
OrdId10 SecId1 Sell 1000 User1 Company1
OrdId11 SecId2 Sell 1100 User6 Company2
```

Total quantity matching for securities:
```
SecId1 900
SecId2 600
SecId3 0
```

## Performance
- Performance is measured in Normalized Compute Units (NCUs).
- solution is able to process (adding and matching) up to 1 million orders in 1,500 NCUs or less.

## Error Handling
- Missing or malformed fields in order data (e.g., empty strings, invalid values).
- Operations involving non-existent or duplicate identifiers.
- Invalid use cases such as zero or negative quantities.
