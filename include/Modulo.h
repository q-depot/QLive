/*
 *  Modulo.h
 *  QUBO
 *
 *  Created by Q on 08/08/2011.
 *  Copyright 2011 . All rights reserved.
 *
 */


#pragma once

#include "cinder/app/AppBasic.h"
#include "Live.h"


class Modulo {

public:
	
	Modulo() {};
	
	Modulo(Live *live, LiveClip *clip) {
		mLive		= live;
		mClip		= clip;
		
		updateState();
		updateBrightness();
	};
	
	~Modulo() { };
	
	virtual void render(float height) {};
	
	virtual void update(float *values) { };
	
	bool updateModulo() 
	{
		updateBrightness();
		
		return updateState();
	};
	
	void play() { mIsClipPlaying = true; };
	
	void stop() { mIsClipPlaying = false; };
	
	bool updateState() 
	{
		ClipState state = mClip->getState();
		
		if ( state == HAS_CLIP )
			mIsClipPlaying = false;
		else if ( state == CLIP_PLAYING )
			mIsClipPlaying = true;
				
		mIsPlaying = mIsClipPlaying && mLive->isPlaying();

		return mIsPlaying;
	};
	
	bool isPlaying() { return mIsClipPlaying && mLive->isPlaying(); };
	
	LiveClip*	getClip() { return mClip; };
	
	LiveTrack*	getTrack() { return mLive->getTrack( mClip->getTrackIndex() ); };
	
	void		updateBrightness() { mBrightness = pow( mLive->getTrack(mClip->getTrackIndex())->getVolume(), 2); };
	
	float		getBrightness() { return mBrightness; };
	
	std::string getSettings() { return mSettingsStr; };
	
	std::string	getName() { return mName; };
	
	int			getTrackIndex() { return mClip->getTrackIndex(); };
	

protected:
	Live			*mLive;
	LiveClip		*mClip;
	
	bool			mIsPlaying;			// true when both clip and Live are playing
	bool			mIsClipPlaying;		// true when clip is playing
	
	float			mBrightness;
	
	std::string		mSettingsStr;
	
	std::string		mName;

};

