/*
 *  QLiveParams.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QLIVE_PARAMS
#define QLIVE_PARAMS

#pragma once

#include "OscSender.h"
#include "OscListener.h"
#include "cinder/Thread.h"


class QLiveParams {
    
public:
    
    QLiveParams( int oscPort )
    {
        mOscListener    = NULL;
        mOscPort        = oscPort;
        
        initOsc();
    }
    
    
    void initOsc()
    {
        if ( mOscListener )					// close OSC listener
        {
            mOscListener->shutdown();		
            delete mOscListener;
            mOscListener = NULL;
            ci::sleep(50);
        }
       
        try 
        {
            mOscListener = new ci::osc::Listener();
            mOscListener->setup(mOscPort);
            ci::app::console() << "Live Param: OSC listener Initialized " << mOscPort << std::endl;
        } 
        catch (...) 
        {
            mOscListener = NULL;
            ci::app::console() << "Live Param: Failed to bind OSC listener socket " << mOscPort << std::endl;
        }
        
        boost::thread receiveDataThread( &QLiveParams::receiveData, this);
    }

    
    void addParam( std::string name, float defaultVal = 0.0f )
    {
        float *var = new float(defaultVal);
        mParams[name] = var;
    }
    
    
    void addParam( std::string name, float *var )
    {
        mParams[name] = var;
    }
    
    
    float getParam( std::string name )
    {
        if ( !hasParam(name) )
            addParam(name);
        
        return *mParams[name];
    }
    
    
    float* getParamRef( std::string name )
    {
        if ( !hasParam(name) )
            addParam(name);
        
        return mParams[name];
    }
    
    
    bool hasParam( std::string name ) { return ( mParams.count( name ) > 0 ); }
    

    void receiveData(){
        
        std::string paramName;
        float       paramValue;
        
        while( mOscListener ) {
            
            while (mOscListener->hasWaitingMessages()) {
                ci::osc::Message message;
                mOscListener->getNextMessage(&message);
                
                std::string	msgAddress = message.getAddress();
                
                // debug
                if (false)
                {
                    ci::app::console() << "LIVE: Address: " << message.getAddress() << " ";
                    
                    for (int i = 0; i < message.getNumArgs(); i++) {
                        if (message.getArgType(i) == ci::osc::TYPE_INT32){
                            try {
                                ci::app::console() << "INT " << message.getArgAsInt32(i) << " ";
                            }
                            catch (...) {
                                ci::app::console() << "Exception reading argument as int32" << std::endl;
                            }
                            
                        }else if (message.getArgType(i) == ci::osc::TYPE_FLOAT){
                            try {
                                ci::app::console() << "float " << message.getArgAsFloat(i) << " ";
                            }
                            catch (...) {
                                ci::app::console() << "Exception reading argument as float" << std::endl;
                            }
                        }else if (message.getArgType(i) == ci::osc::TYPE_STRING){
                            try {
                                ci::app::console() << "STRING " << message.getArgAsString(i).c_str() << " ";
                            }
                            catch (...) {
                                ci::app::console() << "Exception reading argument as string" << std::endl;
                            }
                            
                        }
                    }
                    ci::app::console() << std::endl;
                }
               
                if ( msgAddress == "/params" )
                {
                    paramName = message.getArgAsString(0);
                    
                    if ( message.getArgType(1) == ci::osc::TYPE_INT32 )
                        paramValue = message.getArgAsInt32(1);
                    
                    else if ( message.getArgType(1) == ci::osc::TYPE_FLOAT )
                        paramValue = message.getArgAsFloat(1);
                    
                    if ( mParams.count( paramName ) == 0 )
                        addParam( paramName, paramValue );
                    else
                        *mParams[paramName] = paramValue;
                }
            }
            
            ci::sleep(15.0f);
            //		boost::this_thread::sleep(boost::posix_time::milliseconds(15));
            
        }
        ci::app::console() << "Live Param > receiveData() thread exited!" << std::endl;
    }

    
    void shutdown()
    {
        if ( mOscListener )
        {
            mOscListener->shutdown();
            delete mOscListener;
            mOscListener = NULL;
        }
        
        for ( std::map<std::string, float*>::iterator it = mParams.begin(); it != mParams.end(); it++ )
            delete it->second;
        mParams.clear();
    }
    
public:
    
    ci::osc::Listener				*mOscListener;
    int                             mOscPort;
    
    std::map<std::string, float*>   mParams;

};


#endif
