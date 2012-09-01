/*
 *  QLiveObject.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#ifndef QLIVE_OBJECT
#define QLIVE_OBJECT

#pragma once

#include "cinder/Xml.h"
#include "cinder/Function.h"

namespace nocte {
    
    
    enum ClipState {
        NO_CLIP,			// 0
        HAS_CLIP,			// 1
        CLIP_PLAYING,		// 2
        CLIP_TRIGGERED		// 3
    };


    class QLiveObject {
        
        friend class QLive;

    public:
        
        QLiveObject( int index, std::string name ) : mIndex(index), mName(name) { }
        
        ~QLiveObject() {}
        
        int			getIndex() { return mIndex; }
        
        std::string getName() { return mName; }
        
    protected:
        
        virtual ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node( "object", "" );
            node.setAttribute( "name", mName );
            node.setAttribute( "index", mIndex );

            return node;
        }
        
        virtual void loadXmlNode( ci::XmlTree node )
        {
            mName   = node.getAttributeValue<std::string>( "name" );
            mIndex  = node.getAttributeValue<int>( "index" );
        }
        
        std::string colorToHex( const ci::ColorA &color ) {
            unsigned int a = ((unsigned int) (color.a * 255) & 0xFF) << 24;
            unsigned int r = ((unsigned int) (color.r * 255) & 0xFF) << 16;
            unsigned int g = ((unsigned int) (color.g * 255) & 0xFF) << 8;
            unsigned int b = ((unsigned int) (color.b * 255) & 0xFF);
            
            unsigned int value = a + r + g + b;
            
            std::stringstream clr;
            clr << std::hex << value;
            
            return clr.str();
        }
        
        ci::ColorA hexToColor( const std::string &hex ) {
            std::stringstream converter(hex);
            unsigned int value;
            converter >> std::hex >> value;
            
            float a = ((value >> 24) & 0xFF) / 255.0f;
            float r = ((value >> 16) & 0xFF) / 255.0f;
            float g = ((value >> 8) & 0xFF) / 255.0f;
            float b = ((value) & 0xFF) / 255.0f;
            
            return ci::ColorA(r, g, b, a);
        }
        
        
        int			mIndex;
        std::string	mName;
        
    };

    
    class QLiveClip : public QLiveObject {
        
        friend class QLive;
        friend class QLiveTrack;
        
    public:
        
        QLiveClip(int index, std::string name, ci::Color color = ci::Color::white() ) : QLiveObject(index, name), mColor(color) 
        { 
            mState		= HAS_CLIP;
            mIsPlaying  = false;
        };
        
        ~QLiveClip() {}

        ClipState	getState() { return mState; };
        
        void		setState(ClipState state) 
        { 
            if ( state == mState )
                return;

            mState = state; 
            
            mIsPlaying = ( mState == CLIP_PLAYING )  ? true : false;
            
            callStateUpdateCallbacks();
        };
        
        bool *getIsPlayingRef() { return &mIsPlaying; };
        
        bool		isPlaying() { return ( mState == CLIP_PLAYING ); };
        
        ci::ColorA	getColor() { return mColor; };
        
        void addStateUpdateCallback( const std::function<void ()> &callback )
        {
            std::shared_ptr<std::function<void ()> > callbackPtr( new std::function<void ()>( callback ) );
            mStateUpdateCallbacks.push_back( callbackPtr );
        }    
        
        
    protected:

        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = QLiveObject::getXmlNode();
            node.setTag("clip");
            node.setAttribute( "color", colorToHex(mColor) );
            
            return node;
        }
        
        void loadXmlNode( ci::XmlTree node )
        {
            QLiveObject::loadXmlNode( node );
            
            mColor  = hexToColor( node.getAttributeValue<std::string>( "color" ) );
        }
    
    protected:
    
        ci::ColorA	mColor;
        bool        mIsPlaying;

    private:
        
        ClipState	mState;
        std::vector<std::shared_ptr<std::function<void()> > >   mStateUpdateCallbacks;
        
    private:
    
        void callStateUpdateCallbacks()
        {
            for( size_t k=0; k < mStateUpdateCallbacks.size(); k++ )
            {
                std::function<void ()> *fn = reinterpret_cast<std::function<void ()>*>( mStateUpdateCallbacks[k].get() );
                (*fn)();
            }
        }
    
    };


    class QLiveParam : public QLiveObject {
        
        friend class QLive;
        friend class QLiveDevice;
        
    public:
        
        QLiveParam( int index, std::string name, float value = 0.0f, float minVal = 0.0f, float maxVal = 1.0f ) 
        : QLiveObject(index, name), mValue(value), mMinValue(minVal), mMaxValue(maxVal) {}
        
        ~QLiveParam() {}
        
        float getValue() { return mValue; }
        
        float* getRef() { return &mValue; }
        
        float getMin() { return mMinValue; }
        
        float getMax() { return mMaxValue; }
        
    protected:
        
        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = QLiveObject::getXmlNode();
            node.setTag("param");
            node.setAttribute( "value", mValue );
            node.setAttribute( "min",   mMinValue);
            node.setAttribute( "max",   mMaxValue );

            return node;
        }
        
        void loadXmlNode( ci::XmlTree node )
        {
            QLiveObject::loadXmlNode( node );

            mValue      = node.getAttributeValue<float>( "value" );
            mMinValue   = node.getAttributeValue<float>( "min" );
            mMaxValue   = node.getAttributeValue<float>( "max" );
        }
        
    protected:
        
        float       mValue;
        float       mMinValue;
        float       mMaxValue;
        
    };


    class QLiveDevice : public QLiveObject {
        
        friend class QLive;
        friend class QLiveTrack;
        
    public:
        
        QLiveDevice( int index, std::string name ) : QLiveObject(index, name) {}
        
        ~QLiveDevice()
        {
            for( size_t k=0; k < mParams.size(); k++ )
                delete mParams[k];
            mParams.clear();
        }
        
        void addParam( int index, float value, std::string name, float minVal, float maxVal ) { mParams.push_back( new QLiveParam( index, name, value, minVal, maxVal ) ); }
        
        std::vector<QLiveParam*> getParams() { return mParams; }
        
        QLiveParam* getParam( int idx ) 
        { 
            if ( idx < mParams.size() )
                return mParams[idx]; 
            
            QLiveParam* param = NULL;
            return param;
        }
        
        QLiveParam* getParam( std::string name )
        {
            for( size_t k=0; k < mParams.size(); k++ )
                if( mParams[k]->mName == name )
                    return mParams[k];
            
            QLiveParam *param = NULL;
            return param;
        }
        
        float getParamValue( std::string name )
        {
            QLiveParam *param = getParam(name);
            if ( param )
                return param->getValue();
                    
            return 0;
        }
        
        float* getParamRef( std::string name )
        {   
            QLiveParam *param = getParam(name);
            if ( param )
                return param->getRef();
            
            float *ref = NULL;
            return ref;
        }
        
    protected:
        
        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = QLiveObject::getXmlNode();
            node.setTag("device");
            
            for( size_t k=0; k < mParams.size(); k++ )
                node.push_back( mParams[k]->getXmlNode() );
            
            return node;
        }
        
        void loadXmlNode( ci::XmlTree node, bool forceXmlSettings = false )
        {
            QLiveObject::loadXmlNode( node );
            
            QLiveParam      *param;
            int             index;
            std::string     name;
            
            // parse params
            for( ci::XmlTree::Iter nodeIt = node.begin("param"); nodeIt != node.end(); ++nodeIt )
            {
                index   = nodeIt->getAttributeValue<int>("index");
                name    = nodeIt->getAttributeValue<std::string>("name");
                param   = getParam(name);

                if ( param )
                    param->loadXmlNode( *nodeIt );
                    
                else if ( !param && forceXmlSettings )
                {
                    param = new QLiveParam( index, name );
                    param->loadXmlNode( *nodeIt );
                    mParams.push_back( param );
                }
            }
            
        }
        
    protected:
        
        std::vector<QLiveParam*>    mParams;

    };


    class QLiveTrack : public QLiveObject {
        
        friend class QLive;
        
    public:
     
        QLiveTrack( int index, std::string name, ci::Color color = ci::Color::white() ) : QLiveObject(index, name), mColor(color), mVolume(0.0f) {}
        
        ~QLiveTrack() 
        {
            for(int k=0; k < mClips.size(); k++)
                delete mClips[k];
            mClips.clear();
            
            for(int k=0; k < mDevices.size(); k++)
                delete mDevices[k];
            mDevices.clear();
        }
        
        void addClip( QLiveClip *obj )		{ mClips.push_back(obj); }
        void addDevice( QLiveDevice *obj )	{ mDevices.push_back(obj); }
            
        
        std::vector<QLiveClip*> getClips() { return mClips; }
        
        std::vector<QLiveDevice*> getDevices() { return mDevices; }

        QLiveClip *getClip( int idx ) 
        { 
            for( size_t k=0; k < mClips.size(); k++ )
                if ( mClips[k]->mIndex == idx )
                    return mClips[k];
            
            QLiveClip *clip = NULL;
            return clip;
        }
        
        QLiveClip *getClip( const std::string &name ) 
        { 
            for( size_t k=0; k < mClips.size(); k++ )
                if ( mClips[k]->mName == name )
                    return mClips[k];
            
            QLiveClip *clip = NULL;  
            return clip;
        }

        void clearPointers() { mClips.clear(); mDevices.clear(); }
        
        float getVolume() { return mVolume; }

        float* getVolumeRef() { return &mVolume; }
        
        QLiveDevice* getDevice( int idx ) 
        {   
            for( size_t k=0; k < mDevices.size(); k++ )
                if ( mDevices[k]->mIndex == idx )
                    return mDevices[k];
            
            QLiveDevice *device = NULL;
            return device;
        }

        
        QLiveDevice* getDevice( const std::string &name ) 
        { 
            for( size_t k=0; k < mDevices.size(); k++ )
                if ( mDevices[k]->mName == name )
                    return mDevices[k];
            
            QLiveDevice *device = NULL;
            return device;
        }

        
        ci::ColorA	getColor() { return mColor; };
        
    protected:
        
        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = QLiveObject::getXmlNode();
            node.setTag("track");
            node.setAttribute( "volume", mVolume );
            node.setAttribute( "color", colorToHex(mColor) );
            
            ci::XmlTree clips( "clips", "" );
            ci::XmlTree devices( "devices", "" );
            
            for( size_t k=0; k < mClips.size(); k++ )
                clips.push_back( mClips[k]->getXmlNode() );

            for( size_t k=0; k < mDevices.size(); k++ )
                devices.push_back( mDevices[k]->getXmlNode() );
            
            node.push_back( clips );
            node.push_back( devices );
            
            return node;
        }
        
        void loadXmlNode( ci::XmlTree node, bool forceXmlSettings = false ) 
        {
            QLiveObject::loadXmlNode( node );
            mVolume = node.getAttributeValue<float>("volume");
            mColor  = hexToColor( node.getAttributeValue<std::string>("color") );
            
            QLiveClip       *clip;
            QLiveDevice     *device;
            int             index;
            std::string     name;
            
            // parse clips
            for( ci::XmlTree::Iter nodeIt = node.begin("clips/clip"); nodeIt != node.end(); ++nodeIt )
            {
                index   = nodeIt->getAttributeValue<int>("index");
                name    = nodeIt->getAttributeValue<std::string>("name");
                clip    = getClip(name);
 
                if ( clip )
                    clip->loadXmlNode( *nodeIt );
                    
                else if ( !clip && forceXmlSettings )
                {
                    clip = new QLiveClip( index, name, ci::Color::white() );
                    clip->loadXmlNode( *nodeIt );
                    mClips.push_back( clip );
                }
                
            }
            
            
            // parse devices            
            for( ci::XmlTree::Iter nodeIt = node.begin("devices/device"); nodeIt != node.end(); ++nodeIt )
            {
                index   = nodeIt->getAttributeValue<int>("index");
                name    = nodeIt->getAttributeValue<std::string>("name");
                device  = getDevice(name);

                if ( device )
                    device->loadXmlNode( *nodeIt, forceXmlSettings );
                    
                else if ( !device && forceXmlSettings )
                {
                    device = new QLiveDevice( index, name );
                    device->loadXmlNode( *nodeIt, forceXmlSettings );
                    mDevices.push_back( device );
                }
            }
        
        }
        
    protected:        
        
        std::vector<QLiveClip*>		mClips;
        std::vector<QLiveDevice*>	mDevices;
        float						mVolume;
        ci::ColorA                  mColor;
        
    };


    class QLiveScene : public QLiveObject {

        friend class QLive;
        
    public:
        
        QLiveScene( int index, std::string name ) : QLiveObject(index, name) {};

        ~QLiveScene() {}
        
    protected:
        
        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = QLiveObject::getXmlNode();
            node.setTag("scene");

            return node;
        }

    };


}

#endif