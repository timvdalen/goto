/*
 * Movement module
 *
 * Date: 30-05-12 16:44
 *
 * Description:
 */

#ifndef _MOVEMENT_H
#define _MOVEMENT_H

#include "video.h"

//! Movement module
//! Takes care of camera movement
namespace Movement {

using namespace Video;

//------------------------------------------------------------------------------

//! Camera controller
class Controller
{
	public:
	Camera &camera; //!< Assigned camera
	
	enum Direction
	{
		dirDown    = -1, dirLeft  = -1, dirBackward = -1,
		dirNeutral =  0,
		dirUp      =  1, dirRight =  1, dirForward  =  1
	};
	
	Controller(Camera &C) : camera(C) {}
	
	void moveX(Direction); //! Move camera left and right
	void moveY(Direction); //! Move camera forward and backwards
	void moveZ(Direction); //! Move camera up and down
	
	void lookX(Direction); //! Look (rotate camera) left and right
	void lookY(Direction); //! Zoom in and out
	void lookZ(Direction); //! Look (rotate camera) up and down
};

//------------------------------------------------------------------------------

} // namespace Movement

#endif // _MOVEMENT_H

//------------------------------------------------------------------------------
