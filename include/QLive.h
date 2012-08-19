/*
 *  QLive.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "OscSender.h"
#include "OscListener.h"
#include "cinder/Utilities.h"
#include "QLiveObject.h"
#include "QLiveModule.h"
#include "QLiveAnalyzer.h"


#define	GET_INFO_MIN_DELAY	2.0f		// minimum delay between info requests


namespace nocte {
    
//class QLiveObject;
//class QLiveAnalyzer;
//class QLiveModule;
//class QLiveClip;
//class QLiveScene;
//class QLiveDevice;
//class QLiveTrack;

class QLive {
	
public:
    
	QLive( std::string osc_host = "localhost", int osc_live_in_port = 9001, int osc_live_out_port = 9000, int osc_analyzer_in_port = 8000, bool init = false );
	
	void sendMessage(std::string address, std::string args = "");
	
	bool getInfo();

	void play(bool isPlaying = true) { mIsPlaying = isPlaying; };
	
    void renderDebug();
    
    void renderAnalyzer();
    
//    void renderAnalyzer()
//    {
//        mAnalyzer->render();
//    }
    
//	bool isDebugMode() { return mDebugMode; };
	
//	void debug(bool debugMode = true) { mDebugMode = debugMode; };
	
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
	
	QLiveClip* getClip(int n) { return mClips[n]; };
	int getClipsN() { return mClips.size(); };
	
	QLiveTrack* getTrack(int n) { return mTracks[n]; };
	int getTracksN() { return mTracks.size(); };
	
	QLiveScene* getScene(int n) { return mScenes[n]; };
	int getScenesN() { return mScenes.size(); };
	
	QLiveDevice* getDevice(int n) { return mDevices[n]; };
	int getDevicesN() { return mDevices.size(); };
	
	bool isPlaying() { return mIsPlaying; };
	
	QLiveTrack*	getSelectedTrack() { return mSelectedTrack; };
	
	void	shutdown();
	
	bool	isOscListenerConnected()	{ return mOscListener != NULL; };
	
	bool	isOscSenderConnected()		{ return mOscSender != NULL; };	
	
	void	initOsc();
	
	float	getFreqAmplitude(int freq, int channel);
    
    float* getFftBuffer(int channel);
    float* getAmplitudeRef(int channel);
    float* getAmplitudeGainRef() { return &mAnalyzer->mAmplitudeGain; }
    float* getFftGainRef() { return &mAnalyzer->mFftGain; }
    float* getFftOffsetRef() { return &mAnalyzer->mFftOffset; }
    float* getFftDumpingRef() { return &mAnalyzer->mFftDumping; }
    
    void getParams() 
    {
        sendMessage( "/live/devicelist" );
//        sendMessage("/live/play/scene", "i" + ci::toString(scene) ); 
    };
    
    
//    98	/live/devicelist        (int track)                                     Returns a list of all devices and names on track number track as: /live/device (int track, int device, str name, ...)
//    99	/live/device            (int track, int device)                         Returns a list of all parameter values and names on device on track number track
private:
    
	void	listTrackDevices(int trackIndex) { sendMessage("/live/devicelist", "i" + ci::toString(trackIndex) ); };
	
	void	receiveData();
	
	void	deleteObjects();
	
	void	parseScene( ci::osc::Message message );
	void	parseTrack( ci::osc::Message message );
	void	parseClip( ci::osc::Message message );
	void	parseClipInfo( ci::osc::Message message );
	void	parseDevice( ci::osc::Message message );
	void	parseDeviceParams( ci::osc::Message message );

	std::vector<QLiveTrack*>		mTracks;
	std::vector<QLiveScene*>		mScenes;
	std::vector<QLiveClip*>			mClips;
	std::vector<QLiveDevice*>		mDevices;
	
    QLiveAnalyzer                   *mAnalyzer;
    
	ci::osc::Sender					*mOscSender;
	ci::osc::Listener				*mOscListener;
	
	bool							mIsPlaying;
	
	std::string						mOscHost;
	int								mOscLiveInPort;
	int								mOscLiveOutPort;
	int                             mOscAnalyzerInPort;
	
	ci::ColorA						mColor1;
	ci::ColorA						mColor2;
	ci::ColorA						mColor3;
	ci::ColorA						mColor4;
    
	ci::Font						mFontSmall;
	ci::Font						mFontMedium;
	
	double							mGetInfoRequestAt;
	
//	bool							mDebugMode;
	
	QLiveTrack						*mSelectedTrack;
    
};

}