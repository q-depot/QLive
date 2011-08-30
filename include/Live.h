/*
 *  Live.h
 *  QUBO
 *
 *  Created by Q on 08/08/2011.
 *  Copyright 2011 . All rights reserved.
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "OscSender.h"
#include "OscListener.h"
#include "cinder/Utilities.h"
#include "LiveObject.h"

#define	OSC_OUT_PORT	9000
#define	OSC_IN_PORT		9001
#define	OSC_HOST		"localhost"

class Modulo;

/*
class LiveTrack;
class LiveScene;
class LiveClip;
class LiveDevice;
*/

class Live {
	
	
public:
	Live();
	
	void debug();
	
	void drawDebug();
	
	void sendMessage(std::string address, std::string args = "");
	
	void getInfo();
	
	void play( bool playContinue = false ) { if ( playContinue ) sendMessage("/live/play/continue"); else sendMessage("/live/play"); };
	
	void stop() { sendMessage("/live/stop"); };
	
	void playClip(int track, int clip) { sendMessage("/live/play/clip", "i" + ci::toString(track) + " i" + ci::toString(clip) ); };
	void stopClip(int track, int clip) { sendMessage("/live/stop/clip", "i" + ci::toString(track) + " i" + ci::toString(clip) ); };
	
	void setTrackName(int index, std::string name) { sendMessage("/live/name/track", "i" + ci::toString(index) + " s" + name ); };
	
	void setClip() {
		// /live/name/clip         (int track, int clip, string name)              Sets clip number clip in track number track's name to name
		};
	
	void playScene(int scene) {	sendMessage("/live/play/scene", "i" + ci::toString(scene) ); };
	
	LiveClip* getClip(int n) { return mClips[n]; };
	int getClipsN() { return mClips.size(); };
	
	LiveTrack* getTrack(int n) { return mTracks[n]; };
	int getTracksN() { return mTracks.size(); };
	
	LiveScene* getScene(int n) { return mScenes[n]; };
	int getScenesN() { return mScenes.size(); };
	
	LiveDevice* getDevice(int n) { return mDevices[n]; };
	int getDevicesN() { return mDevices.size(); };
	
	void	addModule(Modulo *m) { mModules.push_back(m); };
	
	void	removeModule(Modulo *m) 
	{ 
		for (std::vector<Modulo*>::iterator it = mModules.begin(); it != mModules.end(); it++)
			if ( *it == m ) { mModules.erase(it); return; }
	};
	
	void clearModules() { mModules.clear(); };
	
	bool isPlaying() { return mIsPlaying; };
	
private:
	void	listTrackDevices(int trackIndex) { sendMessage("/live/devicelist", "i" + ci::toString(trackIndex) ); };
	
	void	receiveData();
	
	
	std::vector<Modulo*>			mModules;
	
	void	parseScene( ci::osc::Message message );
	void	parseTrack( ci::osc::Message message );
	void	parseClip( ci::osc::Message message );
	void	parseClipInfo( ci::osc::Message message );
	void	parseDevice( ci::osc::Message message );
	void	parseDeviceParams( ci::osc::Message message );
	
	std::vector<LiveTrack*>			mTracks;
	std::vector<LiveScene*>			mScenes;
	std::vector<LiveClip*>			mClips;
	std::vector<LiveDevice*>		mDevices;
	
	ci::osc::Sender					mOscSender;
	ci::osc::Listener				mOscListener;
	
	std::vector<ci::osc::Message>	mMessageQueue;
	
	ci::Font						mFontSmall;
	
	bool							mIsPlaying;
	
	
};