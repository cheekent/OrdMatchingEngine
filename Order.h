#pragma once

#include "OrdEvent.h"

#include <list>
#include <string>
#include <memory>
#include <iostream>

class Order
{
public:
	using OrdEventList = std::list< std::unique_ptr<OrdEvent> >;
	using ord_iterator = OrdEventList::iterator;
	using const_ord_iterator = OrdEventList::const_iterator;

public:
	Order(const TClientId& clientId, OrdSide side, const TPrice& px, const TQty& qty) : 
		m_clientId(clientId), m_ordId(0), m_side(side), m_px(px), m_qty(qty),
		m_qtyOutstanding(TQty(0)), m_qtyCancelled(TQty(0)), m_qtyExec(TQty(0)),
		m_state(OrdStateType::NONE)
	{}

	inline const TClientId& clientId() const { return m_clientId; }
	inline const TOrdId& ordId() const { return m_ordId; }
	inline OrdSide side() const { return m_side; }
	inline const TPrice& px() const { return m_px; }
	inline const TQty& qty() const { return m_qty; }
	inline const TQty& qtyOutstanding() const { return m_qtyOutstanding; }
	inline const TQty& qtyCancelled() const { return m_qtyCancelled; }
	inline const TQty& qtyExec() const { return m_qtyExec; }
	inline OrdStateType state() const { return m_state; }

	inline const NewOrdEvent* getNewOrd() const
	{
		assert(!m_ordEvents.empty());

		return dynamic_cast<const NewOrdEvent*>(m_ordEvents.front().get());
	}

	inline NewOrdEvent* addNew(const TOrdId& newOrdId) 
	{
		// Precondition
		assert(m_ordEvents.empty());
		assert(m_state == OrdStateType::NONE);

		m_ordEvents.emplace_back(std::make_unique<NewOrdEvent>(newOrdId, m_side, m_px, m_qty));
		m_ordId = newOrdId;
		m_state = OrdStateType::NEW;
		return dynamic_cast<NewOrdEvent*>(m_ordEvents.back().get());
	}

	inline NewRejOrdEvent* addNewRej()
	{
		assert(m_state == OrdStateType::NEW);

		m_ordEvents.emplace_back(std::make_unique<NewRejOrdEvent>(ordId()));
		m_state = OrdStateType::REJECTED;
		return dynamic_cast<NewRejOrdEvent*>(m_ordEvents.back().get());
	}

	inline NewAckOrdEvent* addNewAck(const TPrice& px, const TQty& qtyOutstanding)
	{
		assert(m_state == OrdStateType::NEW);

		m_ordEvents.emplace_back(std::make_unique<NewAckOrdEvent>(ordId(), px, qtyOutstanding));
		m_state = OrdStateType::ACTIVE;
		m_qtyOutstanding = qtyOutstanding;
		return dynamic_cast<NewAckOrdEvent*>(m_ordEvents.back().get());
	}

	inline CanOrdEvent* addCan()
	{
		assert(m_state == OrdStateType::ACTIVE);

		m_ordEvents.emplace_back(std::make_unique<CanOrdEvent>(m_qtyOutstanding));
		return dynamic_cast<CanOrdEvent*>(m_ordEvents.back().get());
	}

	inline CanRejOrdEvent* addCanRej()
	{
		m_ordEvents.emplace_back(std::make_unique<CanRejOrdEvent>());
		return dynamic_cast<CanRejOrdEvent*>(m_ordEvents.back().get());
	}

	inline CanAckOrdEvent* addCanAck(const TQty& qtyCancelled)
	{
		const NewOrdEvent* newOrd(getNewOrd());

		m_ordEvents.emplace_back(std::make_unique<CanAckOrdEvent>(qtyCancelled));
		m_qtyOutstanding -= qtyCancelled;
		m_qtyCancelled += qtyCancelled;
		m_state = OrdStateType::CANCELLED;
		return dynamic_cast<CanAckOrdEvent*>(m_ordEvents.back().get());
	}

	inline Execution* addExecution(const TExecId& execId, const TPrice& pxExec, const TQty& qtyExec)
	{
		m_ordEvents.emplace_back(std::make_unique<Execution>(execId,  pxExec, qtyExec));
		m_qtyOutstanding -= qtyExec;
		m_qtyExec += qtyExec;
		if (m_qtyOutstanding == 0) {
			if (m_state != OrdStateType::CANCELLED) {
				m_state = OrdStateType::COMPLETED;
			}
		}
		return dynamic_cast<Execution*>(m_ordEvents.back().get());
	}

	inline Expired* addExpired(const TQty& qtyCancelled)
	{
		m_ordEvents.emplace_back(std::make_unique<Expired>(qtyCancelled));
		m_qtyOutstanding -= qtyCancelled;
		m_state = OrdStateType::EXPIRED;
		return dynamic_cast<Expired*>(m_ordEvents.back().get());
	}

	inline ord_iterator beginOrdEvents() { return m_ordEvents.begin(); }
	inline const_ord_iterator beginOrdEvents() const { return m_ordEvents.begin(); }
	inline const_ord_iterator endOrdEvents() const { return m_ordEvents.end(); }

	inline void dumpOrder(bool dumpOrdEvents = false) const
	{
		std::cout << " [" << m_clientId << ", " << m_ordId << ", " << m_px << ", " << m_qty << ", " << toString(m_state);
		std::cout << ", " << m_qtyOutstanding << ", " << m_qtyExec << ", " << m_qtyCancelled;
		if (dumpOrdEvents && !m_ordEvents.empty()) {
			std::cout << " :";
			for (auto& p : m_ordEvents) {
				std::cout << " (";
				p->dump();
				std::cout << ")";
			}
		}
		std::cout << "]";
	}

protected:
	OrdEventList	m_ordEvents;
	TClientId		m_clientId;
	TOrdId			m_ordId;
	OrdSide			m_side;
	TPrice			m_px;
	TQty			m_qty;
	TQty			m_qtyOutstanding, m_qtyCancelled, m_qtyExec;
	OrdStateType	m_state;
};
