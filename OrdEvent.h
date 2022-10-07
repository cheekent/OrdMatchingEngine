#pragma once

#include "Defn.h"

#include <numeric>
#include <iostream>

class OrdEvent
{
public:
	OrdEvent(OrdEventType evtType) : m_evtType(evtType) {}
	virtual ~OrdEvent() {}

	inline OrdEventType eventType() const { return m_evtType; }

	virtual void dump()
	{
		std::cout << toString(m_evtType);
	}

protected:
	OrdEventType	m_evtType;
};

class NewOrdEvent : public OrdEvent
{
public:
	NewOrdEvent(const TOrdId& newOrdId, OrdSide side, const TPrice& px, const TQty& qty) : 
		OrdEvent(OrdEventType::NEW), m_newOrdId(newOrdId), m_side(side), m_px(px), m_qty(qty) 
	{}

	inline const TOrdId& newOrdId() const { return m_newOrdId; }
	inline OrdSide side() const { return m_side; }
	inline const TPrice& px() const { return m_px; }
	inline const TQty& qty() const { return m_qty; }

	virtual void dump()
	{
		OrdEvent::dump();
		std::cout << ", " << m_newOrdId << ", " << toString(m_side) << ", " << m_px << ", " << m_qty;
	}

protected:
	TOrdId	m_newOrdId;
	OrdSide	m_side;
	TPrice	m_px;
	TQty	m_qty;
};

class NewRejOrdEvent : public OrdEvent
{
public:
	NewRejOrdEvent(const TOrdId& newOrdId) : OrdEvent(OrdEventType::NEW_REJECT), m_newOrdId(newOrdId) {}

	inline const TOrdId& newOrdId() const { return m_newOrdId; }

	virtual void dump()
	{
		OrdEvent::dump();
		std::cout << ", " << m_newOrdId;
	}
	
protected:
	TOrdId	m_newOrdId;
};

class NewAckOrdEvent : public OrdEvent
{
public:
	NewAckOrdEvent(const TOrdId& newOrdId, const TPrice& px, const TQty& qtyOutstanding) :
		OrdEvent(OrdEventType::NEW_ACK),
		m_newOrdId(newOrdId),
		m_px(px),
		m_qtyOutstanding(qtyOutstanding)
	{}

	inline const TOrdId& newOrdId() const { return m_newOrdId; }
	inline const TPrice& px() const { return m_px; }
	inline const TQty& qtyOutstanding() const { return m_qtyOutstanding; }

	virtual void dump()
	{
		OrdEvent::dump();
		std::cout << ", " << m_newOrdId << ", " << m_px << ", " << m_qtyOutstanding;
	}

protected:
	TOrdId		m_newOrdId;
	TPrice		m_px;
	TQty		m_qtyOutstanding;
};

class CanOrdEvent : public OrdEvent
{
public:
	CanOrdEvent(const TQty& qtyCancel) : 
		OrdEvent(OrdEventType::CANCEL) 
	{}

	inline const TQty& qtyCancel() const { return m_qtyCancel; }

	virtual void dump()
	{
		OrdEvent::dump();
		std::cout << ", " << m_qtyCancel;
	}

protected:
	TQty	m_qtyCancel;
};

class CanRejOrdEvent : public OrdEvent
{
public:
	CanRejOrdEvent() : OrdEvent(OrdEventType::CANCEL_REJECT) {}

protected:
};

class CanAckOrdEvent : public OrdEvent
{
public:
	CanAckOrdEvent(const TQty& qtyCancelled) :
		OrdEvent(OrdEventType::CANCEL_ACK), m_qtyCancelled(qtyCancelled) 
	{}

	inline const TQty& qtyCancelled() const { return m_qtyCancelled; }

	virtual void dump()
	{
		OrdEvent::dump();
		std::cout << ", " << m_qtyCancelled;
	}

protected:
	TQty	m_qtyCancelled;
};

class Execution : public OrdEvent
{
public:
	Execution(const TExecId& execId, const TPrice& pxExec, const TQty& qtyExec) :
		OrdEvent(OrdEventType::EXECUTION), m_execId(execId), m_pxExec(pxExec), m_qtyExec(qtyExec)
	{}

	inline const TExecId& execId() const { return m_execId; }
	inline const TPrice& pxExec() const { return m_pxExec; }
	inline const TQty& qtyExec() const { return m_qtyExec; }

	virtual void dump()
	{
		OrdEvent::dump();
		std::cout << ", " << execId() << ", " << pxExec() << ", " << qtyExec();
	}

protected:
	TExecId		m_execId;
	TPrice		m_pxExec;
	TQty		m_qtyExec;
};

class Expired : public OrdEvent
{
public:
	Expired(const TQty& qtyCancelled) : OrdEvent(OrdEventType::EXPIRY), m_qtyCancelled(qtyCancelled) {}

	inline const TQty& qtyCancelled() const { return m_qtyCancelled; }

	virtual void dump()
	{
		OrdEvent::dump();
		std::cout << ", " << qtyCancelled();
	}

protected:
	TQty	m_qtyCancelled;
};
