#pragma once

#include "NetMsg.h"
#include "Vector.h"

#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr {
	class NetMsgNewSharedObject : public NetMsg {
		public: 
			NetMsgMacroDeclaration(NetMsgNewSharedObject);

			//NetMsgNewSharedObject();
			//virtual ~NetMsgNewSharedObject();
			virtual bool toStream(NetMessengerStreamBuffer& os) const;
			virtual bool fromStream(NetMessengerStreamBuffer& is);
			virtual void onMessageArrived();
			virtual std::string toString() const;

			std::string model_path;
			Vector size_scale;
			Vector location;
		protected:
	};
}

#endif