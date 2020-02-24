#pragma once

#include "NetMsg.h"
#include "Vector.h"
#include "WO.h"

#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr {
	class NetMsgMoveSphere : public NetMsg {
	public:
		NetMsgMacroDeclaration(NetMsgMoveSphere);

		//NetMsgCameraObject();
		//virtual ~NetMsgCameraObject();
		virtual bool toStream(NetMessengerStreamBuffer& os) const;
		virtual bool fromStream(NetMessengerStreamBuffer& is);
		virtual void onMessageArrived();
		virtual std::string toString() const;

		Vector location;
	};
}

#endif