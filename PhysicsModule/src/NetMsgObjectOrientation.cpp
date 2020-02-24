#include <sstream>

#include "NetMsgObjectOrientation.h"
#include "ManagerEnvironmentConfiguration.h"
#include "ManagerGLView.h"
#include "GLViewPhysicsModule.h"
#include "WorldContainer.h"
#include "Model.h"

using namespace Aftr;

NetMsgMacroDefinition(NetMsgObjectOrientation);

// Send location, orientation, and the index as payload
bool NetMsgObjectOrientation::toStream(NetMessengerStreamBuffer& os) const {
	os << orientation[0] << orientation[1] << orientation[2] << orientation[3];
	os << orientation[4] << orientation[5] << orientation[6] << orientation[7];
	os << orientation[8] << orientation[9] << orientation[10]<< orientation[11];
	os << orientation[12] << orientation[13] << orientation[14] << orientation[15];
	os << location.x << location.y << location.z;
	os << wo_index;
	return true;
}

// Receive location, orientation, and the index as payload
bool NetMsgObjectOrientation::fromStream(NetMessengerStreamBuffer& is) {
	is >> orientation[0] >> orientation[1] >> orientation[2] >> orientation[3];
	is >> orientation[4] >> orientation[5] >> orientation[6] >> orientation[7];
	is >> orientation[8] >> orientation[9] >> orientation[10] >> orientation[11];
	is >> orientation[12] >> orientation[13] >> orientation[14] >> orientation[15];
	is >> location.x >> location.y >> location.z;
	is >> wo_index;
	return true;
}

// Update position and orientation
void NetMsgObjectOrientation::onMessageArrived() {
	WO* wo = ManagerGLView::getGLView()->getWorldContainer()->at(wo_index);
	wo->getModel()->setDisplayMatrix(orientation);
	wo->setPosition(location);
}

// For debug purposes
std::string NetMsgObjectOrientation::toString() const {
	std::stringstream ss;

	ss << NetMsg::toString();
	ss << "  Payload: \n"
		<< "Position: x = " << location.x << " y = " << location.y << " z = " << location.z << '\n'
		<< "Orientation:\n" << orientation[0] << ' ' << orientation[1] << ' ' << orientation[2] << ' ' << orientation[3] << '\n'
		<< orientation[4] << ' ' << orientation[5] << ' ' << orientation[6] << ' ' << orientation[7] << '\n'
		<< orientation[8] << ' ' << orientation[9] << ' ' << orientation[10] << ' ' << orientation[11] << '\n'
		<< orientation[12] << ' ' << orientation[13] << ' ' << orientation[14] << ' ' << orientation[15] << '\n'
		<< "Index: " << wo_index << '\n';
	return ss.str();
}
