/*
 * Movement module -- see header file for more info
 */

#include "movement.h"
#include <iostream>
namespace Movement {

	double movespeed = 0.5;
	double jetpackspeed = 0.2;
	double lookspeed = 0.035;
	double zoomspeed = 0.5;

//------------------------------------------------------------------------------

Controller::Controller(Camera &C, ObjectHandle P, double width, double height) : camera(C), player(P)
{
	fps = false;
    this->width = width;
    this->height = height;
	pos = player->origin;

	Vector<double> vec = camAngle * Vector<double>(0,1,0);

	zoom = 10.0;
	camera.origin = pos - (~vec * zoom);
	camera.lookAt(pos);

	camAngle = Qd();
}

//------------------------------------------------------------------------------

void Controller::setView(bool fp = true){
	fps = fp;
}

//------------------------------------------------------------------------------

bool Controller::getView(){
	return fps;
}

//------------------------------------------------------------------------------

void Controller::moveX()
{
    Point<double> posrollback = Point<double>(pos);
	if (move[dirLeft])
	{
		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));
		double yaw = atan2(vec.x, vec.y);

		yaw -= (.5 * Pi);

		pos.x = pos.x + movespeed * sin(yaw);
		pos.y = pos.y + movespeed * cos(yaw);
        if(!insideBounds(pos)){
            pos = posrollback;
            return;
        }
		camera.origin.x = camera.origin.x + movespeed * sin(yaw);
		camera.origin.y = camera.origin.y + movespeed * cos(yaw);
        
		//test
		player->origin = pos;
	}
	else if (move[dirRight])
	{
		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));
		double yaw = atan2(vec.x, vec.y);

		yaw += (.5 * Pi);

		pos.x = pos.x + movespeed * sin(yaw);
		pos.y = pos.y + movespeed * cos(yaw);
        if(!insideBounds(pos)){
            pos = posrollback;
            return;
        }
		camera.origin.x = camera.origin.x + movespeed * sin(yaw);
		camera.origin.y = camera.origin.y + movespeed * cos(yaw);

		//test
		player->origin = pos;
	}
}

//------------------------------------------------------------------------------

void Controller::moveY()
{
    Point<double> posrollback = Point<double>(pos);
	if (move[dirForward])
	{
		Vector<double> vec = ~(-player->rotation * Vector<double>(0,1,0));
		double yaw = atan2(vec.x, vec.y);

		pos.x = pos.x + movespeed * sin(yaw);
		pos.y = pos.y + movespeed * cos(yaw);
        
        if(!insideBounds(pos)){
            pos = posrollback;
            return;
        }
		camera.origin.x = camera.origin.x + movespeed * sin(yaw);
		camera.origin.y = camera.origin.y + movespeed * cos(yaw);
		//test
		player->origin = pos;
	}
	else if (move[dirBackward])
	{
		Vector<double> vec = ~(-player->rotation * Vector<double>(0,-1,0));
		double yaw = atan2(vec.x, vec.y);

		pos.x = pos.x + movespeed * sin(yaw);
		pos.y = pos.y + movespeed * cos(yaw);
        if(!insideBounds(pos)){
            pos = posrollback;
            return;
        }
		camera.origin.x = camera.origin.x + movespeed * sin(yaw);
		camera.origin.y = camera.origin.y + movespeed * cos(yaw);

		//test
		player->origin = pos;
	}
}

//------------------------------------------------------------------------------

void Controller::moveZ()
{
	if (move[dirUp])
	{
		pos.z = pos.z + jetpackspeed;
		camera.origin.z = camera.origin.z + jetpackspeed;

		//test
		player->origin = pos;
	}
	else if (move[dirDown])
	{
		pos.z = pos.z - jetpackspeed;
		camera.origin.z = camera.origin.z - jetpackspeed;

		//test
		player->origin = pos;
	}
}

//------------------------------------------------------------------------------

void Controller::lookX()
{
	if (look[dirLeft])
	{
		camAngle = Qd(Rd(lookspeed, Vd(0,0,1))) * camAngle;

		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));

		if (fps == true)
		{
			camera.origin = player->origin;
			camera.lookAt(pos + (vec * 5.0));
		}
		else
		{
			camera.origin = pos - (vec * zoom);
			camera.lookAt(pos);
		}
	}
	else if (look[dirRight])
	{
		camAngle = Qd(Rd(-lookspeed, Vd(0,0,1))) * camAngle;

		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));

		if (fps == true)
		{
			camera.origin = player->origin;
			camera.lookAt(pos + (vec * 5.0));
		}
		else
		{
			camera.origin = pos - (vec * zoom);
			camera.lookAt(pos);
		}
	}
}

//------------------------------------------------------------------------------

void Controller::lookY()
{
	if (look[dirForward]) // Zoom in
	{
		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));

		if (zoom > 5.0)
		{
			zoom -= zoomspeed;
		}
		else if (zoom <= 5.0)
		{
			fps = true;
		}

		if (fps == true)
		{
			camera.origin = player->origin;
			camera.lookAt(pos + (vec * 5.0));
		}
		else
		{
			camera.origin = pos - (vec * zoom);
			camera.lookAt(pos);
		}
	}
	else if (look[dirBackward]) // Zoom out
	{
		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));

		if (zoom < 15.0) 
		{
			zoom += zoomspeed;
		}
		
		if (fps == true)
		{
			fps = false;
			zoom = 5.0;
		}

		if (fps == true)
		{
			camera.origin = player->origin;
			camera.lookAt(pos + (vec * 5.0));
		}
		else
		{
			camera.origin = pos - (vec * zoom);
			camera.lookAt(pos);
		}
	}
}

//------------------------------------------------------------------------------

void Controller::lookZ()
{
    Qd buffer = camAngle;//Used to rollback if out of bounds
	if (look[dirUp])
	{
		Vector<double> mystery = ~(camAngle * Vector<double>(0,1,0));
		double mysteryYaw = atan2(mystery.x, mystery.y);
		camAngle = Qd(Rd(lookspeed, Vd(cos(mysteryYaw),-sin(mysteryYaw),0))) * camAngle;
        if((camAngle*Vector<double>(0,1,0)).z > 0.99){
            camAngle = buffer;
            return;
        }
		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));

		if (fps == true)
		{
			camera.origin = player->origin;
			camera.lookAt(pos + (vec * 5.0));
		}
		else
		{
			camera.origin = pos - (vec * zoom);
			camera.lookAt(pos);
		}
	}
	else if (look[dirDown])
	{
		Vector<double> mystery = ~(camAngle * Vector<double>(0,1,0));
		double mysteryYaw = atan2(mystery.x, mystery.y);

		camAngle = Qd(Rd(-lookspeed, Vd(cos(mysteryYaw),-sin(mysteryYaw),0))) * camAngle;
        if((camAngle*Vector<double>(0,1,0)).z < -0.99){
            camAngle = buffer;
            return;
        }
		Vector<double> vec = ~(camAngle * Vector<double>(0,1,0));

		if (fps == true)
		{
			camera.origin = player->origin;
			camera.lookAt(pos + (vec * 5.0));
		}
		else
		{
			camera.origin = pos - (vec * zoom);
			camera.lookAt(pos);
		}
	}
}

//------------------------------------------------------------------------------
    
void Controller::frame()
{
	/*if (move[dirLeft] || move[dirRight])
	{
		moveX();
	}*/
	if (move[dirForward] || move[dirBackward])
	{
		moveY();
		if (move[dirLeft]) player->rotation = player->rotation * Rd(-0.05,Vd(0,0,1));
		else if (move[dirRight]) player->rotation = player->rotation * Rd(0.05,Vd(0,0,1));
		else {
			double axis = Rd(player->rotation/camAngle).v.z;
            Vd camv = camAngle*Vd(0,1,0);
            Vd plav = player->rotation*Vd(0,1,0);
            camv.z = 0; camv = ~camv;
            plav.z = 0; plav = ~plav;
            double angle = atan2(camv.y - plav.y, camv.x - plav.x);
            double angle2 = (angle < 0? -angle : angle);
            angle2 = angle2 > 0.5*Pi ? Pi - angle2 : angle2;
            angle2 = angle2 < 0.05? angle2 : 0.05;
			if (axis != 0) {
                player->rotation = player->rotation * Rd(-angle2,Vd(0,0,axis));
            }
		}
	}

	if (move[dirUp] || move[dirDown])
	{
		moveZ();
	}


	if (look[dirLeft] || look[dirRight])
	{
		lookX();
	}

	if (look[dirForward] || look[dirBackward])
	{
		lookY();
	}

	if (look[dirUp] || look[dirDown])
	{
		lookZ();
	}

	
Objects::Player * p = dynamic_cast<Objects::Player*>(&*player);
	p->velocity = Vd(0,movespeed,0);
}

//------------------------------------------------------------------------------
bool Controller::insideBounds(Point<double> p){
    return -width/2 < p.x && p.x < width/2//Inside x-interval
        && -height/2 < p.y && p.y < height/2;//Inside y-interval
        
}
    
//------------------------------------------------------------------------------
} // namespace Movement

//------------------------------------------------------------------------------
