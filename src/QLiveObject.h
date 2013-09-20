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
#include <boost/signals2.hpp>

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

public:
    
    QLiveObject( int index, std::string name ) : mIndex(index), mName(name), mIsSelected(false) { }
    
    ~QLiveObject() {}
    
    int			getIndex() { return mIndex; }
    
    std::string getName() { return mName; }
    
    bool isSelected() { return mIsSelected; }
    
    void setName( std::string name ) { mName = name; }

    void setIndex( int index ) { mIndex = index; }
    
    virtual void select( bool val )
    {
        mIsSelected = val;
        
        if ( val )
            mOnSelectSignal();
    }
    
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
    
    
protected:
    
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
    
    bool                            mIsSelected;
    boost::signals2::signal<void()> mOnSelectSignal;

private:

    int                             mIndex;
    std::string                     mName;
    
    
private:
    // disallow
    QLiveObject(const QLiveObject&);
    QLiveObject& operator=(const QLiveObject&);
    
};


/* ------------------ */
/* --- QLive Clip --- */
/* ------------------ */

class QLiveClip : public QLiveObject {
    
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
        
        mOnStateUpdateSignal();
    };
    
    bool        *getIsPlayingRef() { return &mIsPlaying; }
    
    bool		isPlaying() { return mIsPlaying; }
    
    ci::ColorA	getColor() { return mColor; }
    
    boost::signals2::connection connectOnStateUpdate( const std::function<void ()> &f )    { return mOnStateUpdateSignal.connect( f ); }

    boost::signals2::connection connectOnSelect( const std::function<void ()> &f )         { return mOnSelectSignal.connect( f ); }

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
    
    void setColor( ci::Color col ) { mColor = col; }
    
    
protected:
    
    QLiveClip( int index, std::string name, ci::Color color ) : QLiveObject(index, name), mColor(color)
    {
        mState		= HAS_CLIP;
        mIsPlaying  = false;
    }

protected:

    ci::ColorA	mColor;
    bool        mIsPlaying;

private:
    
    ClipState	mState;
    
    boost::signals2::signal<void()>     mOnStateUpdateSignal;
    
    
private:
/*
    void callStateUpdateCallbacks()
    {
        for( size_t k=0; k < mStateUpdateCallbacks.size(); k++ )
        {
            std::function<void ()> *fn = reinterpret_cast<std::function<void ()>*>( mStateUpdateCallbacks[k].get() );
            (*fn)();
        }
    }
*/
};


/* ------------------- */
/* --- QLive Param --- */
/* ------------------- */

class QLiveParam : public QLiveObject {
    
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
    
    void setRange( float minVal, float maxVal )
    {
        mMinValue = minVal;
        mMaxValue = maxVal;
    }
    
    ci::XmlTree getXmlNode()
    {
        ci::XmlTree node = QLiveObject::getXmlNode();
        node.setTag("param");
        node.setAttribute( "value", *mValue.get() );
        node.setAttribute( "min",   mMinValue);
        node.setAttribute( "max",   mMaxValue );
        
        return node;
    }
    
    void loadXmlNode( ci::XmlTree node, bool forceXmlSettings )
    {
        QLiveObject::loadXmlNode( node );
        
        if ( forceXmlSettings )
        {
            *mValue.get()   = node.getAttributeValue<float>( "value" );
            mMinValue       = node.getAttributeValue<float>( "min" );
            mMaxValue       = node.getAttributeValue<float>( "max" );
        }
    }
    
protected:
   
    QLiveParam( int index, std::string name, float val, float minVal, float maxVal )
    : QLiveObject(index, name), mMinValue(minVal), mMaxValue(maxVal)
    {
        mValue = std::shared_ptr<float>( new float(val) );
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
    
    QLiveParamRef getParamByIndex( int idx )
    {
        for( size_t k=0; k < mParams.size(); k++ )
            if ( mParams[k]->getIndex() == idx )
                return mParams[k];
        
        return QLiveParamRef();
    }
    
    QLiveParamRef getParamByName( std::string name )
    {
        for( size_t k=0; k < mParams.size(); k++ )
            if( mParams[k]->getName() == name )
                return mParams[k];
        
        return QLiveParamRef();
    }
    
    float getParamValue( std::string name )
    {
        QLiveParamRef param = getParamByName(name);
        if ( param )
            return param->getValue();
                
        return 0;
    }
    
    std::shared_ptr<float> getParamRef( std::string name )
    {   
        QLiveParamRef param = getParamByName(name);

        if ( param )
            return param->getRef();
        
        return std::shared_ptr<float>();
    }
    
    ci::XmlTree getXmlNode()
    {
        ci::XmlTree node = QLiveObject::getXmlNode();
        node.setTag("device");
        
        for( size_t k=0; k < mParams.size(); k++ )
            node.push_back( mParams[k]->getXmlNode() );
        
        return node;
    }
    
    void loadXmlNode( ci::XmlTree node, bool forceXmlSettings )
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
            param   = getParamByIndex(index);
            
            if ( param )
                param->loadXmlNode( *nodeIt, forceXmlSettings );
            
            else if ( !param && forceXmlSettings )
            {
                param = QLiveParam::create( index, name );
                param->loadXmlNode( *nodeIt, forceXmlSettings );
                mParams.push_back( param );
            }
        }
        
    }
    
protected:
   
    QLiveDevice( int index, const std::string name ) : QLiveObject(index, name) {}
    
protected:
    
    std::vector<QLiveParamRef>    mParams;
    
};


/* ------------------- */
/* --- QLive Track --- */
/* ------------------- */

class QLiveTrack : public QLiveObject {
    
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

    QLiveClipRef getClipByIndex( int idx )
    { 
        for( size_t k=0; k < mClips.size(); k++ )
            if ( mClips[k]->getIndex() == idx )
                return mClips[k];
        
        return QLiveClipRef();
    }

    QLiveDeviceRef getDeviceByIndex( int idx )
    {
        for( size_t k=0; k < mDevices.size(); k++ )
            if ( mDevices[k]->getIndex() == idx )
                return mDevices[k];
        
        return QLiveDeviceRef();
    }

    // TODO double check what this is used for, now I'm using shared ptr <<<<<<<<<<<<<<<<<<<<<<<<
    void clearPointers() { mClips.clear(); mDevices.clear(); }
    
    float getVolume() { return *mVolume.get(); }
    void setVolume( float val ) { *mVolume.get() = val; }
    
    std::shared_ptr<float> getVolumeRef() { return mVolume; }
    
    ci::ColorA	getColor() { return mColor; }
    
    QLiveClipRef getPlayingClip()
    {
        for( size_t k=0; k < mClips.size(); k++ )
            if ( mClips[k]->isPlaying() )
                return mClips[k];
        
        return QLiveClipRef();
    }
    
    ci::XmlTree getXmlNode()
    {
        ci::XmlTree node = QLiveObject::getXmlNode();
        node.setTag("track");
        node.setAttribute( "volume", *mVolume.get() );
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
    
    void loadXmlNode( ci::XmlTree node, bool forceXmlSettings )
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
            clip    = getClipByIndex(index);
            
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
            device  = getDeviceByIndex(index);
            
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
    
    QLiveTrack( int index, std::string name, ci::Color color ) : QLiveObject(index, name), mColor(color)
    {
        mVolume = std::shared_ptr<float>( new float( 0.0f ) );
    }
    
    
protected:        
    
    std::vector<QLiveClipRef>   mClips;
    std::vector<QLiveDeviceRef>	mDevices;
    std::shared_ptr<float>      mVolume;
    ci::ColorA                  mColor;
};


/* ------------------- */
/* --- QLive Scene --- */
/* ------------------- */

class QLiveScene : public QLiveObject {
    
public:
    
    static QLiveSceneRef create( int index, std::string name )
    {
        return QLiveSceneRef( new QLiveScene( index, name ) );
    }

    ~QLiveScene() {}
    
    ci::XmlTree getXmlNode()
    {
        ci::XmlTree node = QLiveObject::getXmlNode();
        node.setTag("scene");
        
        return node;
    }
    
protected:
    
    QLiveScene( int index, std::string name ) : QLiveObject(index, name) {}
    
};


#endif