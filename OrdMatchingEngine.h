// OrdMatchingEngine.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

// TODO: Reference additional headers your program requires here.
#include "Defn.h"
#include "Order.h"
#include "OrdBook.h"

#include <memory>
#include <numeric>
#include <unordered_map>
#include <queue>

class OrdME
{
public:
	class Callback
	{
	public:
		virtual void onNew(Order* order, NewOrdEvent* event) = 0;
		virtual void onNewRej(Order* order, NewRejOrdEvent* event) = 0;
		virtual void onNewAck(Order* order, NewAckOrdEvent* event) = 0;
		virtual void onCan(Order* order, CanOrdEvent* event) = 0;
		virtual void onCanRej(Order* order, CanRejOrdEvent* event) = 0;
		virtual void onCanAck(Order* order, CanAckOrdEvent* event) = 0;
		virtual void onExec(Order* order, Execution* event) = 0;
		virtual void onExpiry(Order* order, Expired* event) = 0;
	};

public:
	OrdME() {}
	virtual ~OrdME() {}

	bool registerClient(const TClientId& clientId, Callback* callback)
	{
		auto tup = m_clientInfos.emplace(std::make_pair(clientId, ClientInfo(callback)));
		return tup.second;
	}

	void submitNewOrder(std::unique_ptr<Order> upOrder);
	void submitCanOrder(const TClientId& clientId, const TOrdId& orderId);

	inline void dumpOrdBook() {
		m_ordBook.dump();
	}

protected:
	struct ClientInfo {
		Callback* pCallback;
		TOrdId	nextOrdId;
		std::unordered_map< TOrdId, std::unique_ptr<Order> >	orders;

		ClientInfo(Callback* cb) : pCallback(cb), nextOrdId(0) {}
	};

	struct OrdEventResponse
	{
		Order* order;
		OrdEvent*	ordEvent;
	};

	using ClientInfoMap = std::unordered_map<TClientId, ClientInfo>;

	void handleEvents(std::list<OrdEventResponse>& responses);

	void processEvent(Order* order, OrdEvent* ordEvent);

	void tradeAgainstBids(Order* pOrder, std::list<OrdEventResponse>& responses);

	void tradeAgainstAsks(Order* pOrder, std::list<OrdEventResponse>& responses);

	void cross(Order* pTakerOrd, Order* pMakerOrd, std::list<OrdEventResponse>& responses);


	inline TExecId newExecId() { return ++globalExecId; }

protected:
	OrdBook	m_ordBook;
	ClientInfoMap	m_clientInfos;

	static TExecId	globalExecId;
};
