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
//#include "boost/tuple/tuple.hpp"

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
        
        bool updateModule();

        bool isPlaying() { return mIsPlaying; }
        
        QLiveClip*	getClip() { return mClip; }
        
        QLiveTrack*	getTrack() { return mTrack; }
        
        void		updateBrightness();
        
        float		getTrackVolume() { return mTrackVolume; }
        
        std::string getSettings() { return mSettingsStr; }
        
        int			getTrackIndex();
        
        std::string getName();
        
        std::string getTypeString() { return mTypeString; }
        
        virtual ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node( "module", "" );
            node.setAttribute( "effect",        mTrack->getName() );
            node.setAttribute( "name",          mClip->getName() );
            node.setAttribute( "track",         mTrack->getIndex() );
            node.setAttribute( "clip",          mClip->getIndex() );
            
            // params
            ci::XmlTree pNode( "param", "" );
            std::map< std::string, std::pair<float,float*> >::iterator it;
            for ( it=mParams.begin(); it != mParams.end(); it++ )
            {
                pNode.setAttribute( "name", it->first );
                pNode.setAttribute( "value", it->second.first );
                node.push_back( pNode );
            }
            
            return node;
        }
        
        virtual void loadXmlNode( ci::XmlTree node )
        {       
            std::string name;
            float       value;

            for( ci::XmlTree::Iter it = node.begin("param"); it != node.end(); ++it )
            {
                name    = it->getAttributeValue<std::string>("name");
                value   = it->getAttributeValue<float>("value");    
                
                if ( mParams.count(name) )
                    mParams[name].first = value;

            }    
            
            clipStateUpdateCallback();
        }
        
        void sendLiveParamValue( const std::string &name, float value ); // set Live value to Module local value is it's playing(is selected)

        static void saveSettings( std::vector<QLiveModule*> modules ) {}
        static void loadSettings( std::vector<QLiveModule*> modules ) {}
        
        void clipStateUpdateCallback();
        
    protected:
        
        bool    updateState();
        
    protected:
        
        QLive               *mLive;
        QLiveTrack          *mTrack;
        QLiveClip           *mClip;
        QLiveDevice         *mDevice;

        std::map< std::string, std::pair<float,float*> >    mParams;
        double                                              mParamsUpdatedAt;
        
        bool                mIsPlaying;			// true when both clip and Live are playing
        
        float               mTrackVolume;
        
        std::string         mSettingsStr;
        std::string         mTypeString;
        
        std::vector<int>    mFreqs;
        float**             mFftBuffer;
        
        
    };

}

#endif