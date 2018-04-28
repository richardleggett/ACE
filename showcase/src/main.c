#include "main.h"
#include <stdlib.h> // EXIT_SUCCESS
#include <ace/managers/memory.h>
#include <ace/managers/log.h>
#include <ace/managers/timer.h>
#include <ace/managers/blit.h>
#include <ace/managers/copper.h>
#include <ace/managers/game.h>
#include <ace/managers/system.h>

#include "input.h"
#include "menu/menu.h"

int main(void) {
	systemCreate();
	memCreate();
	logOpen();
	timerCreate();

	blitManagerCreate();
	copCreate();

	inputOpen();

	gameCreate();
	gamePushState(gsMenuCreate, gsMenuLoop, gsMenuDestroy);
	while (gameIsRunning()) {
		timerProcess();
		inputProcess();
		gameProcess();
	}
	gameDestroy();

	inputClose();

	copDestroy();
	blitManagerDestroy();

	timerDestroy();
	logClose();
	memDestroy();
	systemDestroy();
	return EXIT_SUCCESS;
}
