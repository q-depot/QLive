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
		mIsPlaying	= false;
		
		mLive->addModule(this);
	};
	
	~Modulo() {
		mLive->removeModule(this);
	};
	
	virtual void render() {};
	
	virtual void update() {
		
		ci::app::console() << "super update!" << std::endl;
	};
	
	void play() {
		mLive->playClip(mClip->getTrackIndex() , mClip->getIndex() );
		mIsPlaying = true;
		
		ci::app::console() << "play!" << std::endl;

	};
	
	void stop() {
		mLive->stopClip(mClip->getTrackIndex() , mClip->getIndex() );
		mIsPlaying = false;
	};
	
	bool updateState() 
	{
		ClipState state = mClip->getState();
		
		if ( state == HAS_CLIP )
			mIsPlaying = false;
		else if ( state == CLIP_PLAYING )
			mIsPlaying = true;
				
		return mIsPlaying && mLive->isPlaying();
	};
	
	bool isPlaying() { return mIsPlaying && mLive->isPlaying(); };
	
protected:
	Live		*mLive;
	LiveClip	*mClip;
	
	bool		mIsPlaying;
	
};