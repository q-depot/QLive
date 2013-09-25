/*
 *  QLive.h
 *
 *  Created by Andrea Cuius
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QLIVE
#define QLIVE

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/TextureFont.h"
#include "OscSender.h"
#include "OscListener.h"
#include "cinder/Utilities.h"
#include "QLiveObject.h"
#include "QLiveModule.h"

#define	GET_INFO_MIN_DELAY	2.0f		// minimum delay between info requests


class QLive;
typedef std::shared_ptr<QLive> QLiveRef;


class QLive {
    
public:
    
    static QLiveRef create( std::string osc_host = "localhost", int osc_live_in_port = 9001, int osc_live_out_port = 9000, bool initFromLive = true )
    {
        return QLiveRef( new QLive( osc_host, osc_live_in_port, osc_live_out_port, initFromLive ) );
    }
    
    ~QLive()
    {
        shutdown();
    }

    void sendMessage(std::string address, std::string args = "");
    
    bool getInfo();

    void renderDebug( ci::Vec2i offset = ci::Vec2i( 15, 15 ) );
    
    void play( bool playContinue = false ) 
    {
        if ( playContinue )
            sendMessage("/live/play/continue");
        else 
            sendMessage("/live/play");
    
        mIsPlaying = true;
    }
    
    void stop() 
    {         
        sendMessage("/live/stop");
        mIsPlaying = false;
    }

    void playClip( int trackIdx, int clipIdx );
    
    void stopClip( int trackIdx, int clipIdx );
        
    void setTrackVolume( int trackIdx, float volume ) 
    { 
        QLiveTrackRef track = getTrackByIndex( trackIdx );
        
        if ( !track )
            return;
        
        sendMessage("/live/volume", "i" + ci::toString(trackIdx) + " f" + ci::toString(volume) );
        track->setVolume( volume );
    }
    
    void setParamByIndex( int trackIdx, int deviceIdx, int paramIdx, float value )
    {
        QLiveParamRef param = getParamByIndex( trackIdx, deviceIdx, paramIdx );
        if(!param)
            return;
        param->setValue( value );
        sendMessage("/live/device", "i" + ci::toString(trackIdx) + " i" + ci::toString(deviceIdx) + " i" + ci::toString(paramIdx) + " f" + ci::toString(value) );
    }

    void setParamByName( int trackIdx, int deviceIdx, std::string name, float value )
    {
        QLiveParamRef param = getParamByName( trackIdx, deviceIdx, name );
        if(!param)
            return;
        param->setValue( value );
        sendMessage("/live/device", "i" + ci::toString(trackIdx) + " i" + ci::toString(deviceIdx) + " i" + ci::toString(param->getIndex()) + " f" + ci::toString(value) );
    }

    std::vector<QLiveTrackRef> getTracks() { return mTracks; }

    int getTrackSize() { return mTracks.size(); }
    
    std::vector<QLiveSceneRef> getScenes() { return mScenes; }
    
    QLiveClipRef getClipByIndex( int trackIdx, int clipIdx )
    {   
        QLiveTrackRef track = getTrackByIndex(trackIdx);
        
        if ( track )
            return track->getClipByIndex(clipIdx);
            
        return QLiveClipRef();
    }
    
    QLiveTrackRef getTrackByIndex( int trackIdx )
    { 
        for( size_t k=0; k < mTracks.size(); k++ )
            if ( mTracks[k]->getIndex() == trackIdx )
                return mTracks[k];
        
        return QLiveTrackRef();
    }
    
    QLiveSceneRef getSceneByIndex( int idx )
    {   
        for( size_t k=0; k < mScenes.size(); k++ )
            if ( mScenes[k]->getIndex() == idx )
                return mScenes[k];
        
        return QLiveSceneRef();
    }
    
    QLiveDeviceRef getDeviceByIndex( int trackIdx, int deviceIdx )
    {         
        QLiveTrackRef track = getTrackByIndex(trackIdx);
        
        if ( track )
            return track->getDeviceByIndex(deviceIdx);            
        
        return QLiveDeviceRef();
    }
    
    QLiveParamRef getParamByIndex( int trackIdx, int deviceIdx, int paramIdx )
    {
        QLiveTrackRef track = getTrackByIndex( trackIdx );
        
        if ( track )
        {
            QLiveDeviceRef device = track->getDeviceByIndex( deviceIdx );
            if ( device )
                return device->getParamByIndex(paramIdx);
        }
        
        return QLiveParamRef();
    }

    QLiveParamRef getParamByName( int trackIdx, int deviceIdx, std::string name )
    {
        QLiveTrackRef track = getTrackByIndex( trackIdx );
        
        if ( track )
        {
            QLiveDeviceRef device = track->getDeviceByIndex( deviceIdx );
            if ( device )
                return device->getParamByName(name);
        }
        
        return QLiveParamRef();
    }

    
    std::shared_ptr<float> getParamRefByName( int trackIdx, int deviceIdx, std::string name )
    {
        QLiveTrackRef track = getTrackByIndex( trackIdx );
        
        if ( track )
        {
            QLiveDeviceRef device = track->getDeviceByIndex( deviceIdx );
            if ( device )
            {
                QLiveParamRef param = device->getParamByName(name);
                if ( param )
                    return param->getRef();
            }
        }
        
        return std::shared_ptr<float>();
    }
    
    float getParamValueByName( int trackIdx, int deviceIdx, std::string name )
    { 
        QLiveTrackRef track = getTrackByIndex( trackIdx );
        
        if ( track )
        {   
            QLiveDeviceRef device = track->getDeviceByIndex( deviceIdx );
            if ( device )
            {
                QLiveParamRef param = device->getParamByName(name);
                if ( param )
                    return param->getValue();
            }
        }
        
        return 0;
    }
    
    bool    isPlaying() { return mIsPlaying; }

    bool*   getIsPlayingRef() { return &mIsPlaying; }
    
    bool	isOscListenerConnected()	{ return mOscListener != NULL; }
    
    bool	isOscSenderConnected()		{ return mOscSender != NULL; }
    
    void	initOsc();

    bool    isReady() { return mIsReady; }
    
    ci::XmlTree getSettingsXml();
    
    void    loadSettingsXml( ci::XmlTree, bool forceSettings = false );
    
    bool    isAlive()
    {
        if ( ci::app::getElapsedSeconds() - mPingReceivedAt < 0.5f )
            return true;

        return false;
    }
    
    QLiveSceneRef   getSelectedScene()  { return mSelectedScene; }
    
    QLiveTrackRef   getSelectedTrack()  { return mSelectedTrack; }

    QLiveClipRef    getSelectedClip()   { return mSelectedClip; }
    
    std::string getOscHost()        { return mOscHost; }
    int         getOscInPort()      { return mOscInPort; }
    int         getOscOutPort()     { return mOscOutPort; }

    
private:
    
    QLive( std::string osc_host, int osc_live_in_port, int osc_live_out_port, bool initFromLive );
    
    void	listTrackDevices(int trackIndex) { sendMessage("/live/devicelist", "i" + ci::toString(trackIndex) ); }
    
    void	receiveData();
    
    void	clearObjects();
    
    void	shutdown();
    
    void	closeOsc();
    
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
    
    void    setSelectedScene( QLiveSceneRef scene );
    void    setSelectedTrack( QLiveTrackRef track );
    void    setSelectedClip( QLiveClipRef clip );
    
    void    updateSelectedClip( int sceneIdx, int trackIdx );
    
    std::vector<QLiveTrackRef>  mTracks;
    std::vector<QLiveSceneRef>	mScenes;
    
    QLiveTrackRef				mSelectedTrack;
    QLiveSceneRef				mSelectedScene;
    QLiveClipRef                mSelectedClip;
    
    ci::gl::TextureFontRef		mFontSmall;
    ci::gl::TextureFontRef      mFontMedium;
    
    bool						mIsPlaying;
    bool                        mIsReady;
    
    double                      mPingReceivedAt;
    double						mGetInfoRequestAt;
    
    ci::osc::Sender             *mOscSender;
    ci::osc::Listener			*mOscListener;
    std::string					mOscHost;
    int							mOscInPort;
    int							mOscOutPort;
    std::thread                 mReceiveOscDataThread;
    bool                        mRunOscDataThread;
    
    bool                        mForceInitSettings;
};


#endif
