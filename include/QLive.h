/*
 *  QLive.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QLIVE
#define QLIVE

#pragma once

#include "cinder/app/AppBasic.h"
#include "OscSender.h"
#include "OscListener.h"
#include "cinder/Utilities.h"
#include "QLiveObject.h"
#include "QLiveModule.h"
#include "QLiveModuleWithFixtures.h"
#include "QLiveAnalyzer.h"
#include "QLiveParams.h"

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
    
	QLive( std::string osc_host = "localhost", int osc_live_in_port = 9001, int osc_live_out_port = 9000, int osc_analyzer_in_port = 8000, int live_params_in_port = 9500, bool init = false );
	
	void sendMessage(std::string address, std::string args = "");
	
	bool getInfo();

	//void play(bool isPlaying = true) { mIsPlaying = isPlaying; };
	
    void renderDebug();
    
    void renderAnalyzer();
    
//    void renderAnalyzer()
//    {
//        mAnalyzer->render();
//    }
	
//	void debug(bool debugMode = true) { mDebugMode = debugMode; };
	
	void play( bool playContinue = false ) { if ( playContinue ) sendMessage("/live/play/continue"); else sendMessage("/live/play"); }
	
	void stop() { sendMessage("/live/stop"); }

	// play/stop clip, track: they should also set the local value in case Live doesn't respond
	void playClip(int track, int clip) { sendMessage("/live/play/clip", "i" + ci::toString(track) + " i" + ci::toString(clip) ); }
    
	void stopClip(int track, int clip) { sendMessage("/live/stop/clip", "i" + ci::toString(track) + " i" + ci::toString(clip) ); }
	
	void stopTrack(int track) { sendMessage("/live/stop/track", "i" + ci::toString(track) ); }
    
	void setTrackVolume( int track, float volume ) { sendMessage("/live/volume", "i" + ci::toString(track) + " f" + ci::toString(volume) ); }
	
    void setParam( int track, int device, int param, float value ) { sendMessage("/live/device", "i" + ci::toString(track) + " i" + ci::toString(device) + " i" + ci::toString(param) + " f" + ci::toString(value) ); }
    
//	void setTrackName(int index, std::string name) { sendMessage("/live/name/track", "i" + ci::toString(index) + " s" + name ); };
	
//	void setClip() { // /live/name/clip         (int track, int clip, string name)              Sets clip number clip in track number track's name to name };
	
//	void playScene(int scene) {	sendMessage("/live/play/scene", "i" + ci::toString(scene) ); };
	
	QLiveClip* getClip(int n) { return mClips[n]; }
	int getClipsN() { return mClips.size(); }
	
	QLiveTrack* getTrack(int n) { return mTracks[n]; }
	int getTracksN() { return mTracks.size(); }
	
	QLiveScene* getScene(int n) { return mScenes[n]; }
	int getScenesN() { return mScenes.size(); }
	
	QLiveDevice* getDevice(int n) { return mDevices[n]; }
	int getDevicesN() { return mDevices.size(); }
	
	bool isPlaying() { return mIsPlaying; }

	bool* getIsPlayingRef() { return &mIsPlaying; }
	
	QLiveTrack*	getSelectedTrack() { return mSelectedTrack; }
	
	void	shutdown();
	
	bool	isOscListenerConnected()	{ return mOscListener != NULL; }
	
	bool	isOscSenderConnected()		{ return mOscSender != NULL; }
	
	void	initOsc();
	
	float	getFreqAmplitude(int freq, int channel);
    
    float* getFftBuffer(int channel);
    float* getAmplitudeRef(int channel);
    float* getAmplitudeGainRef() { return &mAnalyzer->mAmplitudeGain; }
    float* getFftGainRef() { return &mAnalyzer->mFftGain; }
    float* getFftOffsetRef() { return &mAnalyzer->mFftOffset; }
    float* getFftDumpingRef() { return &mAnalyzer->mFftDumping; }

    bool isReady() { return mIsReady; }
    
    float getParam( std::string name ) { return mLiveParams->getParam(name); }
    
    float* getParamRef( std::string name ) { return mLiveParams->getParamRef(name); }

    void saveSettings( std::vector<QLiveModuleWithFixtures*> modules );
    
    void loadSettings( std::vector<QLiveModuleWithFixtures*> modules );
    
private:
    
	void	listTrackDevices(int trackIndex) { sendMessage("/live/devicelist", "i" + ci::toString(trackIndex) ); }
	
	void	receiveData();
	
	void	deleteObjects();
	
    ci::ColorA colorIntToColorA( int colorInt )
    {
        ci::ColorA color;
        color.r = (float)((colorInt >> 16) & 0xB0000FF) / 255.0f;
        color.g = (float)((colorInt >> 8) & 0xB0000FF) / 255.0f;
        color.b = (float)(colorInt & 0xB0000FF) / 255.0f;
        color.a	= 1.0f;
        
        return color;
    }
    
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
    QLiveParams                     *mLiveParams;
    
	ci::osc::Sender					*mOscSender;
	ci::osc::Listener				*mOscListener;
	
	bool							mIsPlaying;
	
	std::string						mOscHost;
	int								mOscLiveInPort;
	int								mOscLiveOutPort;
	int                             mOscAnalyzerInPort;
	int                             mOscLiveParamsInPort;
    
	ci::ColorA						mColor1;
	ci::ColorA						mColor2;
	ci::ColorA						mColor3;
	ci::ColorA						mColor4;
    
	ci::Font						mFontSmall;
	ci::Font						mFontMedium;
	
	double							mGetInfoRequestAt;
	
//	bool							mDebugMode;
	
	QLiveTrack						*mSelectedTrack;
    
    bool                            mIsReady;
};

}

#endif