#include <sstream>

#include "NetMsgMoveSphere.h"
#include "ManagerEnvironmentConfiguration.h"
#include "ManagerGLView.h"
#include "GLViewPhysicsModule.h"
#include "WorldContainer.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgMoveSphere);

// Send location as payload
bool NetMsgMoveSphere::toStream(NetMessengerStreamBuffer& os) const {
	os << location.x << location.y << location.z;
	return true;
}

// Receive location as payload
bool NetMsgMoveSphere::fromStream(NetMessengerStreamBuffer& is) {
	is >> location.x >> location.y >> location.z;
	return true;
}

// Update position
void NetMsgMoveSphere::onMessageArrived() {
	ManagerGLView::getGLView<GLViewPhysicsModule>()->track_sphere->setPosition(location);
}

// For debug purposes
std::string NetMsgMoveSphere::toString() const {
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "  Payload: \n"
		<< "Position: x = " << location.x << " y = " << location.y << " z = " << location.z << "\n";
	return ss.str();
}
