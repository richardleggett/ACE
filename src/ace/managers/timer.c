#include <ace/managers/timer.h>
#include <ace/managers/log.h>
#include <ace/managers/system.h>

/* Globals */
tTimerManager g_sTimerManager = {0};

/* Functions */
#ifdef AMIGA

/**
 * Timer VBlank server
 * Increments frame counter
 */
FN_HOTSPOT
void INTERRUPT timerVBlankServer(
	UNUSED_ARG REGARG(volatile tCustom *pCustom, "a0"),
	REGARG(volatile void *pData, "a1")
) {
	UWORD *pCounter = (UWORD*)pData;
	++(*pCounter);
	INTERRUPT_END;
}

#endif // AMIGA

/**
 * Creates Vertical Blank server for counting frames
 */
void timerCreate(void) {
	g_sTimerManager.uwFrameCounter = 0;
	systemSetInt(
		INTB_VERTB, timerVBlankServer, &g_sTimerManager.uwFrameCounter
	);
}

/**
 * Removes Vertical Blank server
 */
void timerDestroy(void) {
	systemSetInt(INTB_VERTB, 0, 0);
}

/**
 * Gets current time based on frame number
 * One tick equals: PAL - 20ms, NTSC - 16.67ms
 * Max time capacity: 33 months
 */
ULONG timerGet(void) {
	return g_sTimerManager.uwFrameCounter;
}

/**
 * Gets as precise current time as possible
 * Implementation based on ray position and frame number
 * One tick equals: PAL - 0.40us, NTSC - 0.45us
 * Max time capacity: 1715s (28,5 min)
 */
ULONG timerGetPrec(void) {
	#ifdef AMIGA
	UWORD uwFr1, uwFr2; // frame counts
	tRayPos sRay;
	ULONG *pRay = (ULONG*)&sRay, *pReg = (ULONG*)g_pRayPos;

	// There are 4 cases how measurments may take place:
	// a) uwFr1, pRay, uwFr2 on frame A
	// b) uwFr1, pRay on frame A; uwFr2 on frame B
	// c) uwFr1 on frame A; pRay, uwFr2 on frame B
	// d) uwFr2, pRay, uwFr2 on frame B
	// So if pRay took place at low Y pos, it must be on frame B so use uwFr2,
	// Otherwise, pRay took place on A, so use uwFr1
	uwFr1 = g_sTimerManager.uwFrameCounter;
	*pRay = *pReg;
	uwFr2 = g_sTimerManager.uwFrameCounter;
	if(sRay.bfPosY < 100)
		return (uwFr2*160*313 + sRay.bfPosY*160 + sRay.bfPosX);
	else
		return (uwFr1*160*313 + sRay.bfPosY*160 + sRay.bfPosX);
	#else
	return 0;
	#endif
}

/**
 * Gets time difference between two times
 * For use on both precise and frame time
 */
ULONG timerGetDelta(ULONG ulStart, ULONG ulStop) {
	if(ulStop >= ulStart)
		return ulStop-ulStart;
	return (0xFFFFFFFF - ulStart) + ulStop;
}

/**
 * Returns if timer has passed without updating its state
 */
BYTE timerPeek(ULONG *pTimer, ULONG ulTimerDelay) {
	return (*pTimer + ulTimerDelay) <= g_sTimerManager.ulGameTicks;
}

/**
 * Returns if timer has passed
 * If passed, its state gets resetted and countdown starts again
 */
BYTE timerCheck(ULONG *pTimer, ULONG ulTimerDelay) {
	if (timerPeek(pTimer, ulTimerDelay)) {
		*pTimer = g_sTimerManager.ulGameTicks;
		return 1;
	}
	return 0;
}

/**
 * Updates game ticks if game not paused
 */
void timerProcess(void) {
	ULONG ulCurrentTime;

	ulCurrentTime = timerGet();
	if(!g_sTimerManager.ubPaused) {
		if(ulCurrentTime > g_sTimerManager.ulLastTime)
			g_sTimerManager.ulGameTicks += ulCurrentTime - g_sTimerManager.ulLastTime;
		else
			g_sTimerManager.ulGameTicks += (0xFFFF - g_sTimerManager.ulLastTime) + ulCurrentTime + 1;
	}
	g_sTimerManager.ulLastTime = ulCurrentTime;
}

/**
 * Formats precise time to human readable form on supplied buffer
 * Current version works correctly only on ulPrecTime < 0xFFFFFFFF/4 (7 min)
 * and there seems to be no easy fix for this
 */
void timerFormatPrec(char *szBfr, ULONG ulPrecTime) {
	ULONG ulResult, ulRest;
	if(ulPrecTime > 0xFFFFFFFF>>2) {
		sprintf(szBfr, ">7min");
		return;
	}
	// ulResult [us]
	ulResult = ulPrecTime*4;
	ulRest = ulResult % 10;
	ulResult = ulResult / 10;
	if(ulResult < 1000) {
		sprintf(szBfr, "%3lu.%01lu us", ulResult, ulRest);
		return;
	}
	// ulResult [ms]
	ulRest = ulResult % 1000;
	ulResult /= 1000;
	if(ulResult < 1000) {
		sprintf(szBfr, "%3lu.%03lu ms", ulResult, ulRest);
		return;
	}
	// ulResult [s]
	ulRest = ulResult % 1000;
	ulResult /= 1000;
	sprintf(szBfr, "%lu.%03lu s", ulResult, ulRest);
}

void timerWaitUs(UWORD uwUsCnt) {
	// timerGetPrec(): One tick equals: PAL - 0.40us, NTSC - 0.45us
	ULONG ulStart = timerGetPrec();
	UWORD uwTickCnt = uwUsCnt*2/5;
	while(timerGetPrec() - ulStart < uwTickCnt);
}
