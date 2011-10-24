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

#define	GET_INFO_MIN_DELAY	2.0f		// minimum delay between info requests


class Modulo;


class Live {
	
	
public:
	Live(std::string osc_host, int osc_in_port, int osc_out_port, bool debugMode, bool init = false);
	
	void sendMessage(std::string address, std::string args = "");
	
	bool getInfo();

	void play(bool isPlaying = true) { mIsPlaying = isPlaying; };
	
	void render();
	
	bool isDebugMode() { return mDebugMode; };
	
	void debug(bool debugMode = true) { mDebugMode = debugMode; };
	
//	void play( bool playContinue = false ) { if ( playContinue ) sendMessage("/live/play/continue"); else sendMessage("/live/play"); };
	
//	void stop() { sendMessage("/live/stop"); };
	
	void playClip(int track, int clip) { 
		sendMessage("/live/play/clip", "i" + ci::toString(track) + " i" + ci::toString(clip) ); 
		ci::app::console() << "play clip!" << std::endl;
	};
	void stopClip(int track, int clip) { sendMessage("/live/stop/clip", "i" + ci::toString(track) + " i" + ci::toString(clip) ); 
		ci::app::console() << "stop clip!" << std::endl;
	
	};
	
	void stopTrack(int track) { sendMessage("/live/stop/track", "i" + ci::toString(track) );
	
		ci::app::console() << "stop track!" << std::endl;
	};
	
//	void setTrackName(int index, std::string name) { sendMessage("/live/name/track", "i" + ci::toString(index) + " s" + name ); };
	
//	void setClip() { // /live/name/clip         (int track, int clip, string name)              Sets clip number clip in track number track's name to name };
	
//	void playScene(int scene) {	sendMessage("/live/play/scene", "i" + ci::toString(scene) ); };
	
	LiveClip* getClip(int n) { return mClips[n]; };
	int getClipsN() { return mClips.size(); };
	
	LiveTrack* getTrack(int n) { return mTracks[n]; };
	int getTracksN() { return mTracks.size(); };
	
	LiveScene* getScene(int n) { return mScenes[n]; };
	int getScenesN() { return mScenes.size(); };
	
	LiveDevice* getDevice(int n) { return mDevices[n]; };
	int getDevicesN() { return mDevices.size(); };
	
	bool isPlaying() { return mIsPlaying; };
	
	LiveTrack*	getSelectedTrack() { return mSelectedTrack; };
	
	void	shutdown();
	
	bool	isOscListenerConnected()	{ return mOscListener != NULL; };
	
	bool	isOscSenderConnected()		{ return mOscSender != NULL; };	
	
	void	initOsc();
	
	
private:
	void	listTrackDevices(int trackIndex) { sendMessage("/live/devicelist", "i" + ci::toString(trackIndex) ); };
	
	void	receiveData();
	
	void	drawDebug();
	
	void	deleteObjects();
	
	void	parseScene( ci::osc::Message message );
	void	parseTrack( ci::osc::Message message );
	void	parseClip( ci::osc::Message message );
	void	parseClipInfo( ci::osc::Message message );
	void	parseDevice( ci::osc::Message message );
//	void	parseDeviceParams( ci::osc::Message message );

	std::vector<LiveTrack*>			mTracks;
	std::vector<LiveScene*>			mScenes;
	std::vector<LiveClip*>			mClips;
	std::vector<LiveDevice*>		mDevices;
	
	ci::osc::Sender					*mOscSender;
	ci::osc::Listener				*mOscListener;
	
	bool							mIsPlaying;
	
	std::string						mOscHost;
	int								mOscInPort;
	int								mOscOutPort;
	
	
	ci::ColorA						mColorWhite;
	ci::ColorA						mColor1;
	ci::ColorA						mColor2;
	ci::ColorA						mColor3;
	ci::ColorA						mColor4;

	ci::Font						mFontSmall;
	
	double							mGetInfoRequestAt;
	
	bool							mDebugMode;
	
	LiveTrack						*mSelectedTrack;
};

