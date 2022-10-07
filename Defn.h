#pragma once

#include "DecimalLong.h"

#include <numeric>
#include <type_traits>

using TPrice = DecimalLong<2>;
using TQty = std::uint32_t;
using TOrdId = std::uint32_t;
using TExecId = std::uint32_t;
using TClientId = int;

enum class OrdEventType {
	NONE,
	NEW,
	NEW_ACK,
	NEW_REJECT,
	CANCEL,
	CANCEL_ACK,
	CANCEL_REJECT,
	EXECUTION,
	EXPIRY
};

static const std::string OrdEventTypeStr[] = {
	"NONE",
	"NEW",
	"NEW_ACK",
	"NEW_REJECT",
	"CANCEL",
	"CANCEL_ACK",
	"CANCAL_REJECT",
	"EXECUTION",
	"EXPIRY"
};

enum class OrdStateType {
	NONE,
	NEW,
	REJECTED,
	ACTIVE,
	CANCELLED,
	COMPLETED,
	EXPIRED
};

static const std::string OrdStateTypeStr[] = { 
	"NONE", 
	"NEW",
	"REJECTED",
	"ACTIVE",
	"CANCELLED",
	"COMPLETED",
	"EXPIRED"
};

enum class OrdSide {
	NONE,
	BUY,
	SELL
};

static const std::string OrdSideStr[] = {
	"NONE",
	"BUY",
	"SELL"
};

static const std::string& toString(OrdEventType evt)
{
	return OrdEventTypeStr[static_cast<std::underlying_type<OrdEventType>::type>(evt)];
}

static const std::string& toString(OrdStateType state)
{
	return OrdStateTypeStr[static_cast<std::underlying_type<OrdStateType>::type>(state)];
}

static const std::string& toString(OrdSide side)
{
	return OrdSideStr[static_cast<std::underlying_type<OrdSide>::type>(side)];
}
