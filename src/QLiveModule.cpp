/*
 *  QLiveModule.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#include "cinder/app/AppBasic.h"
#include "QLive.h"

namespace nocte {
    
    
    QLiveModule::QLiveModule( QLive *live, QLiveClip *clip) 
    {
        mLive		= live;
        mClip		= clip;
        
        updateState();
        updateBrightness();
    };
    
    bool QLiveModule::updateState() 
    {
        if ( mClip == NULL )
            return false;
        
        ClipState state = mClip->getState();
        
        if ( state == HAS_CLIP )
            mIsClipPlaying = false;
        else if ( state == CLIP_PLAYING )
            mIsClipPlaying = true;
        
        mIsPlaying = mIsClipPlaying && mLive->isPlaying();
        
        return mIsPlaying;
    };
    
    bool QLiveModule::isPlaying() { return mIsClipPlaying && mLive->isPlaying(); };
    
    QLiveTrack*	QLiveModule::getTrack() { return mLive->getTrack( mClip->getTrackIndex() ); };
    
    void QLiveModule::updateBrightness() 
    { 
        if ( mClip == NULL ) 
            return;
        
        mTrackVolume = pow( getTrack()->getVolume(), 2); 
    }
    
    int			QLiveModule::getTrackIndex() { return mClip->getTrackIndex(); };
    
}
