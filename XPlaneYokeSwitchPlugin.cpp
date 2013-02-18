
/*
 * XPlaneYokeSwitchPlugin.cpp
 * https://github.com/dankrusi/XPlaneYokeSwitchPlugin
 * Copyright (c) 2013 Dan Krusi, licensed under GNU GPLv3
 * 
 * An X-Plane plugin to switch/change all yoke/jostick 
 * settings between two sets of hardware.
 * 
 */

#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>

#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"

// Constants
#define MAX_JOYSTICK_ASSIGNMENTS 100

// Variables
static XPLMHotKeyID	gHotKeyToggleYokes = NULL;
static XPLMHotKeyID	gHotKeySetConfigurationA = NULL;
static XPLMHotKeyID	gHotKeySetConfigurationB = NULL;
static XPLMDataRef gDataRef = NULL;
static int gJoystickAssignmentsA[MAX_JOYSTICK_ASSIGNMENTS];
static int gJoystickAssignmentsB[MAX_JOYSTICK_ASSIGNMENTS];
static int *gJoystickAssignmentsCurrent;
//static char gPreferencesPath[512];
static std::ostringstream gPreferencesPath;

// Forward declares
void YokeSwitchPluginToggleYokes(void *inRefcon); 
void YokeSwitchPluginSetConfigurationA(void *inRefcon);  
void YokeSwitchPluginSetConfigurationB(void *inRefcon);  
void YokeSwitchPluginMenuCallBack(void *inMenuRef, void *inItemRef);
bool YokeSwitchPluginSaveConfiguration();
bool YokeSwitchPluginLoadConfiguration();

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
	// Set plugin info
	strcpy(outName, "YokeSwitchPlugin");
	strcpy(outSig, "com.dankrusi.YokeSwitchPlugin");
	strcpy(outDesc, "Allows you to switch betweet different yokes using a hotkey.");
	
	// Get path to preferences file
	char prefDir[512];
	XPLMGetPrefsPath(prefDir);
	XPLMExtractFileAndPath(prefDir);
	gPreferencesPath << prefDir << XPLMGetDirectorySeparator() << "YokeSwitchPlugin.prf";

	// Create menu
	XPLMMenuID	myMenu;
	int			mySubMenuItem;
	
	// First we put a new menu item into the plugin menu.
	mySubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Yoke Switch", 0, 1);						
	
	// Now create a submenu attached to our menu item.
	myMenu = XPLMCreateMenu("Yoke Switch", XPLMFindPluginsMenu(), mySubMenuItem, YokeSwitchPluginMenuCallBack, 0);						
						
	// Append a few menu items to our submenu.  
	XPLMAppendMenuItem(myMenu,"Save current configuration to A",(void*)1,1);
	XPLMAppendMenuItem(myMenu,"Save current configuration to B",(void*)2,1);

	// Hot keys
	//TODO: Save/load this configuration. It's bull shit X-Plane doesnt do this.
	gHotKeySetConfigurationB = XPLMRegisterHotKey(XPLM_VK_F12, xplm_DownFlag, "Set conf. B",YokeSwitchPluginSetConfigurationB,NULL);
	gHotKeySetConfigurationA = XPLMRegisterHotKey(XPLM_VK_F11, xplm_DownFlag, "Set conf. A",YokeSwitchPluginSetConfigurationA,NULL);
	gHotKeyToggleYokes = XPLMRegisterHotKey(XPLM_VK_F10, xplm_DownFlag, "Toggle Yokes",YokeSwitchPluginToggleYokes,NULL);
	
	// Look up our data ref.  You find the string name of the data ref
	gDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
	
	// Initial selection
	gJoystickAssignmentsCurrent = gJoystickAssignmentsA;

	// Load preferences, if any
	if(YokeSwitchPluginLoadConfiguration()) {
		// Update to current
		XPLMSetDatavi(gDataRef,gJoystickAssignmentsCurrent,0,MAX_JOYSTICK_ASSIGNMENTS);
	}
	
	return 1;
}

PLUGIN_API void	XPluginStop(void) {
	XPLMUnregisterHotKey(gHotKeyToggleYokes);
	XPLMUnregisterHotKey(gHotKeySetConfigurationA);
	XPLMUnregisterHotKey(gHotKeySetConfigurationB);
}

PLUGIN_API void XPluginDisable(void) {
}

PLUGIN_API int XPluginEnable(void) {
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, int inMessage, void *inParam) {
}





bool YokeSwitchPluginSaveConfiguration() {
	int ok = 0;
	FILE* fp = fopen(gPreferencesPath.str().c_str(), "wb");
	if (fp != NULL) {
		//TODO: check for ok's
		ok = fwrite(gJoystickAssignmentsA, sizeof(int), MAX_JOYSTICK_ASSIGNMENTS, fp) == 1;
		ok = fwrite(gJoystickAssignmentsB, sizeof(int), MAX_JOYSTICK_ASSIGNMENTS, fp) == 1;
		fclose(fp);
		return true;
	}
	return false;
}

bool YokeSwitchPluginLoadConfiguration() {
	int bytes;
	FILE* fp = fopen(gPreferencesPath.str().c_str(), "r");
	if (fp != NULL) {
		//TODO: check bytes written
		bytes = fread(gJoystickAssignmentsA, sizeof(int), MAX_JOYSTICK_ASSIGNMENTS, fp);
		bytes = fread(gJoystickAssignmentsB, sizeof(int), MAX_JOYSTICK_ASSIGNMENTS, fp);
		fclose(fp);
		return true;
	}
	return false;
}
 
void YokeSwitchPluginSetConfigurationA(void *inRefcon) {
	gJoystickAssignmentsCurrent = gJoystickAssignmentsA;
	XPLMSetDatavi(gDataRef,gJoystickAssignmentsCurrent,0,MAX_JOYSTICK_ASSIGNMENTS);
}
 
void YokeSwitchPluginSetConfigurationB(void *inRefcon) {
	gJoystickAssignmentsCurrent = gJoystickAssignmentsB;
	XPLMSetDatavi(gDataRef,gJoystickAssignmentsCurrent,0,MAX_JOYSTICK_ASSIGNMENTS);
}

void YokeSwitchPluginToggleYokes(void *inRefcon) {
	// Toggle
	if(gJoystickAssignmentsCurrent == gJoystickAssignmentsA) 
		gJoystickAssignmentsCurrent = gJoystickAssignmentsB;
	else	
		gJoystickAssignmentsCurrent = gJoystickAssignmentsA;
	
	// Update to current
	XPLMSetDatavi(gDataRef,gJoystickAssignmentsCurrent,0,MAX_JOYSTICK_ASSIGNMENTS);
}

void YokeSwitchPluginMenuCallBack(void *inMenuRef, void *inItemRef) {
	// Switch pointer to settings based on which menu was pressed and load in all values
	intptr_t menuItem = (intptr_t)inItemRef;
	std::ostringstream msg;
	msg << "Saved current yoke settings to configuration ";
	if(menuItem == 1) {
		int ret = XPLMGetDatavi(gDataRef,gJoystickAssignmentsA,0,MAX_JOYSTICK_ASSIGNMENTS);
		gJoystickAssignmentsCurrent = gJoystickAssignmentsA;
		msg << "A";
	} else if (menuItem == 2) {
		int ret = XPLMGetDatavi(gDataRef,gJoystickAssignmentsB,0,MAX_JOYSTICK_ASSIGNMENTS);
		gJoystickAssignmentsCurrent = gJoystickAssignmentsB;
		msg << "B";
	}
	// Try to save configuration
	if(YokeSwitchPluginSaveConfiguration()) {
		XPLMSpeakString(const_cast<char*>(msg.str().c_str()));
	} else {
		std::ostringstream error;
		error << "Could not save configuration to file " << gPreferencesPath;
		XPLMSpeakString(const_cast<char*>(error.str().c_str()));
	}
}

