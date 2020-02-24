#pragma once

#include "NetMsg.h"
#include "Vector.h"
#include "Mat4.h"
#include "WO.h"

#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr {
	class NetMsgObjectOrientation : public NetMsg {
	public:
		NetMsgMacroDeclaration(NetMsgObjectOrientation);

		//NetMsgCameraObject();
		//virtual ~NetMsgCameraObject();
		virtual bool toStream(NetMessengerStreamBuffer& os) const;
		virtual bool fromStream(NetMessengerStreamBuffer& is);
		virtual void onMessageArrived();
		virtual std::string toString() const;

		Vector location;
		Mat4 orientation;
		int wo_index;
	};
}

#endif