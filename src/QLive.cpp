/*
 *  QLive.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#include "cinder/Thread.h"
#include "cinder/Text.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Xml.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "QLive.h"

using namespace ci;
using namespace ci::app;
using namespace std;


bool sortTracksByIndex( QLiveTrackRef a, QLiveTrackRef b ) { return ( a->getIndex() < b->getIndex() ); }


QLive::QLive( string osc_host, int osc_live_in_port, int osc_live_out_port, bool initFromLive ) 
: mOscHost(osc_host), mOscInPort(osc_live_in_port), mOscOutPort(osc_live_out_port)
{
    mOscListener        = NULL;
    mOscSender          = NULL;
    mIsPlaying          = false;
    mIsReady            = false;
    
    mRunOscDataThread   = false;

    initOsc();
    
    // set LiveOsc peer: /remix/set_peer HOST PORT (null host > host is automatically set to the host that sent the request)
    sendMessage("/remix/set_peer", "s i" + ci::toString(mOscInPort) );
    
    mGetInfoRequestAt = - GET_INFO_MIN_DELAY * 2.0f;
    
    if ( initFromLive )
        getInfo();
    
    mFontSmall = gl::TextureFont::create( Font( "Arial", 12 ) );
    mFontMedium = gl::TextureFont::create( Font( "Arial", 14 ) );
    
    console() << "Live > Initialized!" << endl;	
    
    mPingReceivedAt = getElapsedSeconds();
}


void QLive::initOsc()
{
    closeOsc();
    
    try {
        
        if ( mOscSender )					// close OSC sender
        {
            delete mOscSender;
            mOscSender = NULL;
        }
        
        mOscSender = new osc::Sender();
        mOscSender->setup(mOscHost, mOscOutPort);
        
        console() << "LIVE: Initialized OSC sender " << toString(mOscHost) + ":" << toString(mOscOutPort) << endl;
    } 
    catch (...) {
        mOscSender = NULL;
        console() << "LIVE: Failed to bind OSC sender socket " << toString(mOscHost) << ":" << toString(mOscOutPort) << endl;
    }
    
    
    try {
        
        if ( mOscListener )					// close OSC listener
        {
            mOscListener->shutdown();
            delete mOscListener;
            mOscListener = NULL;
            ci::sleep(50);
        }
        
        mOscListener = new osc::Listener();
        mOscListener->setup(mOscInPort);
        console() << "LIVE: Initialized OSC listener " << mOscInPort << endl;
    } 
    catch (...) {
        mOscListener = NULL;
    }
    
    mReceiveOscDataThread = std::thread( &QLive::receiveData, this );
}


void QLive::shutdown() 
{
    mIsReady = false;

    closeOsc();
    
    clearObjects();                     // delete objects and clear pointers
}


void QLive::closeOsc()
{
    mRunOscDataThread = false;
    
    if ( mReceiveOscDataThread.joinable() )
        mReceiveOscDataThread.join();

    if ( mOscListener )
    {
        mOscListener->shutdown();
        delete mOscListener;
        mOscListener = NULL;
    }
    
    if ( mOscSender )					// close OSC sender
    {
        delete mOscSender;
        mOscSender = NULL;
    }
}


void QLive::clearObjects()
{
    mTracks.clear();
    mScenes.clear();
}


bool QLive::getInfo() 
{ 
    if ( getElapsedSeconds() - mGetInfoRequestAt < GET_INFO_MIN_DELAY )
        return false;
    
    mIsReady = false;
    
    clearObjects();
    
    mSelectedTrack = NULL;
    
    mGetInfoRequestAt = getElapsedSeconds();

    sendMessage("/live/name/scene");

    ci::sleep(50);
    sendMessage("/live/name/track");

    ci::sleep(200);
    sendMessage("/live/name/clip");    

    ci::sleep(200);
    sendMessage("/live/devicelist");

    ci::sleep(500);
    
    // Select first available clip
    vector<QLiveClipRef> clips;
    for( auto k=0; k < mTracks.size(); k ++ )
    {
        clips = mTracks[k]->getClips();
        if ( clips.empty() )
            continue;
    
        updateSelectedClip( clips[0]->getIndex(), mTracks[k]->getIndex() );
        sendMessage("/live/scene", "i" + to_string( clips[0]->getIndex() ) );
        sendMessage("/live/track/view", "i" + to_string( mTracks[k]->getIndex() ) );
        break;
    }

    mIsReady = true;
    
    return mIsReady;
}


void QLive::renderDebug( Vec2i offset )
{
    vector<QLiveClipRef>    clips;
    vector<QLiveDeviceRef>  devices;
    Vec2f                   initPos;
    string                  clipsStr, devicesStr;
    Vec2f                   pos = offset;
    
    gl::color( ColorA( 0.0f, 0.0f, 0.0f, 0.45f ) );
    gl::drawSolidRect( getWindowBounds() );
    
    gl::color( Color::white() );
    
    mFontMedium->drawString( "QLIVE DEBUGGER", pos );    pos.y += 15;
    mFontSmall->drawString( mOscHost + " / in " + toString(mOscInPort) + " / out " + toString(mOscOutPort), pos );    pos.y += 25;
    
    initPos = pos;
    
    // Scenes
    mFontMedium->drawString( "Scenes (" + to_string( mScenes.size() ) + ")", pos );    pos.y += 25;
    for (int k = 0; k < mScenes.size(); k++)
    {
        mFontSmall->drawString( toString(mScenes[k]->getIndex()) + "\t" + mScenes[k]->getName(), pos );
        pos.y += 15;
    }
    pos.x += 260;
    pos.y = initPos.y;
    
    // Tracks
    mFontMedium->drawString( "Tracks (" + to_string( mTracks.size() ) + ")", pos );    pos.y += 25;
    for (int k = 0; k < mTracks.size(); k++)
    {
        mFontMedium->drawString( toString(mTracks[k]->getIndex()) + "\t" + mTracks[k]->getName(), pos );
        pos.y += 18;
        
        clips   = mTracks[k]->getClips();
        devices = mTracks[k]->getDevices();
        
        clipsStr = "\tClips ------- ";
        for( int i = 0; i < clips.size(); i++ )
            clipsStr += to_string( clips[i]->getIndex() ) + " [ " + clips[i]->getName() + " ]     ";
        mFontSmall->drawString( clipsStr, pos );
        pos.y += 18;
        
        
        devicesStr = "\tDevices --- ";
        for( int i = 0; i < devices.size(); i++ )
            devicesStr += to_string( devices[i]->getIndex() ) + " [ " + devices[i]->getName() + " ]     ";
        mFontSmall->drawString( devicesStr, pos );
        pos.y += 25;
    }
}


void QLive::sendMessage(string address, string args)
{	
    osc::Message message;
    message.setAddress( address );
    message.setRemoteEndpoint(mOscHost, mOscOutPort);
    vector<string> splitValues;
    boost::split(splitValues, args, boost::is_any_of(" "));
    
    for (int k = 0; k < splitValues.size(); k++) {
        char argType = splitValues[k][0];
        splitValues[k].erase(0,1);
        
        if ( argType == 'i' )
            message.addIntArg( boost::lexical_cast<int>( splitValues[k] ) );
        else
            if ( argType == 'f' )
            message.addFloatArg( boost::lexical_cast<float>( splitValues[k] ) );
        else 
            if ( argType == 's' )
            message.addStringArg( splitValues[k] );
    }
    
    mOscSender->sendMessage(message);
}


void QLive::parseScene( osc::Message message )
{
    int     index       = message.getArgAsInt32(0);
    string  name        = message.getArgAsString(1);
    
    QLiveSceneRef scene = getSceneByIndex( index );
    
    if ( scene )
    {
        scene->setName( name );
        return;
    }
    
    mScenes.push_back( QLiveScene::create( index, name ) );
}


void QLive::parseTrack( osc::Message message ) 
{          
    int     index	= message.getArgAsInt32(0);
    string  name    = message.getArgAsString(1);
    ColorA  color   = colorIntToColorA( message.getArgAsInt32(2) );
    
    QLiveTrackRef track = getTrackByIndex( index );
    
    if ( track )
        track->setName( name );
    else
        mTracks.push_back( QLiveTrack::create( index, name, color ) );

    sort( mTracks.begin(), mTracks.end(), sortTracksByIndex );

    sendMessage("/live/volume", "i" + toString(index) );	// get track volume

    listTrackDevices(index);
}


void QLive::parseClip( osc::Message message ) 
{
    int     trackIdx	= message.getArgAsInt32(0);
    int 	clipIdx		= message.getArgAsInt32(1);
    string 	name		= message.getArgAsString(2);
    ColorA  color       = colorIntToColorA( message.getArgAsInt32(3) );

    QLiveTrackRef   track = getTrackByIndex( trackIdx );
    QLiveClipRef    clip;
    
    if ( track )
    {
        clip = track->getClipByIndex(clipIdx);
        if ( clip )
        {
            clip->setName( name );
            clip->setColor( color );
            return;
        }
    }

    clip = QLiveClip::create( clipIdx, name, color );
    
    mTracks[trackIdx]->addClip( clip );
    
    sendMessage("/live/clip/info", "i" + toString(trackIdx) + " i" + toString(clipIdx) );       // get clip info
}


void QLive::parseClipInfo( osc::Message message ) 
{    
    int             trackIdx	= message.getArgAsInt32(0);
    int             clipIdx		= message.getArgAsInt32(1);
    ClipState       state       = (ClipState)message.getArgAsInt32(2);
    QLiveTrackRef   track       = getTrackByIndex( trackIdx );
    QLiveClipRef    clip;
    
    if ( !track )
        return;

    clip = track->getClipByIndex(clipIdx);

    if ( !clip )
        return;

    // Select clip
    if ( state == CLIP_TRIGGERED )
        updateSelectedClip( clip->getIndex(), track->getIndex() );
    
    // Set clip state
    clip->setState(state);
}


void QLive::parseDeviceList( osc::Message message )
{
    if ( message.getNumArgs() < 3 )	// seems there is an error in the APIs!
       return;
    
    int             deviceIdx;
    string          deviceName;

    QLiveTrackRef   track = getTrackByIndex( message.getArgAsInt32(0) );
    
    if ( !track )
        return;
    
    for( int k=1; k < message.getNumArgs(); k+=2 )
    {
        deviceIdx	= message.getArgAsInt32(k);
        deviceName  = message.getArgAsString(k+1);
            
        if ( !track->getDeviceByIndex(deviceIdx) )
        {
            track->addDevice( QLiveDevice::create( deviceIdx, deviceName ) );
            
            // get device params	
            sendMessage("/live/device", "i" + toString(track->getIndex()) + " i" + toString(deviceIdx) );
        }
    }
}


void QLive::parseDeviceAllParams( osc::Message message )
{
    //	int trackIdx, int deviceIdx, int parameterIdx, int value,  str name, ...
    if ( message.getNumArgs() < 7 )
        return;
    
    int             trackIdx    = message.getArgAsInt32(0);
    int             deviceIdx   = message.getArgAsInt32(1);
    QLiveTrackRef	track       = getTrackByIndex(trackIdx);
    QLiveDeviceRef  device      = track->getDeviceByIndex( deviceIdx );
    
    if ( !device )
        return;
    
    int         paramIdx;
    float       paramVal;
    float       paramMinVal;
    float       paramMaxVal;
    std::string paramName;
    
    for (int k=2; k < message.getNumArgs(); k+=5) 
    {
        paramIdx    = message.getArgAsInt32(k);
        paramVal    = message.getArgAsFloat(k+1);
        paramName   = message.getArgAsString(k+2);
        paramMinVal = message.getArgAsFloat(k+3);
        paramMaxVal = message.getArgAsFloat(k+4);

        device->addParam( paramIdx, paramVal, paramName, paramMinVal, paramMaxVal );
    }
}


void QLive::parseDeviceParam( osc::Message message )
{
    //	/live/device/param (int track, int device, int paarmeter, int value, str name)
    
    if ( message.getNumArgs() < 7 )
        return;
    
    int             trackIdx    = message.getArgAsInt32(0);
    int             deviceIdx   = message.getArgAsInt32(1);
    int             paramIdx    = message.getArgAsInt32(2);
    float           paramValue  = ( message.getArgType(3) == osc::TYPE_INT32 ) ? message.getArgAsInt32(3) : message.getArgAsFloat(3);
    string          paramName   = message.getArgAsString(4);
    float           paramMin    = message.getArgAsFloat(5);
    float           paramMax    = message.getArgAsFloat(6);
    
    QLiveParamRef   param       = getParamByIndex( trackIdx, deviceIdx, paramIdx );
    
    if ( param )    // we assume the param exists, parseDeviceAllParams() creates one for each device param in Live
    {
        param->setRange( paramMin, paramMax );
        param->setValue( paramValue );
    }
}


void QLive::receiveData()
{    
    mRunOscDataThread = true;

    int selectedSceneIdx, selectedTrackIdx;
    
    while( mRunOscDataThread && mOscListener ) {
        
        selectedSceneIdx = -1;
        selectedTrackIdx = -1;
        
        while ( mOscListener->hasWaitingMessages() )
        {
            osc::Message message;
            mOscListener->getNextMessage(&message);
            
            string	msgAddress = message.getAddress();

            // debug
//            if ( true && msgAddress != "/live/ping" && msgAddress != "/live/beat" )
//                debugOscMessage( message );

            // Parse Live objects
            if ( msgAddress == "/live/name/scene" )
                parseScene(message);
            
            else if ( msgAddress == "/live/name/track" )
                parseTrack(message);

            else if ( msgAddress == "/live/scene" ) 
                selectedSceneIdx = message.getArgAsInt32(0) - 1;                // -1 LiveOSC inconsistency
            
            else if ( msgAddress == "/live/track" )
                selectedTrackIdx = message.getArgAsInt32(0) - 1;                // -1 LiveOSC inconsistency
            
            else if ( msgAddress == "/live/name/clip" )
                parseClip(message);
    
            else if ( msgAddress == "/live/clip/info" )
                parseClipInfo(message);
            
            else if ( msgAddress == "/live/devicelist" )
                parseDeviceList(message);
            
            else if ( msgAddress == "/live/device/allparam" )
                parseDeviceAllParams(message);
        
            else if ( msgAddress == "/live/play" )
                mIsPlaying = ( (ClipState)message.getArgAsInt32(0) == 2 ) ? true : false;

            else if ( msgAddress == "/live/volume" )
            {
                int trackIndex = message.getArgAsInt32(0);
                if ( trackIndex < mTracks.size() )
                    mTracks[trackIndex]->setVolume( message.getArgAsFloat(1) );
            }
            
            else if ( msgAddress == "/live/device/param" )
                parseDeviceParam(message);
            
            else if ( msgAddress == "/live/ping" )
                mPingReceivedAt = getElapsedSeconds();
        }
        
        updateSelectedClip( selectedSceneIdx, selectedTrackIdx );
        
        sleep(15.0f);
//		boost::this_thread::sleep(boost::posix_time::milliseconds(15));
    }
    console() << "Live > receiveData() thread exited!" << endl;
}


void QLive::debugOscMessage( osc::Message message )
{
    console() << "LIVE: Address: " << message.getAddress() << " ";
    
    for (int i = 0; i < message.getNumArgs(); i++) {
        if (message.getArgType(i) == osc::TYPE_INT32){
            try {
                console() << message.getArgAsInt32(i) << " ";
            }
            catch (...) {
                console() << "Exception reading argument as int32" << std::endl;
            }
            
        }else if (message.getArgType(i) == osc::TYPE_FLOAT){
            try {
                console() << message.getArgAsFloat(i) << " ";
            }
            catch (...) {
                console() << "Exception reading argument as float" << std::endl;
            }
        }else if (message.getArgType(i) == osc::TYPE_STRING){
            try {
                console() << message.getArgAsString(i).c_str() << " ";
            }
            catch (...) {
                console() << "Exception reading argument as string" << std::endl;
            }
            
        }
    }
    console() << endl;
}


XmlTree QLive::getSettingsXml()
{
    XmlTree liveSettings("QLiveSettings", "" );
    
    liveSettings.setAttribute( "force",         mForceInitSettings );
    liveSettings.setAttribute( "oscSenderHost", mOscHost );
    liveSettings.setAttribute( "oscOutPort",    mOscOutPort );
    liveSettings.setAttribute( "oscInPort",     mOscInPort );
    
    XmlTree scenes("scenes", "" );
    XmlTree tracks("tracks", "" );
    
    for( size_t k=0; k < mScenes.size(); k++ )
        scenes.push_back( mScenes[k]->getXmlNode() );
    
    for( size_t k=0; k < mTracks.size(); k++ )
        tracks.push_back( mTracks[k]->getXmlNode() );
    
    liveSettings.push_back( scenes );
    liveSettings.push_back( tracks );
    
    return liveSettings;
}


void QLive::loadSettingsXml( XmlTree liveSettings, bool forceSettings )
{
    mForceInitSettings  = forceSettings;
        
    string  oscHost     = liveSettings.getAttributeValue<string>( "oscSenderHost" );
    int     oscOutPort  = liveSettings.getAttributeValue<int>( "oscOutPort" );
    int     oscInPort   = liveSettings.getAttributeValue<int>( "oscInPort" );

    if ( mOscHost != oscHost || mOscInPort != oscInPort || mOscOutPort != oscOutPort )
    {
        mOscHost        = oscHost;
        mOscInPort  = oscInPort;
        mOscOutPort = oscOutPort;
        
        initOsc();
    }
    
    if ( mForceInitSettings )
        clearObjects();
    
    QLiveSceneRef   scene;
    QLiveTrackRef   track;
    int             index;
    string          name;

    // parse scenes
    for( XmlTree::Iter nodeIt = liveSettings.begin("scenes/scene"); nodeIt != liveSettings.end(); ++nodeIt )
    {
        index   = nodeIt->getAttributeValue<int>("index");
        name    = nodeIt->getAttributeValue<string>("name");
        scene   = getSceneByIndex(index);
        
        if ( scene )
            scene->loadXmlNode( *nodeIt );
        
        else if ( !scene && mForceInitSettings )
        {
            scene = QLiveScene::create( index, name );
            scene->loadXmlNode( *nodeIt );
            mScenes.push_back( scene );
        }
    }
    
    // parase tracks
    for( XmlTree::Iter nodeIt = liveSettings.begin("tracks/track"); nodeIt != liveSettings.end(); ++nodeIt )
    {
        index   = nodeIt->getAttributeValue<int>("index");
        name    = nodeIt->getAttributeValue<string>("name");
        track   = getTrackByIndex(index);

        if ( track )
            track->loadXmlNode( *nodeIt, mForceInitSettings );
        
        else if ( !track && mForceInitSettings )
        {
            track = QLiveTrack::create( index, name );
            track->loadXmlNode( *nodeIt, mForceInitSettings );
            mTracks.push_back( track );
        }
    }
    
   // syncLiveClips();        // send clip state to Live
    
}


void QLive::updateSelectedClip( int sceneIdx, int trackIdx )
{
    // nothing changed
    if ( sceneIdx < 0 && trackIdx < 0 )
        return;
    
    QLiveSceneRef   scene;
    QLiveTrackRef   track;
    QLiveClipRef    clip;
    
    // track changed
    if ( sceneIdx < 0 )
    {
        
        QLiveTrackRef track = getTrackByIndex( trackIdx );
        
        setSelectedTrack( track );
        
        if ( !mSelectedTrack )
            return;
    
        clip = mSelectedTrack->getPlayingClip();                        // find playing clip

        if ( !clip && mSelectedTrack->getClips().size() > 0 )           // otherwise find the first clip
            clip = mSelectedTrack->getClips()[0];
        
        setSelectedClip( clip );
    }
    
    // scene changed
    else if ( trackIdx < 0 && mSelectedTrack )
    {
        clip    = getClipByIndex( mSelectedTrack->getIndex(), mSelectedScene->getIndex() );
        scene   = getSceneByIndex( sceneIdx );

        setSelectedScene( scene );
        setSelectedClip( clip );
    }
    
    // track + scene changed
    else
    {
        track   = getTrackByIndex( trackIdx );
        scene   = getSceneByIndex( sceneIdx );
        clip    = getClipByIndex( trackIdx, sceneIdx );
        
        setSelectedScene( scene );
        setSelectedTrack( track );
        setSelectedClip( clip );
    }
}


void QLive::setSelectedScene( QLiveSceneRef scene )
{
    if ( scene == mSelectedScene )  return;
    if ( mSelectedScene )           mSelectedScene->select(false);
    mSelectedScene                  = scene;
    if ( mSelectedScene )           mSelectedScene->select(true);
}


void QLive::setSelectedTrack( QLiveTrackRef track )
{
    if ( track == mSelectedTrack )  return;
    if ( mSelectedTrack )           mSelectedTrack->select(false);
    mSelectedTrack                  = track;
    if ( mSelectedTrack )           mSelectedTrack->select(true);
}


void QLive::setSelectedClip( QLiveClipRef clip )
{
    if ( clip == mSelectedClip )    return;
    if ( mSelectedClip )            mSelectedClip->select(false);
    mSelectedClip                   = clip;
    if ( mSelectedClip )            mSelectedClip->select(true);
}


void QLive::playClip( int trackIdx, int clipIdx )
{
    QLiveTrackRef track = getTrackByIndex( trackIdx );
    
    if ( !track )
        return;
    
    QLiveClipRef clip = track->getClipByIndex(clipIdx);
    
    if ( !clip )
        return;
    
    sendMessage("/live/play/clip", "i" + ci::toString(trackIdx) + " i" + ci::toString(clipIdx) );
    
    std::vector<QLiveClipRef> clips = track->getClips();
    for( size_t k=0; k < clips.size(); k++ )
        if ( clips[k] == clip )
            clip->setState( CLIP_PLAYING );
        else
            clips[k]->setState( HAS_CLIP );

    updateSelectedClip( clipIdx, trackIdx );
}


void QLive::stopClip( int trackIdx, int clipIdx )
{
    QLiveTrackRef track = getTrackByIndex( trackIdx );
    
    if ( !track )
        return;
    
    QLiveClipRef clip = track->getClipByIndex(clipIdx);
    
    if ( !clip )
        return;
    
    sendMessage("/live/stop/clip", "i" + ci::toString(trackIdx) + " i" + ci::toString(clipIdx) );
    clip->setState( HAS_CLIP );

    clip->select( false );
    
    if ( clip == mSelectedClip )
        mSelectedClip.reset();
}

