# module-5-physics-Brutus314
module-5-physics-Brutus314 created by GitHub Classroom

This module contains 2 separate modules. PhysicsModule contains actual PhysX code and runs a physics engine.
PseudoPhysicsModule does not contain PhysX code and does not run an engine. Instead, it receives messages from
PhysicsModule to update objects, giving the illusion that it does have a physics engine. 

In order to run this module, an instance of each of these must be running. The aftr.conf files are set up already.
Also, make sure the files included in lib are in the lib64 folder of the renderer.

To drive this module, all of the work is done on PhysicsModule. Both instances should see a small, yellow sphere
at around position 20, 20, 100. Pressing '1' will move this sphere to PhysicsModule's camera, and will update its
position accordingly in PseudoPhysicsModule. This is the location to drop cubes from. Pressing '2' will drop a
cube that is affected by physics and gravity. Though PhysicsModule is the only one running actual physics, both
instances should see the cube fall and interact with the ground and other cubes that may be there. Of course, 
PseudoPhysicsModule should be running before either of these buttons are pressed.