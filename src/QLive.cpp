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
    //	Logger::log("Live > Initialized!");
        
        
        mAnalyzer   = new QLiveAnalyzer( mOscAnalyzerInPort );
        
        mLiveParams = new QLiveParams(mOscLiveParamsInPort);   
    //    getParams();
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
        
        if ( mLiveParams )
        {
            mLiveParams->shutdown();
            delete mLiveParams;
            mLiveParams = NULL;
        }
        
        if ( mLiveParams )
        {
            mLiveParams->shutdown();
            delete mLiveParams;
            mLiveParams = NULL;
        }
    //	Logger::log("Live > shutdown!");
        
        
    }


    bool QLive::getInfo() 
    { 
        mIsReady = false;
        
        if ( getElapsedSeconds() - mGetInfoRequestAt < GET_INFO_MIN_DELAY )
            return false;
        
    //	Logger::log("Live > getInfo()");
        
        mSelectedTrack = NULL;
        
        mGetInfoRequestAt = getElapsedSeconds();

        deleteObjects();
        
        sendMessage("/live/name/scene");

        ci::sleep(50);
        sendMessage("/live/name/track"); 

        ci::sleep(200);
        sendMessage("/live/name/clip");    
        
        ci::sleep(200);
        sendMessage("/live/devicelist");
        
        ci::sleep(500);
        
        mIsReady = true;
        
        return mIsReady;
    }


    void QLive::deleteObjects()
    {
        for(int k=0; k < mTracks.size(); k++) {
            delete mTracks[k];
        }
        mTracks.clear();
        
        for(int k=0; k < mScenes.size(); k++)
            delete mScenes[k];
        mScenes.clear();
        
        for(int k=0; k < mClips.size(); k++)
            delete mClips[k];
        mClips.clear();
        
        for(int k=0; k < mDevices.size(); k++)
            delete mDevices[k];
        mDevices.clear();
    }


    void QLive::renderDebug() 
    {
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
        
        /*
        textLayout.addLine( "SCENES" );
        textLayout.addLine( "index\tname" );
        for (int k = 0; k < mScenes.size(); k++)
            textLayout.addLine( toString(mScenes[k]->mIndex) + "\t\t" + mScenes[k]->mName );
        textLayout.addLine( " " );	
        */
        textLayout.addLine( "TRACKS" );
        textLayout.addLine( "index\tname" );
        
        for (int k = 0; k < mTracks.size(); k++)
            textLayout.addLine( toString(mTracks[k]->mIndex) + "\t\t" + mTracks[k]->mName );
        textLayout.addLine( " " );
        
        textLayout.addLine( "CLIPS" );
        textLayout.addLine( "index\ttrack\t\tname" );
        for (int k = 0; k < mClips.size(); k++)
        {
            if ( mClips[k]->getState() == CLIP_PLAYING )
                textLayout.setColor( mColor1 );
            else
                textLayout.setColor( Color::white() );
                
            textLayout.addLine( toString(mClips[k]->mIndex) + "\t\t" + toString(mClips[k]->mTrackIndex) + "\t\t" + mClips[k]->mName );
        }
        textLayout.addLine( " " );

        textLayout.setColor( Color::white() );	
        
        textLayout.addLine( "DEVICES" );
        textLayout.addLine( "index\ttrack\t\tname" );
        for (int k = 0; k < mDevices.size(); k++)
            textLayout.addLine( toString(mDevices[k]->mIndex) + "\t\t" + toString(mDevices[k]->mTrackIndex) + "\t\t" + mDevices[k]->mName );
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
        
        //console() << "parse scene: " << endl;
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

        sort (mTracks.begin(), mTracks.end(), sortTracksByIndex); 

        sendMessage("/live/volume", "i" + toString(index) );	// get track volume

        listTrackDevices(index);
    }


    void QLive::parseClip( osc::Message message ) 
    {
        int     trackIndex	= message.getArgAsInt32(0);
        int 	index		= message.getArgAsInt32(1);
        string 	name		= message.getArgAsString(2);
        ColorA  color       = colorIntToColorA( message.getArgAsInt32(3) );
        
        // return if the clips already exists
        for(int k=0; k < mClips.size(); k++)
            if ( mClips[k]->mIndex == index && mClips[k]->mTrackIndex == trackIndex )
                return;

        QLiveClip *clip = new QLiveClip( index, name, trackIndex, color );
        
        mClips.push_back( clip );
        if ( trackIndex < mTracks.size() )
            mTracks[trackIndex]->addClip( clip );
        
        sendMessage("/live/clip/info", "i" + toString(trackIndex) + " i" + toString(index) );	// get clip info
    }


    void QLive::parseClipInfo( osc::Message message ) 
    {    
        int trackIndex	= message.getArgAsInt32(0);
        int index		= message.getArgAsInt32(1);
        ClipState state	= (ClipState)message.getArgAsInt32(2);
        
        for(int k=0; k < mClips.size(); k++)
        {
            if ( mClips[k]->getTrackIndex() == trackIndex && mClips[k]->mIndex == index)
                mClips[k]->setState(state);
            
            if ( state == CLIP_PLAYING && mClips[k]->getTrackIndex() == trackIndex && mClips[k]->mIndex != index )     // stop clip
                mClips[k]->setState( HAS_CLIP );
        }
    }


    void QLive::parseDevice( osc::Message message ) 
    {
        if ( message.getNumArgs() < 3 )	// seems there is an error in the APIs!
           return;
        
        int			trackIndex = message.getArgAsInt32(0);
        int			index;
        string		name;
        QLiveDevice	*dev;
        bool		deviceExists;
        
        for (int k=1; k < message.getNumArgs(); k+=2) 
        {
            index		= message.getArgAsInt32(k);
            name		= message.getArgAsString(k+1);
            
            deviceExists = false;
            
            // update if the device already exists
            for(int k=0; k < mDevices.size(); k++)
                if ( mDevices[k]->mName == name && mDevices[k]->mTrackIndex == trackIndex )
                {
                    mDevices[k]->mName = name;
                    deviceExists = true;
                }
                
                
            if ( !deviceExists )
            {
                dev = new QLiveDevice( index, name, trackIndex );	
                mDevices.push_back( dev );
                mTracks[trackIndex]->addDevice( dev );
            }
            
            // get device params	
            sendMessage("/live/device", "i" + toString(trackIndex) + " i" + toString(index) );
        }
    }


    void QLive::parseDeviceParams( osc::Message message )
    {
        //	int trackIdx, int deviceIdx, int parameterIdx, int value,  str name, ...
        if ( message.getNumArgs() < 5 )
            return;
        
        int			trackIdx    = message.getArgAsInt32(0);
        int			deviceIdx   = message.getArgAsInt32(1);
        QLiveTrack	*track      = getTrack(trackIdx);
        QLiveDevice	*device     = track->mDevices[deviceIdx];

        int         paramIdx;
        float       paramVal;
        std::string paramName;
        
        for (int k=2; k < message.getNumArgs(); k+=3) 
        {
            paramIdx    = message.getArgAsInt32(k);
            paramVal    = message.getArgAsFloat(k+1);
            paramName   = message.getArgAsString(k+2);
            
            device->addParam( paramIdx, paramVal, paramName );
        }
        
    }


    void QLive::receiveData(){

        while( mOscListener ) {

            while (mOscListener->hasWaitingMessages()) {
                osc::Message message;
                mOscListener->getNextMessage(&message);
                
                string	msgAddress = message.getAddress();

                // debug
                if (false)
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
                
            //	console() << "receive DATA! " << msgAddress << endl;
                
                // Parse Live objects
                if ( msgAddress == "/live/name/scene" ) {
                    parseScene(message);
                }
                else if ( msgAddress == "/live/name/track" ) {
                    parseTrack(message);
                }
                else if ( msgAddress == "/live/name/clip" ) {
                    parseClip(message);	
    //                console() << "parse clip" << std::endl;
                }
                else if ( msgAddress == "/live/clip/info" ) {
                    parseClipInfo(message);
                }
                else if ( msgAddress == "/live/devicelist" ) {
                    parseDevice(message);	
                }
                else if ( msgAddress == "/live/device/allparam" ) {
                    parseDeviceParams(message);
                }
                else if ( msgAddress == "/live/play" )
                {
                    mIsPlaying = ( (ClipState)message.getArgAsInt32(0) == 2 ) ? true : false;
                }

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

                else if ( msgAddress == "/live/param" )
                {
                    
                }
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
            track = mTracks[k];
            
            for( int i=0; i < track->getDevicesN(); i++ )
            {
                device = track->getDevice(i);
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

}
