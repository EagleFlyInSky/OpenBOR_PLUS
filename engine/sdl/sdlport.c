/*
 * OpenBOR - http://www.chronocrash.com
 * -----------------------------------------------------------------------
 * All rights reserved, see LICENSE in OpenBOR root for details.
 *
 * Copyright (c) 2004 - 2014 OpenBOR Team
 */

#include "sdlport.h"
#include "packfile.h"
#include "ram.h"
#include "video.h"
#include "menu.h"
#ifdef PS3
#include <lv2/sysfs.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/process.h>
#else
#include <time.h>
#endif
#include <unistd.h>

#undef usleep

#ifdef DARWIN
#include <CoreFoundation/CoreFoundation.h>
#elif WIN
#undef main
#endif

#ifdef PS3
#define SYS_PROCESS_PARAM_OPENBOR(prio,stacksize) \
	sys_process_param_t __sys_process_param __attribute__((aligned(8), section(".sys_proc_param"), unused)) = { \
		sizeof(sys_process_param_t), \
		SYS_PROCESS_SPAWN_MAGIC, \
		SYS_PROCESS_SPAWN_VERSION_330, \
		SYS_PROCESS_SPAWN_FW_VERSION_330, \
		prio, \
		stacksize, \
		SYS_PROCESS_SPAWN_MALLOC_PAGE_SIZE_1M, \
		SYS_PROCESS_SPAWN_PPC_SEG_DEFAULT\
	};

SYS_PROCESS_PARAM_OPENBOR(1001, 0x00100000)
#endif

char packfile[MAX_FILENAME_LEN] = {"bor.pak"};
#if ANDROID || PS3
char rootDir[MAX_BUFFER_LEN] = {""};
#endif
char paksDir[MAX_FILENAME_LEN] = {"Paks"};
char savesDir[MAX_FILENAME_LEN] = {"Saves"};
char logsDir[MAX_FILENAME_LEN] = {"Logs"};
char screenShotsDir[MAX_FILENAME_LEN] = {"ScreenShots"};

// sleeps for the given number of microseconds
#if _POSIX_C_SOURCE >= 199309L
void _usleep(u32 usec)
{
    struct timespec sleeptime;
    sleeptime.tv_sec = usec / 1000000LL;
    sleeptime.tv_nsec = (usec % 1000000LL) * 1000;
    nanosleep(&sleeptime, NULL);
}
#endif

#if ANDROID
char* AndroidRoot(char *relPath)
{
	static char filename[MAX_FILENAME_LEN];
	strcpy(filename, rootDir);
	strcat(filename, relPath);
	return filename;
}
#endif

void borExit(int reset)
{
#ifdef GP2X
	gp2x_end();
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
#elif SDL
	//SDL_Delay(1000);
	SDL_Quit(); // Call this instead of atexit(SDL_Quit); It's best practice!
#endif

#ifdef DMALLOC_MODE
PRINT_DMALLOC_INFO;
#endif

    exit(reset);
}

int main(int argc, char *argv[])
{
#ifndef SKIP_CODE
	char pakname[MAX_FILENAME_LEN] = {""};
#endif
#ifdef CUSTOM_SIGNAL_HANDLER
	struct sigaction sigact;
#endif

#ifdef DARWIN
	char resourcePath[PATH_MAX] = {""};
	CFBundleRef mainBundle;
	CFURLRef resourcesDirectoryURL;
	mainBundle = CFBundleGetMainBundle();
	resourcesDirectoryURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	if(!CFURLGetFileSystemRepresentation(resourcesDirectoryURL, true, (UInt8 *) resourcePath, PATH_MAX))
	{
		borExit(0);
	}
	CFRelease(resourcesDirectoryURL);
	chdir(resourcePath);
#endif

#ifdef CUSTOM_SIGNAL_HANDLER
	sigact.sa_sigaction = handleFatalSignal;
	sigact.sa_flags = SA_RESTART | SA_SIGINFO;

	if(sigaction(SIGSEGV, &sigact, NULL) != 0)
	{
		printf("Error setting signal handler for %d (%s)\n", SIGSEGV, strsignal(SIGSEGV));
		borExit(EXIT_FAILURE);
	}
#endif

	setSystemRam();
	initSDL();

	packfile_mode(0);

#ifdef ANDROID
    if(strstr(SDL_AndroidGetExternalStoragePath(), "org.openbor.engine"))
    {
        strcpy(rootDir, "/mnt/sdcard/OpenBOR/");
        strcpy(paksDir, "/mnt/sdcard/OpenBOR/Paks");
        strcpy(savesDir, "/mnt/sdcard/OpenBOR/Saves");
        strcpy(logsDir, "/mnt/sdcard/OpenBOR/Logs");
        strcpy(screenShotsDir, "/mnt/sdcard/OpenBOR/ScreenShots");
    }
    else
    {
        strcpy(rootDir, SDL_AndroidGetExternalStoragePath());
        strcat(rootDir, "/");
        strcpy(paksDir, SDL_AndroidGetExternalStoragePath());
        strcat(paksDir, "/Paks");
        strcpy(savesDir, SDL_AndroidGetExternalStoragePath());
        strcat(savesDir, "/Saves");
        strcpy(logsDir, SDL_AndroidGetExternalStoragePath());
        strcat(logsDir, "/Logs");
        strcpy(screenShotsDir, SDL_AndroidGetExternalStoragePath());
        strcat(screenShotsDir, "/ScreenShots");
    }
	dirExists(rootDir, 1);
    chdir(rootDir);
#elif PS3
    strcpy(rootDir, "/dev_hdd0/OpenBOR");
    strcpy(paksDir, "/dev_hdd0/OpenBOR/Paks");
    strcpy(savesDir, "/dev_hdd0/OpenBOR/Saves");
    strcpy(logsDir, "/dev_hdd0/OpenBOR/Logs");
    strcpy(screenShotsDir, "/dev_hdd0/OpenBOR/ScreenShots");

    //sysFsChmod("/dev_hdd0/", S_IRWXO | S_IRWXU | S_IRWXG | S_IFDIR);
    //chdir("/dev_hdd0");
	dirExists("/dev_hdd0/OpenBOR", 1);
    //chdir(rootDir);
#endif

	dirExists(paksDir, 1);
	dirExists(savesDir, 1);
	dirExists(logsDir, 1);
	dirExists(screenShotsDir, 1);

	Menu();
#ifndef SKIP_CODE
	getPakName(pakname, -1);
    video_set_window_title(pakname);
#endif
	openborMain(argc, argv);
	borExit(0);
	return 0;
}

