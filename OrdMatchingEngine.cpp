// OrdMatchingEngine.cpp : Defines the entry point for the application.
//

#include "OrdMatchingEngine.h"

#include <vector>

TExecId OrdME::globalExecId(0);

void OrdME::submitNewOrder(std::unique_ptr<Order> upOrder) 
{
	auto it = m_clientInfos.find(upOrder->clientId());
	if (it == m_clientInfos.end()) {
		throw std::runtime_error("placeNewOrder unknown clientId " + upOrder->clientId());
	}

	ClientInfo& refCI(it->second);

	Order* pOrder = upOrder.get();
	if (pOrder->side() != OrdSide::BUY && pOrder->side() != OrdSide::SELL) {
		throw std::runtime_error("placeNewOrder unknown side");
	}

	std::list<OrdEventResponse> responses;

	NewOrdEvent* pNew = pOrder->addNew(++refCI.nextOrdId);
	
	responses.push_back(OrdEventResponse{ pOrder, pNew });

	auto pr = refCI.orders.emplace(std::make_pair(pOrder->getNewOrd()->newOrdId(), std::move(upOrder)));
	if (!pr.second) {
		throw std::runtime_error("placeNewOrder cannot insert ordId " + pOrder->getNewOrd()->newOrdId());
	}
	
	NewAckOrdEvent* pNewAck = pOrder->addNewAck(pOrder->px(), pOrder->qty());

	responses.push_back(OrdEventResponse{ pOrder, pNewAck });

	switch (pOrder->side()) {
	case OrdSide::BUY:
	{
		tradeAgainstAsks(pOrder, responses);
		if (pOrder->qtyOutstanding() > 0) {
			if (pOrder->px() == TPrice(0)) {
				// Expire order if it is market order
				Expired* expired = pOrder->addExpired(pOrder->qtyOutstanding());

				responses.push_back(OrdEventResponse{ pOrder, expired });
			}
			else {
				PriceLevel& refBid(m_ordBook.findOrCreateLimitBid(pOrder->px()));

				refBid.insertOrder(pOrder);
			}
		}
		break;
	}

	case OrdSide::SELL:
	{
		tradeAgainstBids(pOrder, responses);
		if (pOrder->qtyOutstanding() > 0) {
			if (pOrder->px() == TPrice(0)) {
				// Expire order if it is market order
				Expired* expired = pOrder->addExpired(pOrder->qtyOutstanding());

				responses.push_back(OrdEventResponse{ pOrder, expired });
			}
			else {
				PriceLevel& refAsk(m_ordBook.findOrCreateLimitAsk(pOrder->px()));

				refAsk.insertOrder(pOrder);
			}
		}
		break;
	}

	default:
		throw std::runtime_error("placeNewOrder unknown side");
		break;
	}

	handleEvents(responses);
}

void OrdME::submitCanOrder(const TClientId& clientId, const TOrdId& orderId)
{
	auto it = m_clientInfos.find(clientId);
	if (it == m_clientInfos.end()) {
		throw std::runtime_error("placeCanOrder cannot find clientId " + clientId);
	}

	ClientInfo& refCI(it->second);

	auto itOrd = refCI.orders.find(orderId);
	if (itOrd == refCI.orders.end()) {
		throw std::runtime_error("placeCanOrder");
	}
	if (itOrd->second->qtyOutstanding() == 0) {
		throw std::runtime_error("placeCanOrder clientId " + clientId);
	}

	std::list<OrdEventResponse> responses;

	Order* pOrd(itOrd->second.get());

	switch (pOrd->side()) {
	case OrdSide::BUY:
	{
		if (pOrd->px() == TPrice(0)) {
			PriceLevel* pPL = &(m_ordBook.mktBid());
			PriceLevel::OrdList::iterator itOrd(pPL->findOrder(pOrd->ordId()));

			if (itOrd == pPL->endOrders()) {
				throw std::runtime_error("placeCanOrder cannot find in ordBook");
			}

			CanOrdEvent* pCan = pOrd->addCan();

			responses.emplace_back(OrdEventResponse{ pOrd, pCan });

			CanAckOrdEvent* pCanAck = pOrd->addCanAck(pOrd->qtyOutstanding());

			responses.emplace_back(OrdEventResponse{ pOrd, pCanAck });
			pPL->removeOrder(itOrd);
		}
		else {
			auto itPL = m_ordBook.findLimitBid(pOrd->px());
			if (itPL == m_ordBook.endLimitBids()) {
				throw std::runtime_error("placeCanOrder cannot find in Limit Bids");
			}

			PriceLevel* pPL = &(itPL->second);
			PriceLevel::OrdList::iterator itOrd(pPL->findOrder(pOrd->ordId()));

			if (itOrd == pPL->endOrders()) {
				throw std::runtime_error("placeCanOrder cannot find in ordBook");
			}

			CanOrdEvent* pCan = pOrd->addCan();

			responses.emplace_back(OrdEventResponse{ pOrd, pCan });

			CanAckOrdEvent* pCanAck = pOrd->addCanAck(pOrd->qtyOutstanding());

			responses.emplace_back(OrdEventResponse{ pOrd, pCanAck });
			pPL->removeOrder(itOrd);
			if (pPL->isEmpty()) {
				m_ordBook.removeLimitBid(itPL);
			}
		}
		break;
	}

	case OrdSide::SELL:
	{
		if (pOrd->px() == TPrice(0)) {
			PriceLevel* pPL = &(m_ordBook.mktAsk());
			PriceLevel::OrdList::iterator itOrd(pPL->findOrder(pOrd->ordId()));

			if (itOrd == pPL->endOrders()) {
				throw std::runtime_error("placeCanOrder cannot find in ordBook");
			}

			CanOrdEvent* pCan = pOrd->addCan();

			responses.emplace_back(OrdEventResponse{ pOrd, pCan });

			CanAckOrdEvent* pCanAck = pOrd->addCanAck(pOrd->qtyOutstanding());

			responses.emplace_back(OrdEventResponse{ pOrd, pCanAck });
			pPL->removeOrder(itOrd);
		}
		else {
			auto itPL = m_ordBook.findLimitAsk(pOrd->px());
			if (itPL == m_ordBook.endLimitAsks()) {
				throw std::runtime_error("placeCanOrder cannot find in Limit Asks");
			}

			PriceLevel* pPL = &(itPL->second);
			PriceLevel::OrdList::iterator itOrd(pPL->findOrder(pOrd->ordId()));
			
			if (itOrd == pPL->endOrders()) {
				throw std::runtime_error("placeCanOrder cannot find in ordBook");
			}

			CanOrdEvent* pCan = pOrd->addCan();

			responses.emplace_back(OrdEventResponse{ pOrd, pCan });

			CanAckOrdEvent* pCanAck = pOrd->addCanAck(pOrd->qtyOutstanding());

			responses.emplace_back(OrdEventResponse{ pOrd, pCanAck });
			pPL->removeOrder(itOrd);
			if (pPL->isEmpty()) {
				m_ordBook.removeLimitAsk(itPL);
			}
		}
		break;
	}

	default:
		throw std::runtime_error("placeNewOrder unknown side");
		break;
	}

	handleEvents(responses);
}

void OrdME::handleEvents(std::list<OrdEventResponse>& responses)
{
	for (auto resp : responses) {
		processEvent(resp.order, resp.ordEvent);
	}
}

void OrdME::processEvent(Order* order, OrdEvent* ordEvent)
{
	auto it = m_clientInfos.find(order->clientId());
	if (it == m_clientInfos.end()) {
		throw std::runtime_error("placeCanOrder cannot find clientId " + order->clientId());
	}

	ClientInfo& refCI(it->second);

	if (!refCI.pCallback) return; // no callback registered

	switch (ordEvent->eventType()) {
	case OrdEventType::NEW:
		refCI.pCallback->onNew(order, dynamic_cast<NewOrdEvent*>(ordEvent));
		break;
	case OrdEventType::NEW_REJECT:
		refCI.pCallback->onNewRej(order, dynamic_cast<NewRejOrdEvent*>(ordEvent));
		break;
	case OrdEventType::NEW_ACK:
		refCI.pCallback->onNewAck(order, dynamic_cast<NewAckOrdEvent*>(ordEvent));
		break;
	case OrdEventType::CANCEL:
		refCI.pCallback->onCan(order, dynamic_cast<CanOrdEvent*>(ordEvent));
		break;
	case OrdEventType::CANCEL_REJECT:
		refCI.pCallback->onCanRej(order, dynamic_cast<CanRejOrdEvent*>(ordEvent));
		break;
	case OrdEventType::CANCEL_ACK:
		refCI.pCallback->onCanAck(order, dynamic_cast<CanAckOrdEvent*>(ordEvent));
		break;
	case OrdEventType::EXECUTION:
		refCI.pCallback->onExec(order, dynamic_cast<Execution*>(ordEvent));
		break;
	case OrdEventType::EXPIRY:
		refCI.pCallback->onExpiry(order, dynamic_cast<Expired*>(ordEvent));
		break;
	case OrdEventType::NONE:
	default:
		throw std::runtime_error("processEvent unknown event type");
		break;
	}
}

void OrdME::tradeAgainstBids(Order* pOrder, std::list<OrdEventResponse>& responses)
{
	assert(pOrder->side() == OrdSide::SELL);

	if (!m_ordBook.mktBid().isEmpty()) {
		PriceLevel& refBid(m_ordBook.mktBid());

		while (!refBid.isEmpty()) {
			Order* pBidOrd = refBid.frontOrder();

			cross(pOrder, pBidOrd, responses);
			if (pBidOrd->qtyOutstanding() == 0) {
				refBid.popFrontOrder();
			}
			if (pOrder->qtyOutstanding() == 0) break;
		}
		if (pOrder->qtyOutstanding() == 0) return;
	}

	while (m_ordBook.hasLimitBid()) {
		PriceLevel& refBid(m_ordBook.bestLimitBid());

		while (!refBid.isEmpty()) {
			Order* pBidOrd = refBid.frontOrder();

			if (pOrder->px() > pBidOrd->px()) return; // cannot trade anymore

			cross(pOrder, pBidOrd, responses);
			if (pBidOrd->qtyOutstanding() == 0) {
				refBid.popFrontOrder();
			}
			if (pOrder->qtyOutstanding() == 0) break;
		}
		if (refBid.isEmpty()) {
			m_ordBook.popBestLimitBid();
		}
		if (pOrder->qtyOutstanding() == 0) return;
	}
}

void OrdME::tradeAgainstAsks(Order* pOrder, std::list<OrdEventResponse>& responses)
{
	assert(pOrder->side() == OrdSide::BUY);

	if (!m_ordBook.mktAsk().isEmpty()) {
		PriceLevel& refAsk(m_ordBook.mktAsk());
		
		while (!refAsk.isEmpty()) {
			Order* pAskOrd = refAsk.frontOrder();

			cross(pOrder, pAskOrd, responses);
			if (pAskOrd->qtyOutstanding() == 0) {
				refAsk.popFrontOrder();
			}
			if (pOrder->qtyOutstanding() == 0) break;
		}
		if (pOrder->qtyOutstanding() == 0) return;
	}

	while (m_ordBook.hasLimitAsk()) {
		PriceLevel& refAsk(m_ordBook.bestLimitAsk());

		while (!refAsk.isEmpty()) {
			Order* pAskOrd = refAsk.frontOrder();

			if (pOrder->px() < pAskOrd->px()) return; // cannot trade anymore

			cross(pOrder, pAskOrd, responses);
			if (pAskOrd->qtyOutstanding() == 0) {
				refAsk.popFrontOrder();
			}
			if (pOrder->qtyOutstanding() == 0) break;
		}
		if (refAsk.isEmpty()) {
			m_ordBook.popBestLimitAsk();
		}
		if (pOrder->qtyOutstanding() == 0) return;
	}
}

void OrdME::cross(Order* pTakerOrd, Order* pMakerOrd, std::list<OrdEventResponse>& responses)
{
	TQty qtyExec(std::min(pTakerOrd->qtyOutstanding(), pMakerOrd->qtyOutstanding()));

	TExecId	execId(newExecId());

	Execution* pMakerExec = pMakerOrd->addExecution(execId, pMakerOrd->px(), qtyExec);
	Execution* pTakerExec = pTakerOrd->addExecution(execId, pMakerOrd->px(), qtyExec);

	responses.emplace_back(OrdEventResponse{ pMakerOrd, pMakerExec });
	responses.emplace_back(OrdEventResponse{ pTakerOrd, pTakerExec });
}

class Client : public OrdME::Callback
{
public:
	Client(const TClientId& clientId) : m_clientId(clientId) {}

	inline const TClientId& clientId() const { return m_clientId; }

	void onNew(Order* order, NewOrdEvent* event) override
	{
		std::cout << "onNew clientId " << clientId() << " ordId " << order->ordId() << " " << toString(order->state()) << " " << toString(order->side());
		std::cout << " px " << order->px() << " qty " << order->qty() << std::endl;
	}

	void onNewRej(Order* order, NewRejOrdEvent* event) override 
	{
		std::cout << "onNewRej clientId " << clientId() << " ordId " << order->ordId() << " px " << order->px() << " qty " << order->qty() << std::endl;
	}

	void onNewAck(Order* order, NewAckOrdEvent* event) override 
	{
		std::cout << "onNewAck clientId " << clientId() << " ordId " << order->ordId() << " " << toString(order->state()) << " " << toString(order->side());
		std::cout << " px " << order->px() << " qty " << order->qty();
		std::cout << " cumOut " << order->qtyOutstanding() << " cumExe " << order->qtyExec() << " cumCan " << order->qtyCancelled() << std::endl;
	}

	void onCan(Order* order, CanOrdEvent* event) override
	{
		std::cout << "onCan clientId " << clientId() << " ordId " << order->ordId() << " " << toString(order->state()) << " " << toString(order->side()) << std::endl;
	}

	void onCanRej(Order* order, CanRejOrdEvent* event) override
	{
		std::cout << "onCanRej clientId " << clientId() << " ordId " << order->ordId() << " " << toString(order->state()) << " " << toString(order->side()) << std::endl;
	}

	void onCanAck(Order* order, CanAckOrdEvent* event) override
	{
		std::cout << "onCanAck clientId " << clientId() << " ordId " << order->ordId() << " " << toString(order->state()) << " " << toString(order->side());
		std::cout << " px " << order->px() << " qty " << order->qty() << " canQty " << event->qtyCancelled();
		std::cout << " cumOut " << order->qtyOutstanding() << " cumExe " << order->qtyExec() << " cumCan " << order->qtyCancelled() << std::endl;
	}

	void onExec(Order* order, Execution* event) override 
	{
		std::cout << "onExec clientId " << clientId() << " ordId " << order->ordId() << " " << toString(order->state()) << " " << toString(order->side());
		std::cout << " execId " << event->execId() << " exePx " << event->pxExec() << " exeQty " << event->qtyExec();
		std::cout << " cumOut " << order->qtyOutstanding() << " cumExe " << order->qtyExec() << " cumCan " << order->qtyCancelled() << std::endl;
	}

	void onExpiry(Order* order, Expired* event) override
	{
		std::cout << "onExpiry clientId " << clientId() << " ordId " << order->ordId() << " " << toString(order->state()) << " " << toString(order->side());
		std::cout << " cancelled " << event->qtyCancelled() << std::endl;
		std::cout << " cumOut " << order->qtyOutstanding() << " cumExe " << order->qtyExec() << " cumCan " << order->qtyCancelled() << std::endl;
	}

protected:
	TClientId	m_clientId;
};

int main()
{
	OrdME		me;
	std::vector<Client>		clients({Client(0), Client(1), Client(2)});

	for (Client& cl : clients) {
		me.registerClient(cl.clientId(), &cl);
	}

	bool	isQuit(false);
	int	currClientId(0);
	int	command(0);

	while (!isQuit) {
		std::cout << "1. Select client (" << currClientId << ")" << std::endl;
		std::cout << "2. Dump order book" << std::endl;
		std::cout << "3. Create order" << std::endl;
		std::cout << "4. Cancel order" << std::endl;
		std::cout << "5. Quit" << std::endl;
		std::cout << "Select command: ";

		std::cin >> command;

		switch (command) {
		case 1:
		{
			int newClientId(0);
			std::cout << "Select client (0.." << clients.size()-1 << "): ";
			std::cin >> newClientId;

			if (newClientId < 0 || newClientId > clients.size()-1) {
				std::cout << "Unknown clientId " << newClientId << std::endl;
			}
			else {
				std::cout << "ClientId set to " << newClientId << std::endl;
				currClientId = newClientId;
			}
			break;
		}
		case 2:
			std::cout << "Dump order book" << std::endl;
			me.dumpOrdBook();
			std::cout << std::endl;
			break;
		case 3:
		{
			char	chSide(0);
			float	price(0.0);
			TQty qty(0);
			OrdSide	ordSide(OrdSide::NONE);

			std::cout << "Create order" << std::endl;
			std::cout << "Side(B/S): ";
			std::cin >> chSide;
			chSide = std::toupper(chSide);
			switch(chSide) {
			case 'B':
				ordSide = OrdSide::BUY;
				break;
			case 'S':
				ordSide = OrdSide::SELL;
				break;
			default:
				std::cout << "Unknown side " << chSide << std::endl;
				continue;
			}

			std::cout << "Price up to 2 decimal precision(0 for market order) : ";
			std::cin >> price;

			TPrice	px(price);

			if (px < TPrice(0)) {
				std::cout << "Price must not be negative" << std::endl;
				continue;
			}

			std::cout << "Qty: ";
			std::cin >> qty;

			if (qty <= 0) {
				std::cout << "Qty must be greater than 0" << std::endl;
				continue;
			}
					
			std::unique_ptr<Order> p(std::make_unique<Order>(currClientId, ordSide, px, qty));
			try {
				me.submitNewOrder(std::move(p));
				std::cout << "Submit new order" << std::endl;
			}
			catch (const std::runtime_error& re) {
				std::cerr << "Failed to submit new order" << std::endl;
			}
			break;
		}
		case 4:
		{
			TOrdId	canOrdId;

			std::cout << "Cancel order" << std::endl;
			std::cout << "Enter orderId: ";
			std::cin >> canOrdId;

			try {
				me.submitCanOrder(currClientId, canOrdId);
				std::cout << "Submit can order " << canOrdId << std::endl;
			}
			catch (const std::runtime_error& re) {
				std::cerr << "Failed to submit can order" << std::endl;
			}
			break;
		}
		case 5:
			std::cout << "Quit..." << std::endl;			
			isQuit = true;
			break;
		default:
			std::cout << "Unknown command " << command << std::endl;
			break;
		}
	}

	return 0;
}
