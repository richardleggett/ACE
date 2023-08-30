/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ace/utils/extview.h>
#include <ace/managers/sdl_private.h>

UBYTE viewIsLoaded(const tView *pView) {
	UBYTE isLoaded = (sdlGetCurrentView() == pView);
	return isLoaded;
}

void viewUpdateCLUT(tView *pView) {
	sdlSetCurrentView(pView);
}

void viewLoad(tView *pView) {
	sdlSetCurrentView(pView);
}

void vPortWaitForPos(const tVPort *pVPort, UWORD uwPosY, UBYTE isExact) {

}
