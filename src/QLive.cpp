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

#define QLIVE_SETTINGS     "/Users/Q/QLiveSettings.xml"

namespace nocte {
        
    QLive::QLive(string osc_host, int osc_live_in_port, int osc_live_out_port, int osc_analyzer_in_port, int live_params_in_port, bool init) 
    : mOscHost(osc_host), mOscLiveInPort(osc_live_in_port), mOscLiveOutPort(osc_live_out_port), mOscAnalyzerInPort(osc_analyzer_in_port), mOscLiveParamsInPort(live_params_in_port),
      mOscListener(NULL), mOscSender(NULL), mSelectedTrack(NULL), mIsPlaying(false), mIsReady(false)
    {
        initOsc();
        
        mGetInfoRequestAt = - GET_INFO_MIN_DELAY * 2.0f;
        
        if ( init )
            getInfo();
        
        mColor1		= ColorA( 0.95f, 0.25f, 0.5f, 1.0f );
        mColor2		= ColorA( 0.39f, 0.89f, 0.92f, 1.0f );
        mColor3		= ColorA( 0.90, 0.88f, 0.56f, 1.0f );
        mColor4		= ColorA( 0.57, 0.88f, 0.36f, 1.0f );
        
        mFontSmall	= Font("Helvetica", 12);
        mFontMedium	= Font("Helvetica", 14);
        
        console() << "Live > Initialized!" << endl;	
        
        mAnalyzer   = new QLiveAnalyzer( mOscAnalyzerInPort );
        
    }


    void QLive::initOsc()
    {
        if ( mOscListener )					// close OSC listener
        {
            mOscListener->shutdown();		
            delete mOscListener;
            mOscListener = NULL;
            ci::sleep(50);
        }
        
        if ( mOscSender )					// close OSC sender
        {
            delete mOscSender;
            mOscSender = NULL;
        }
        
        try {
            mOscSender = new osc::Sender();
            mOscSender->setup(mOscHost, mOscLiveOutPort);
            
            console() << "LIVE: Initialized OSC sender " << toString(mOscHost) + ":" << toString(mOscLiveOutPort) << endl;	
    //        Logger::log("LIVE: Initialized OSC sender " + toString(mOscHost) + ":" + toString(mOscOutPort) );
        } catch (...) {
            mOscSender = NULL;
            console() << "LIVE: Failed to bind OSC sender socket " << toString(mOscHost) << ":" << toString(mOscLiveOutPort) << endl;
    //		Logger::log("LIVE: Failed to bind OSC sender socket " + toString(mOscHost) + ":" + toString(mOscOutPort) );
        }
        
        try {
            mOscListener = new osc::Listener();
            mOscListener->setup(mOscLiveInPort);
            console() << "LIVE: Initialized OSC listener " << mOscLiveInPort << endl; 
    //		Logger::log("LIVE: Initialized OSC listener " + toString(mOscInPort) );
        } catch (...) {
            mOscListener = NULL;
    //		Logger::log("LIVE: Failed to bind OSC listener socket " + toString(mOscInPort) );
        }
        
        thread receiveDataThread( &QLive::receiveData, this);
    }


    void QLive::shutdown() 
    {
        // TODO proper clean up!!!
        
        mIsReady = false;
        
        if ( mOscListener )					// close OSC listener
        {
            mOscListener->shutdown();		
            delete mOscListener;
            mOscListener = NULL;
            ci::sleep(50);
        }

        if ( mOscSender )					// close OSC sender
        {
            delete mOscSender;
            mOscSender = NULL;
        }
        
        deleteObjects();					// delete objects and clear pointers
        
        if ( mAnalyzer )
        {
            mAnalyzer->shutdown();
            delete mAnalyzer;
            mAnalyzer = NULL;
        }
        
    }


    bool QLive::getInfo() 
    { 
        mIsReady = false;
        
        if ( getElapsedSeconds() - mGetInfoRequestAt < GET_INFO_MIN_DELAY )
            return false;
        
        mSelectedTrack = NULL;
        
        mGetInfoRequestAt = getElapsedSeconds();

        deleteObjects();
        
        sendMessage("/live/name/scene");

        ci::sleep(50);
        sendMessage("/live/name/track"); 
//
        ci::sleep(200);
        sendMessage("/live/name/clip");    
//        
        ci::sleep(200);
        sendMessage("/live/devicelist");
//        
        ci::sleep(500);
        
        mIsReady = true;
        
        return mIsReady;
    }


    void QLive::deleteObjects()
    {        
        for(int k=0; k < mTracks.size(); k++)
            delete mTracks[k];
        mTracks.clear();
        
        for(int k=0; k < mScenes.size(); k++)
            delete mScenes[k];
        mScenes.clear();
    }


    void QLive::renderDebug()
    {
        QLiveTrack  *track;
        QLiveClip   *clip;
        QLiveDevice *device;
        
        gl::color( Color::white() );
        
        TextLayout textLayout = TextLayout();
        textLayout.clear( ColorA(0.0f, 0.0f, 0.0f, 0.0f) );
        textLayout.setLeadingOffset(3);
        textLayout.setColor( Color::white() );
        
        textLayout.setFont( mFontMedium );

        textLayout.addLine( "QLIVE" );
        textLayout.addLine( mOscHost + " / in " + toString(mOscLiveInPort) + " / out " + toString(mOscLiveOutPort) + " / fft " + toString(mOscAnalyzerInPort) );
        textLayout.addLine( " " );
        
        textLayout.setFont( mFontSmall );
        
        textLayout.addLine( "SCENES" );
        textLayout.addLine( "index\tname" );
        for (int k = 0; k < mScenes.size(); k++)
            textLayout.addLine( toString(mScenes[k]->mIndex) + "\t\t" + mScenes[k]->mName );
        textLayout.addLine( " " );	

        textLayout.addLine( "TRACKS" );
        for (int k = 0; k < mTracks.size(); k++)
        {
            track = mTracks[k];
            
            textLayout.addLine( toString(track->mIndex) + "\t" + track->mName + "\t" + toString(track->mClips.size()) );
            
            textLayout.addLine( "\tDEVICES" );
            for( int i = 0; i < track->mDevices.size(); i++ )
            {
                device = track->mDevices[i];
                textLayout.addLine( "\t" + toString(device->mIndex) + "\t" + device->mName );
            }
            
            textLayout.addLine( "\tCLIPS" );
            for( int i = 0; i < track->mClips.size(); i++ )
            {
                clip = track->mClips[i];                
                textLayout.addLine( "\t" + toString(clip->mIndex) + "\t" + clip->mName );
            }
        }
        
        textLayout.addLine( " " );
        
        gl::Texture tex = gl::Texture(textLayout.render(true));
        gl::draw( tex, Vec2f( getWindowWidth() - 450, 15) );

    }


    void QLive::sendMessage(string address, string args)
    {	
        
    //	console() << "send message" << endl;
        osc::Message message;
        message.setAddress( address );
        message.setRemoteEndpoint(mOscHost, mOscLiveOutPort);
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


    void QLive::parseScene( osc::Message message ) {
        
        int index	= message.getArgAsInt32(0);
        string name = message.getArgAsString(1);
        
        // update if the scene already exists
        for(int k=0; k < mScenes.size(); k++)
            if ( mScenes[k]->mIndex == index )
            {
                mScenes[k]->mName = name;
                return;
            }
        
        mScenes.push_back( new QLiveScene( index, name ) );
        
//        console() << "parse scene: " << endl;
    }


    bool sortTracksByIndex( QLiveTrack *a, QLiveTrack *b ) { return ( a->getIndex() < b->getIndex() ); };


    void QLive::parseTrack( osc::Message message ) 
    {          
        int     index	= message.getArgAsInt32(0);
        string  name    = message.getArgAsString(1);
        ColorA  color   = colorIntToColorA( message.getArgAsInt32(2) );

        bool has_track = false;
        
        for(int k=0; k < mTracks.size(); k++)
            if( mTracks[k]->mIndex == index )
            {
                mTracks[k]->mName = name;
                has_track = true;
                break;
            }
        
        if ( !has_track )
            mTracks.push_back( new QLiveTrack( index, name, color ) );

        sort( mTracks.begin(), mTracks.end(), sortTracksByIndex );

        sendMessage("/live/volume", "i" + toString(index) );	// get track volume

        listTrackDevices(index);
        
        // parse devices
        // parse clips
    }


    void QLive::parseClip( osc::Message message ) 
    {
        console() << "parse clip" << endl;
        int     trackIdx	= message.getArgAsInt32(0);
        int 	clipIdx		= message.getArgAsInt32(1);
        string 	name		= message.getArgAsString(2);
        ColorA  color       = colorIntToColorA( message.getArgAsInt32(3) );

        QLiveTrack  *track = getTrack( trackIdx );
        
        if ( track )                                            // return if the clip already exists
        {
            if ( track->getClip(clipIdx) )
                return;
        }
    
        QLiveClip *clip = new QLiveClip( clipIdx, name, trackIdx, color );
        
        mTracks[trackIdx]->mClips.push_back( clip );
        
        sendMessage("/live/clip/info", "i" + toString(trackIdx) + " i" + toString(clipIdx) );       // get clip info
    }


    void QLive::parseClipInfo( osc::Message message ) 
    {    
        int trackIdx	= message.getArgAsInt32(0);
        int clipIdx		= message.getArgAsInt32(1);
        ClipState state	= (ClipState)message.getArgAsInt32(2);
        
        QLiveClip*  clip;
        
        
        QLiveTrack  *track = getTrack( trackIdx );
        
        if ( track )
        {
            // stop all clips
            for( size_t k=0; k < track->mClips.size(); k++ )
                track->mClips[k]->setState( HAS_CLIP );
            
            clip = track->getClip(clipIdx);
            
            // play selected clip
            if ( clip )
                clip->setState(state);
        }
    }


    void QLive::parseDeviceList( osc::Message message ) 
    {
        if ( message.getNumArgs() < 3 )	// seems there is an error in the APIs!
           return;
        
        int			deviceIdx;
        string		deviceName;

        QLiveTrack  *track = getTrack( message.getArgAsInt32(0) );
        
        if ( track )
        {
            for( int k=1; k < message.getNumArgs(); k+=2 )
            {
                deviceIdx	= message.getArgAsInt32(k);
                deviceName  = message.getArgAsString(k+1);
                    
                if ( !track->getDevice(deviceIdx) )
                {
                    track->mDevices.push_back( new QLiveDevice( deviceIdx, deviceName, track->mIndex ) );
                    
                    // get device params	
                    sendMessage("/live/device", "i" + toString(track->mIndex) + " i" + toString(deviceIdx) );
                }
            }
        }
    }


    void QLive::parseDeviceAllParams( osc::Message message )
    {
        //	int trackIdx, int deviceIdx, int parameterIdx, int value,  str name, ...
        if ( message.getNumArgs() < 7 )
            return;
        
        int			trackIdx    = message.getArgAsInt32(0);
        int			deviceIdx   = message.getArgAsInt32(1);
        QLiveTrack	*track      = getTrack(trackIdx);
        QLiveDevice	*device     = track->getDevice( deviceIdx );
        
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
        
        if ( message.getNumArgs() < 5 )
            return;
        
        int			trackIdx    = message.getArgAsInt32(0);
        int			deviceIdx   = message.getArgAsInt32(1);
        int			paramIdx    = message.getArgAsInt32(2);
        
        QLiveTrack	*track      = getTrack(trackIdx);
        QLiveDevice	*device     = track->getDevice( deviceIdx );
        QLiveParam	*param      = device->mParams[paramIdx];
        
        param->mValue   = ( message.getArgType(3) == osc::TYPE_INT32 ) ? message.getArgAsInt32(3) : message.getArgAsFloat(3);
        param->mName    = message.getArgAsString(4);
    }
    

    void QLive::receiveData(){

        while( mOscListener ) {

            while (mOscListener->hasWaitingMessages()) {
                osc::Message message;
                mOscListener->getNextMessage(&message);
                
                string	msgAddress = message.getAddress();

                // debug
                if (false)
                    debugOscMessage( message );
                
            //	console() << "receive DATA! " << msgAddress << endl;
                
                // Parse Live objects
                if ( msgAddress == "/live/name/scene" )
                    parseScene(message);
                
                else if ( msgAddress == "/live/name/track" )
                    parseTrack(message);
                
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
                        mTracks[trackIndex]->mVolume = message.getArgAsFloat(1);
                }

                else if ( msgAddress == "/live/track" )
                {
                    int trackIndex = message.getArgAsInt32(0) - 1;  // shift
                    for(int k=0; k < mTracks.size(); k++)
                        if ( mTracks[k]->mIndex == trackIndex )
                        {
                            mSelectedTrack = mTracks[k];
                            break;
                        }
                }
                
                else if ( msgAddress == "/live/device/param" )
                    parseDeviceParam(message);
                
            }
                
            sleep(15.0f);
    //		boost::this_thread::sleep(boost::posix_time::milliseconds(15));
            
        }
    //	Logger::log("Live > receiveData() thread exited!");
    }

    
    void QLive::renderAnalyzer()
    {
        mAnalyzer->render();
    }
    
    
 	float QLive::getFreqAmplitude(int freq, int channel) 
    { 
        return mAnalyzer->getFreqAmplitude(freq, channel); 
    };
    
    
    float* QLive::getFftBuffer(int channel) 
    { 
        return mAnalyzer->mFft[channel];
    };
    
    
    float* QLive::getAmplitudeRef(int channel)
    { 
        return &mAnalyzer->mAmplitude[channel];
    };
    
    void QLive::saveSettings( vector<QLiveModuleWithFixtures*> modules )
    {
        XmlTree                 xmlDoc("settings", "");
        XmlTree                 moduleSettings("modules", "");
        XmlTree                 paramSettings("params", "");
        XmlTree                 node;
        QLiveModuleWithFixtures *module;
        AxisAlignedBox3f        bBox;
        Vec3f                   bBoxMinVec, bBoxMaxVec;
        QLiveTrack              *track;
        QLiveDevice             *device;
        QLiveParam              *param;
        
        // Modules
        for( size_t k=0; k < modules.size(); k++ )
        {
            module      = modules[k];
            bBox        = module->getBoundingBox();
            bBoxMinVec  = bBox.getMin();
            bBoxMaxVec  = bBox.getMax();
            
            node = XmlTree( "module", "" );
            node.setAttribute( "type", module->getTypeString() );
            node.setAttribute( "name", module->getName() );
            node.setAttribute( "bBox_min_x", bBoxMinVec.x ); node.setAttribute( "bBox_min_y", bBoxMinVec.y ); node.setAttribute( "bBox_min_z", bBoxMinVec.z );
            node.setAttribute( "bBox_max_x", bBoxMaxVec.x ); node.setAttribute( "bBox_max_y", bBoxMaxVec.y ); node.setAttribute( "bBox_max_z", bBoxMaxVec.z );
            
            moduleSettings.push_back(node);
        }
        
        // Device settings
        for( size_t k=0; k < mTracks.size(); k++ )
        {
            track   = mTracks[k];
            
            for( size_t i=0; i < track->mDevices.size(); i++ )
            {
                device = track->mDevices[i];
                
                for(int j=0; j < device->getParamsN(); j++ )
                {
                    param = device->getParam(j);
                    
                    node = XmlTree( "param", "" );
                    node.setAttribute( "effect",   track->getName() );      // only used for XML readability
                    node.setAttribute( "name",     param->getName() );
                    node.setAttribute( "track",    track->getIndex() );
                    node.setAttribute( "device",   device->getIndex() );
                    node.setAttribute( "index",    param->getIndex() );
                    node.setAttribute( "value",    param->getValue() );
                    
                    paramSettings.push_back( node );
                }
            }
        }
        
        xmlDoc.push_back( moduleSettings );
        xmlDoc.push_back( paramSettings );
        
        //	moduleSettings.write( writeFile( loadResource(RES_MODULE_SETTINGS)->getFilePath() ) );
        xmlDoc.write( writeFile(QLIVE_SETTINGS) );
    }
    
    void QLive::loadSettings( vector<QLiveModuleWithFixtures*> modules )
    {
        string  moduleName;
        string  moduleType;
        string  paramName;
        float   paramValue;
        int     paramTrackIdx, paramDeviceIdx, paramIdx;
        
        XmlTree settingsXml;
        Vec3f   bBoxMinVec, bBoxMaxVec;
        
        QLiveModuleWithFixtures *module;
        
        try 
        {
            //    XmlTree settingsXml( loadResource(RES_MODULE_SETTINGS) );
            settingsXml = XmlTree( loadFile(QLIVE_SETTINGS) );
        }
        catch ( ... )
        {
            console() << "loadModuleSettings:  settings file " << QLIVE_SETTINGS << " not found!" << endl;
            return;
        }
        
        // parse Modules
        for(XmlTree::Iter moduleIt = settingsXml.begin("settings/modules/module"); moduleIt != settingsXml.end(); ++moduleIt)
        {
            try 
            {
                moduleName      = moduleIt->getAttributeValue<string>("name");
                moduleType      = moduleIt->getAttributeValue<string>("type");
                
                for( size_t k=0; k < modules.size(); k++ )
                {
                    module = modules[k];
                    
                    if ( module->getName() == moduleName && module->getTypeString() == moduleType )
                    {
                        // Bounding Box
                        bBoxMinVec.x    = moduleIt->getAttributeValue<float>("bBox_min_x");
                        bBoxMinVec.y    = moduleIt->getAttributeValue<float>("bBox_min_y");
                        bBoxMinVec.z    = moduleIt->getAttributeValue<float>("bBox_min_z");
                        
                        bBoxMaxVec.x    = moduleIt->getAttributeValue<float>("bBox_max_x");
                        bBoxMaxVec.y    = moduleIt->getAttributeValue<float>("bBox_max_y");
                        bBoxMaxVec.z    = moduleIt->getAttributeValue<float>("bBox_max_z");
                        
                        module->setBoundingBox( bBoxMinVec, bBoxMaxVec );
                    }
                }
            }
            catch ( ... )
            {
                console() << "loadModuleSettings: Module parsing error!" << endl;
                return;
            }
        }
        
        // parse Params
        for(XmlTree::Iter paramIt = settingsXml.begin("settings/params/param"); paramIt != settingsXml.end(); ++paramIt)
        {
            paramName       = paramIt->getAttributeValue<string>("name");
            paramTrackIdx   = paramIt->getAttributeValue<int>("track");
            paramDeviceIdx  = paramIt->getAttributeValue<int>("device");
            paramIdx        = paramIt->getAttributeValue<int>("index");
            paramValue      = paramIt->getAttributeValue<float>("value");
            
            setParam( paramTrackIdx, paramDeviceIdx, paramIdx, paramValue );
        }   
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
}
