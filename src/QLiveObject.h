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
    
    class QLiveClip;
    typedef std::shared_ptr<QLiveClip>      QLiveClipRef;
    
    class QLiveParam;
    typedef std::shared_ptr<QLiveParam>     QLiveParamRef;
    
    class QLiveDevice;
    typedef std::shared_ptr<QLiveDevice>	QLiveDeviceRef;
    
    class QLiveTrack;
    typedef std::shared_ptr<QLiveTrack>     QLiveTrackRef;
    
    class QLiveScene;
    typedef std::shared_ptr<QLiveScene>     QLiveSceneRef;
    
    
    enum ClipState {
        NO_CLIP,			// 0
        HAS_CLIP,			// 1
        CLIP_PLAYING,		// 2
        CLIP_TRIGGERED		// 3
    };
    
    /* -------------------- */
    /* --- QLive Object --- */
    /* -------------------- */
    
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
        
    private:
        // disallow
        QLiveObject(const QLiveObject&);
        QLiveObject& operator=(const QLiveObject&);
        
    };

    
    class QLiveClip : public QLiveObject {
        
        friend class QLive;
        friend class QLiveTrack;
        
    public:
        
        static QLiveClipRef create( int index, std::string name, ci::Color color = ci::Color::white() )
        {
            return QLiveClipRef( new QLiveClip( index, name, color ) );
        }
        
        ~QLiveClip() {}

        ClipState	getState() { return mState; }
        
        void		setState(ClipState state) 
        { 
            if ( state == mState )
                return;

            mState = state; 
            
            mIsPlaying = ( mState == CLIP_PLAYING )  ? true : false;
            
            callStateUpdateCallbacks();
        };
        
        bool        *getIsPlayingRef() { return &mIsPlaying; }
        
        bool		isPlaying() { return mIsPlaying; }
        
        ci::ColorA	getColor() { return mColor; }
        
        void addStateUpdateCallback( const std::function<void ()> &callback )
        {
            std::shared_ptr<std::function<void ()> > callbackPtr( new std::function<void ()>( callback ) );
            mStateUpdateCallbacks.push_back( callbackPtr );
        }    
        
        
    protected:
        
        QLiveClip( int index, std::string name, ci::Color color ) : QLiveObject(index, name), mColor(color)
        {
            mState		= HAS_CLIP;
            mIsPlaying  = false;
        }
        
        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = QLiveObject::getXmlNode();
            node.setTag("clip");
            node.setAttribute( "color", colorToHex(mColor) );
            
            return node;
        }
        
        void        loadXmlNode( ci::XmlTree node )
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

    
    /* ------------------- */
    /* --- QLive Param --- */
    /* ------------------- */
    
    class QLiveParam : public QLiveObject {
        
        friend class QLive;
        friend class QLiveDevice;
        
    public:
        
        static QLiveParamRef create( int index, std::string name, float val = 0.0f, float minVal = 0.0f, float maxVal = 1.0f )
        {
            return QLiveParamRef( new QLiveParam( index, name, val, minVal, maxVal ) );
        }
        
        ~QLiveParam() {}
        
        float getValue() { return *mValue.get(); }
        void setValue( float val ) { *mValue.get() = val; }
        
        std::shared_ptr<float> getRef() { return mValue; }
        
        float getMin() { return mMinValue; }
        
        float getMax() { return mMaxValue; }
        
    protected:
       
        QLiveParam( int index, std::string name, float val, float minVal, float maxVal )
        : QLiveObject(index, name), mMinValue(minVal), mMaxValue(maxVal)
        {
            mValue = std::shared_ptr<float>( new float(val) );
        }
        
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

            *mValue.get()   = node.getAttributeValue<float>( "value" );
            mMinValue       = node.getAttributeValue<float>( "min" );
            mMaxValue       = node.getAttributeValue<float>( "max" );
        }
        
    protected:
        
        std::shared_ptr<float>  mValue;
        float                   mMinValue;
        float                   mMaxValue;
        
    };

    
    /* -------------------- */
    /* --- QLive Device --- */
    /* -------------------- */
    
    class QLiveDevice : public QLiveObject {
        
        friend class QLive;
        friend class QLiveTrack;
        
    public:
        
        static QLiveDeviceRef create( int index, const std::string name )
        {
            return QLiveDeviceRef( new QLiveDevice( index, name ) );
        }
        
        ~QLiveDevice() {}
        
        void addParam( int index, float value, const std::string name, float minVal, float maxVal )
        {
            mParams.push_back( QLiveParam::create( index, name, value, minVal, maxVal ) );
        }
        
        std::vector<QLiveParamRef> getParams() { return mParams; }
        
        QLiveParamRef getParam( int idx )
        { 
            if ( idx < mParams.size() )
                return mParams[idx]; 
            
            return QLiveParamRef();
        }
        
        QLiveParamRef getParam( std::string name )
        {
            for( size_t k=0; k < mParams.size(); k++ )
                if( mParams[k]->mName == name )
                    return mParams[k];
            
            return QLiveParamRef();
        }
        
        float getParamValue( std::string name )
        {
            QLiveParamRef param = getParam(name);
            if ( param )
                return param->getValue();
                    
            return 0;
        }
        
        std::shared_ptr<float> getParamRef( std::string name )
        {   
            QLiveParamRef param = getParam(name);

            if ( param )
                return param->getRef();
            
            return std::shared_ptr<float>();
        }
        
    protected:
       
        QLiveDevice( int index, const std::string name ) : QLiveObject(index, name) {}
        
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
            
            QLiveParamRef   param;
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
                    param = QLiveParam::create( index, name );
                    param->loadXmlNode( *nodeIt );
                    mParams.push_back( param );
                }
            }
            
        }
        
    protected:
        
        std::vector<QLiveParamRef>    mParams;
        
    };

    
    /* ------------------- */
    /* --- QLive Track --- */
    /* ------------------- */
    
    class QLiveTrack : public QLiveObject {
        
        friend class QLive;
        
    public:
        
        static QLiveTrackRef create( int index, std::string name, ci::Color color = ci::Color::white() )
        {
            return QLiveTrackRef( new QLiveTrack( index, name, color ) );
        }
        
        ~QLiveTrack() {}
        
        void addClip( QLiveClipRef obj )		{ mClips.push_back(obj); }
        void addDevice( QLiveDeviceRef obj )	{ mDevices.push_back(obj); }
            
        
        std::vector<QLiveClipRef> getClips() { return mClips; }
        
        std::vector<QLiveDeviceRef> getDevices() { return mDevices; }

        QLiveClipRef getClip( int idx )
        { 
            for( size_t k=0; k < mClips.size(); k++ )
                if ( mClips[k]->mIndex == idx )
                    return mClips[k];
            
            return QLiveClipRef();
        }
        
        QLiveClipRef getClip( const std::string &name )
        { 
            for( size_t k=0; k < mClips.size(); k++ )
                if ( mClips[k]->mName == name )
                    return mClips[k];
            
            return QLiveClipRef();
        }

        // TODO double check what this is used for, now I'm using shared ptr <<<<<<<<<<<<<<<<<<<<<<<<
        void clearPointers() { mClips.clear(); mDevices.clear(); }
        
        float getVolume() { return *mVolume.get(); }
        void setVolume( float val ) { *mVolume.get() = val; }
        
        std::shared_ptr<float> getVolumeRef() { return mVolume; }
   
        QLiveDeviceRef getDevice( int idx )
        {   
            for( size_t k=0; k < mDevices.size(); k++ )
                if ( mDevices[k]->mIndex == idx )
                    return mDevices[k];
            
            return QLiveDeviceRef();
        }

        
        QLiveDeviceRef getDevice( const std::string &name )
        { 
            for( size_t k=0; k < mDevices.size(); k++ )
                if ( mDevices[k]->mName == name )
                    return mDevices[k];
            
            return QLiveDeviceRef();
        }

        ci::ColorA	getColor() { return mColor; }
        
        QLiveClipRef getPlayingClip()
        {
            for( size_t k=0; k < mClips.size(); k++ )
                if ( mClips[k]->isPlaying() )
                    return mClips[k];
            
            return QLiveClipRef();
        }
        
        
    protected:
        
        QLiveTrack( int index, std::string name, ci::Color color ) : QLiveObject(index, name), mColor(color)
        {
            mVolume = std::shared_ptr<float>( new float( 0.0f ) );
        }
        
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
            *mVolume.get()  = node.getAttributeValue<float>("volume");
            mColor          = hexToColor( node.getAttributeValue<std::string>("color") );
            
            QLiveClipRef    clip;
            QLiveDeviceRef  device;
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
                    clip = QLiveClip::create( index, name, ci::Color::white() );
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
                    device = QLiveDevice::create( index, name );
                    device->loadXmlNode( *nodeIt, forceXmlSettings );
                    mDevices.push_back( device );
                }
            }
        
        }
        
    protected:        
        
        std::vector<QLiveClipRef>   mClips;
        std::vector<QLiveDeviceRef>	mDevices;
        std::shared_ptr<float>      mVolume;
        ci::ColorA                  mColor;
        std::map<int, float>        mSends;
        
    };

    
    /* ------------------- */
    /* --- QLive Scene --- */
    /* ------------------- */
    
    class QLiveScene : public QLiveObject {

        friend class QLive;
        
    public:
        
        static QLiveSceneRef create( int index, std::string name )
        {
            return QLiveSceneRef( new QLiveScene( index, name ) );
        }

        ~QLiveScene() {}
        
    protected:
        
        QLiveScene( int index, std::string name ) : QLiveObject(index, name) {}
        
        ci::XmlTree getXmlNode() 
        {
            ci::XmlTree node = QLiveObject::getXmlNode();
            node.setTag("scene");

            return node;
        }
        
    };


}

#endif