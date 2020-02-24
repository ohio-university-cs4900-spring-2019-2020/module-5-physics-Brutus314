#include <sstream>

#include "NetMsgNewSharedObject.h"
#include "WO.h"
#include "ManagerGLView.h"
#include "GLViewPhysicsModule.h"
#include "WorldContainer.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgNewSharedObject);

// Send model path, scale, and location as payload
bool NetMsgNewSharedObject::toStream(NetMessengerStreamBuffer& os) const {
	os << model_path << size_scale.x << size_scale.y << size_scale.z;
	os << location.x << location.y << location.z;
	return true;
}

// Get model path, scale, and location as payload
bool NetMsgNewSharedObject::fromStream(NetMessengerStreamBuffer& is) {
	is >> model_path >> size_scale.x >> size_scale.y >> size_scale.z;
	is >> location.x >> location.y >> location.z;
	return true;
}

// Add new object at the position
void NetMsgNewSharedObject::onMessageArrived() {
	WO* wo = WO::New(model_path, size_scale);
	wo->setPosition(location);
	ManagerGLView::getGLView()->getWorldContainer()->push_back(wo);
}

// For debug purposes
std::string NetMsgNewSharedObject::toString() const {
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "  Payload: \n"
		<< "Model: " << model_path << "\n"
		<< "Size: x = " << size_scale.x << " y = " << size_scale.y << " z = " << size_scale.z << "\n"
		<< "Position: x = " << location.x << " y = " << location.y << " z = " << location.z << "\n";
	return ss.str();
}
