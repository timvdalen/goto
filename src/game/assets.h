/*
 * Assets definitions
 *
 * Date: 04-06-12 14:06
 *
 * Description: Definition of the artwork and assets used by the game.
 *
 */

#ifndef _ASSETS_H
#define _ASSETS_H

#include "core.h"

namespace Assets {

using namespace Core;

//------------------------------------------------------------------------------

void Initialize(int argc, char *argv[]);
void Terminate();

//------------------------------------------------------------------------------

extern MaterialHandle Grid;
extern MaterialHandle SelectedGrid;

extern MaterialHandle Test;

extern MaterialHandle Cloud;
extern MaterialHandle Grass;

extern MaterialHandle Head;
extern MaterialHandle Body;
extern MaterialHandle Gun;

//------------------------------------------------------------------------------

} // namespace Assets

#endif // _ASSETS_H

//------------------------------------------------------------------------------