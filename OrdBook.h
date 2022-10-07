#pragma once

#include "Defn.h"
#include "OrdEvent.h"
#include "Order.h"

#include <list>
#include <functional>
#include <map>
#include <string>
#include <cassert>
#include <iostream>

class PriceLevel
{
public:
	using OrdList = std::list<Order*>;

	PriceLevel(const TPrice& px) : m_px(px) {}

	inline void insertOrder(Order* pOrd) {
		assert(m_px == pOrd->px());

		m_orders.emplace_back(pOrd);
	}

	inline void removeOrder(OrdList::iterator itOrd) {
		m_orders.erase(itOrd);
	}

	inline bool isEmpty() const { return m_orders.empty(); }

	inline OrdList::const_iterator beginOrders() const { return m_orders.begin(); }
	inline OrdList::iterator beginOrders() { return m_orders.begin(); }
	inline OrdList::const_iterator endOrders() const { return m_orders.end(); }
	inline const OrdList& orders() const { return m_orders; }
	inline OrdList::iterator findOrder(const TOrdId& ordId)
	{
		OrdList::iterator it(m_orders.begin());
		OrdList::const_iterator	itE(m_orders.end());

		for (; it != itE; ++it) {
			if ((*it)->ordId() == ordId) {
				return it;
			}
		}
		return it;
	}

	inline Order* frontOrder() { return m_orders.front(); }
	inline void popFrontOrder() { m_orders.pop_front(); }

	inline const TPrice& px() const { return m_px; }
	inline const TQty vol() const
	{
		TQty	vol(0);

		for (auto p : m_orders) {
			vol += p->qtyOutstanding();
		}
		return vol;
	}

protected:
	TPrice		m_px;
	OrdList		m_orders;
};

class OrdBook
{
public:
	using TBids = std::map< TPrice, PriceLevel, std::greater<TPrice> >;
	using TAsks = std::map<TPrice, PriceLevel>;

	struct OrdEventsResponse
	{
		Order* order;
		std::list<OrdEvent*>	ordEvents;
	};

public:
	OrdBook() : 
		m_mktAsk(TPrice(0)),
		m_mktBid(TPrice(0))
	{}

	inline PriceLevel& mktAsk() { return m_mktAsk; }
	inline PriceLevel& mktBid() { return m_mktBid; }
	inline const PriceLevel& mktAsk() const { return m_mktAsk; }
	inline const PriceLevel& mktBid() const { return m_mktBid; }

	inline PriceLevel& findOrCreateLimitAsk(const TPrice& px) {
		auto pr = m_asks.emplace(std::make_pair(px, PriceLevel(px)));
		return pr.first->second;
	}
	inline PriceLevel& findOrCreateLimitBid(const TPrice& px) {
		auto pr = m_bids.emplace(std::make_pair(px, PriceLevel(px)));
		return pr.first->second;
	}
	inline TAsks::iterator findLimitAsk(const TPrice& px) {
		return m_asks.find(px);
	}
	inline TBids::iterator findLimitBid(const TPrice& px) {
		return m_bids.find(px);
	}

	inline TAsks::const_iterator beginLimitAsks() const { return m_asks.begin(); }
	inline TAsks::iterator beginLimitAsks() { return m_asks.begin(); }
	inline TBids::const_iterator beginLimitBids() const { return m_bids.begin(); }
	inline TBids::iterator beginLimitBids() { return m_bids.begin(); }

	inline TAsks::const_iterator endLimitAsks() const { return m_asks.end(); }
	inline TBids::const_iterator endLimitBids() const { return m_bids.end(); }

	inline const PriceLevel& bestLimitAsk() const { return m_asks.begin()->second; }
	inline PriceLevel& bestLimitAsk() { return m_asks.begin()->second; }
	inline const PriceLevel& bestLimitBid() const { return m_bids.begin()->second; }
	inline PriceLevel& bestLimitBid() { return m_bids.begin()->second; }
	inline bool hasLimitAsk() const { return !m_asks.empty(); }
	inline bool hasLimitBid() const { return !m_bids.empty(); }
	inline void popBestLimitAsk() { m_asks.erase(m_asks.begin()); }
	inline void popBestLimitBid() { m_bids.erase(m_bids.begin()); }

	inline void removeLimitAsk(TAsks::iterator itAsk) { m_asks.erase(itAsk); }
	inline void removeLimitBid(TBids::iterator itBid) { m_bids.erase(itBid); }

	inline void dump()
	{
		std::cout << "ASK(0) | " << m_mktAsk.vol() << " | ";

		for (auto* p : m_mktAsk.orders()) {
			p->dumpOrder();
		}
		std::cout << std::endl;

		if (hasLimitAsk()) {
			TAsks::const_iterator	itAsk(endLimitAsks());
			TAsks::const_iterator	itAskB(beginLimitAsks());

			do {
				--itAsk;
				std::cout << "ASK(" << itAsk->first << ") | ";

				for (auto* pOrd : itAsk->second.orders()) {
					pOrd->dumpOrder();
				}
				std::cout << std::endl;
			} while (itAsk != itAskB);
		}

		std::cout << "BID(0) | " << m_mktBid.vol() << " | ";

		for (auto* p : m_mktBid.orders()) {
			p->dumpOrder();
		}
		std::cout << std::endl;

		for (auto pr : m_bids) {
			std::cout << "BID(" << pr.first << ") | ";

			for (auto* pOrd : pr.second.orders()) {
				pOrd->dumpOrder();
			}
			std::cout << std::endl;
		}
	}

protected:
	PriceLevel		m_mktAsk;
	PriceLevel		m_mktBid;
	TAsks			m_asks;
	TBids			m_bids;
};