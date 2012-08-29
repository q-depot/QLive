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
//#include "QLiveModuleWithFixtures.h"
#include "QLiveAnalyzer.h"

#define	GET_INFO_MIN_DELAY	2.0f		// minimum delay between info requests


namespace nocte {

    

class QLive {
	
public:
    
	QLive( std::string osc_host = "localhost", int osc_live_in_port = 9001, int osc_live_out_port = 9000, int osc_analyzer_in_port = 8000, int live_params_in_port = 9500, bool init = true );
    
    ~QLive() {}

	void sendMessage(std::string address, std::string args = "");
	
	bool getInfo();

    void renderDebug();
    
    void renderAnalyzer();
	
	void play( bool playContinue = false ) { if ( playContinue ) sendMessage("/live/play/continue"); else sendMessage("/live/play"); }
	
	void stop() { sendMessage("/live/stop"); }

	void playClip( int trackIdx, int clipIdx ) 
    { 
        QLiveTrack *track = getTrack( trackIdx );
        
        if ( !track )
            return;
        
        QLiveClip *clip = track->getClip(clipIdx);
        
        if ( !clip )
            return;
        
        if ( clip->isNull() )
            clip->setState( CLIP_PLAYING );
        else
            sendMessage("/live/play/clip", "i" + ci::toString(trackIdx) + " i" + ci::toString(clipIdx) );
    }
    
	void stopClip( int trackIdx, int clipIdx ) 
    {
        QLiveTrack *track = getTrack( trackIdx );
        
        if ( !track )
            return;
        
        QLiveClip *clip = track->getClip(clipIdx);
        
        if ( !clip )
            return;
        
        if ( clip->isNull() )
            clip->setState( HAS_CLIP );
        else
            sendMessage("/live/stop/clip", "i" + ci::toString(track) + " i" + ci::toString(clip) ); 
    }
	
	void stopTrack(int track) { sendMessage("/live/stop/track", "i" + ci::toString(track) ); }
    
	void setTrackVolume( int track, float volume ) { sendMessage("/live/volume", "i" + ci::toString(track) + " f" + ci::toString(volume) ); }
	
    void setParam( int track, int device, int param, float value ) { sendMessage("/live/device", "i" + ci::toString(track) + " i" + ci::toString(device) + " i" + ci::toString(param) + " f" + ci::toString(value) ); }
    
//	void setTrackName(int index, std::string name) { sendMessage("/live/name/track", "i" + ci::toString(index) + " s" + name ); };
	
//	void setClip() { // /live/name/clip         (int track, int clip, string name)              Sets clip number clip in track number track's name to name };
	
//	void playScene(int scene) {	sendMessage("/live/play/scene", "i" + ci::toString(scene) ); };
    
    std::vector<QLiveTrack*> getTracks() { return mTracks; }
    
    std::vector<QLiveScene*> getScenes() { return mScenes; }
    
	QLiveClip* getClip( int trackIdx, int clipIdx )
    { 
        QLiveClip* clip = NULL;
        
        for( size_t k=0; k < mTracks.size(); k++ )
            if ( mTracks[k]->mIndex == trackIdx )
            {
                for( size_t j=0; j < mTracks[k]->mDevices.size(); j++ )
                    if ( mTracks[k]->mClips[j]->mIndex == clipIdx )
                        return mTracks[k]->mClips[j];
            }

        return clip;
    }
	
	QLiveTrack* getTrack( int trackIdx ) 
    { 
        QLiveTrack* track = NULL;
        
        for( size_t k=0; k < mTracks.size(); k++ )
            if ( mTracks[k]->mIndex == trackIdx )
                return mTracks[k];
                
        return track;
    }
	
	QLiveScene* getScene( int sceneIdx )
    { 
        QLiveScene* scene = NULL;
        
        for( size_t k=0; k < mScenes.size(); k++ )
            if ( mScenes[k]->mIndex == sceneIdx )
                return mScenes[k];
        
        return scene; 
    }
	
	QLiveDevice* getDevice( int trackIdx, int deviceIdx ) 
    {         
        QLiveDevice* device = NULL;
        
        for( size_t k=0; k < mTracks.size(); k++ )
            if ( mTracks[k]->mIndex == trackIdx )
            {
                for( size_t j=0; j < mTracks[k]->mDevices.size(); j++ )
                    if ( mTracks[k]->mDevices[j]->mIndex == deviceIdx )
                        return mTracks[k]->mDevices[j];
            }
        
        return device; 
    }
    
	float getParamValue( int trackIdx, int deviceIdx, std::string name ) 
    { 
        QLiveTrack  *track  = getTrack( trackIdx );
        QLiveDevice *device;
        
        if ( track )
        {
            device = track->getDevice( deviceIdx );
            if ( device )
                return device->getParamValue( name );
        }
        
        return 0;
    }
    
    float* getParamRef( int trackIdx, int deviceIdx, std::string name ) 
    { 
        float       *ref    = NULL;
        QLiveTrack  *track  = getTrack( trackIdx );
        QLiveDevice *device;
        
        if ( track )
        {
            device = track->getDevice( deviceIdx );
            if ( device )
                return device->getParamRef( name );
        }
        
        return ref;
    }
    
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
    
    QLiveTrack* addNullTrack( int trackIdx, const std::string &name ) 
    { 
        QLiveTrack *track = new QLiveTrack( trackIdx, name, ci::Color::white(), true );
        mTracks.push_back( track ); 

        return track;
    }
    
    void loadSettings( const std::string &filename, bool forceXmlSettings = false );
    
    void saveSettings( const std::string &filename );
//    bool isEmpty() { return false; }
    
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
	void	parseDeviceList( ci::osc::Message message );
	void	parseDeviceAllParams( ci::osc::Message message );
	void	parseDeviceParam( ci::osc::Message message );

    void    debugOscMessage( ci::osc::Message message );
    
	std::vector<QLiveTrack*>		mTracks;
	std::vector<QLiveScene*>		mScenes;
	
    QLiveAnalyzer                   *mAnalyzer;
    
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
	
	QLiveTrack						*mSelectedTrack;
    
    bool                            mIsReady;
};

}

#endif