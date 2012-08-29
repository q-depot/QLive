/*
 *  QLiveModule.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef QLIVE_MODULE
#define QLIVE_MODULE

#pragma once

//#include "cinder/app/AppBasic.h"
#include "cinder/Xml.h"
#include "QLive.h"

namespace nocte {
    
    class QLive;
    class QLiveClip;
    class QLiveTrack;
        
    class QLiveModule {

    public:
        
        QLiveModule() {}
        
        QLiveModule( QLive *live, QLiveTrack *track, QLiveClip *clip );
        
        ~QLiveModule() {}
        
        virtual void render( float height ) {}
        
        virtual void update( float *values ) {}
        
        bool updateModule() 
        {
            updateBrightness();
            
            return updateState();
        }
        
        void play() { mIsClipPlaying = true; }
        
        void stop() { mIsClipPlaying = false; }
        
        bool updateState();
        
        bool isPlaying();
        
        QLiveClip*	getClip() { return mClip; }
        
        QLiveTrack*	getTrack() { return mTrack; }
        
        void		updateBrightness();
        
        float		getTrackVolume() { return mTrackVolume; }
        
        std::string getSettings() { return mSettingsStr; }
        
        int			getTrackIndex();
        
        std::string getName();
        
        std::string getTypeString() { return mTypeString; }
        
        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = ci::XmlTree( "module", "" );
            node.setAttribute( "type", getTypeString() );
            node.setAttribute( "name", getName() );
            
            return node;
        }
        
        static void saveSettings( std::vector<QLiveModule*> modules ) {}
        static void loadSettings( std::vector<QLiveModule*> modules ) {}
        
    protected:
        
        QLive               *mLive;
        QLiveTrack          *mTrack;
        QLiveClip           *mClip;
        
        bool                mIsPlaying;			// true when both clip and Live are playing
        bool                mIsClipPlaying;		// true when clip is playing
        
        float               mTrackVolume;
        
        std::string         mSettingsStr;
        std::string         mTypeString;
        
        std::vector<int>    mFreqs;
        float**             mFftBuffer;
    };


}

#endif