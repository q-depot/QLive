/*
 *  QLiveModule.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#pragma once

#include "cinder/app/AppBasic.h"
//#include "QLive.h"

namespace nocte {
    
    class QLive;
    class QLiveClip;
    class QLiveTrack;
    
class QLiveModule {

public:
	
	QLiveModule() {}
	
	QLiveModule( QLive *live, QLiveClip *clip );
    
	~QLiveModule() {}
	
	virtual void render() {}
	
	virtual void update( float *values, float masterBrightness, float baseBrightness ) {}
	
    bool updateModulo() 
    {
        updateBrightness();
        
        return updateState();
    }
    
	void play() { mIsClipPlaying = true; }
	
	void stop() { mIsClipPlaying = false; }
	
	bool updateState();
	
	bool isPlaying();
    
	QLiveClip*	getClip() { return mClip; }
	
	QLiveTrack*	getTrack();
    
	void		updateBrightness();
    
	float		getTrackVolume() { return mTrackVolume; }
	
	std::string getSettings() { return mSettingsStr; }
	
	std::string	getName() { return mName; }
	
	int			getTrackIndex();

protected:
    
	QLive			*mLive;
	QLiveClip		*mClip;
	
	bool			mIsPlaying;			// true when both clip and Live are playing
	bool			mIsClipPlaying;		// true when clip is playing
	
	float			mTrackVolume;
	
	std::string		mSettingsStr;
	
	std::string		mName;

};


}