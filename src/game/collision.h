/*******************************************************
 * Collision handler	  file                         *
 *                                                     *
 * Author: Kay Lukas                                   *
 *                                                     *
 * Date: 21-3-2012 17:06                               *
 *                                                     *
 * Description: Collision Detection algorithm          *
 *******************************************************/

#ifndef _COLLISION_H
#define _COLLISION_H

#include <stdint.h>
#include "core.h"
#include "base.h"
#include "shader.h"

namespace Collision {
using namespace Core;
using namespace Base::Alias;
using namespace Assets;


extern ShaderProgram* collShader;

//! Sets the object id of the object currently being drawn, the subtype id will also be set to zero.
void MarkObject(uint16_t objID);

//! Sets the subtype id of the object currently being drawn
void MarkSubType(uint8_t stID);

/*! Unregisters the object, which disables collision detection to be done on this object
 *  and frees the unique id assigned to the object. colID will be zero afterwards
 */
void UnregisterObject(ObjectHandle oh);

/*! Registers the object, assigning (internally) a unique 16-bit id to the object
 *  This id will be also be set as colID in the object. 
 */ 
void RegisterObject(ObjectHandle oh);

	
/*! Places a request to determine with which object the line starting in 'start'
 *  oriented using the vector direction collides. In this context a line is
 *  (mathematically viewed) a linesegment with a length of maxlength.
 *  the line can have a thickness (which is by default negligible or 0.0001), so
 *  so laserbeams can be used with a specific thickness.
 *  after the collision detection has been performed the onCollision method will be called
 *  with an ObjectHandle specifying the object with which a collision occured and a integer between
 *  0-255 giving information about the part of the object which was hit. This is called the subtype.
 */
void DetectCollision(Pd start, Vd direction, double maxLength, void (*onCollision)(ObjectHandle, uint8_t), double lineThickness = 0.0001);


/*! Processes all collission request. This method should only be called by a thread which has an opengl context,
 *  and an empty canvas. The canvas will be cleared after the method has finished executing.
 */
void ProcessCollisions();

//! Performs some initializations, should be called after the opengl context is initialized
void Initialize();

//! Performs clean ups.
void Terminate();
}

#endif /* _COLLISION_H */

//..............................................................................
