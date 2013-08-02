/*
 *  QLiveModuleWithFixtures.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 *  REQUIRES: Scene from NocteBlock
 *
 */


#ifndef QLIVE_MODULE_WITH_FIXTURES
#define QLIVE_MODULE_WITH_FIXTURES

#pragma once

#include "QLiveModule.h"
#include "Fixture.h"


class QLive;
class QLiveClip;
class QLiveTrack;

class QLiveModuleWithFixtures;
typedef std::shared_ptr<QLiveModuleWithFixtures> QLiveModuleWithFixturesRef;


class QLiveModuleWithFixtures : public QLiveModule {
    

protected:      // only sub-class can call the constructor
    
    QLiveModuleWithFixtures( QLiveRef live, QLiveTrackRef track, QLiveClipRef clip, std::vector<FixtureRef> fixtures )
    : QLiveModule(live, track, clip), mFixtures(fixtures)
    {
        mBoundingBox        = ci::AxisAlignedBox3f( ci::Vec3f(-1.0f, 0.0f, -1.0f), ci::Vec3f::one() );
        mShowBoundingBox    = false;
        
        calcActiveFixtures();
    }

public:
    
    ~QLiveModuleWithFixtures() {}
    
    void setBoundingBox( ci::Vec3f minVec, ci::Vec3f maxVec )
    {            
        mBoundingBox = ci::AxisAlignedBox3f( minVec, maxVec );

        calcActiveFixtures();
    }   
    
    ci::AxisAlignedBox3f getBoundingBox() { return mBoundingBox; }
    
    virtual ci::XmlTree getXmlNode()
    {
        ci::XmlTree node        = QLiveModule::getXmlNode();
        ci::Vec3f   bBoxMinVec  = mBoundingBox.getMin();
        ci::Vec3f   bBoxMaxVec  = mBoundingBox.getMax();
        
        node.setAttribute( "bBox_min_x", bBoxMinVec.x ); node.setAttribute( "bBox_min_y", bBoxMinVec.y ); node.setAttribute( "bBox_min_z", bBoxMinVec.z );
        node.setAttribute( "bBox_max_x", bBoxMaxVec.x ); node.setAttribute( "bBox_max_y", bBoxMaxVec.y ); node.setAttribute( "bBox_max_z", bBoxMaxVec.z );
        
        return node;
    }
    
    virtual void loadXmlNode( ci::XmlTree node )
    {
        QLiveModule::loadXmlNode( node );
        ci::Vec3f bBoxMinVec;
        ci::Vec3f bBoxMaxVec;
        
        bBoxMinVec.x    = node.getAttributeValue<float>("bBox_min_x");
        bBoxMinVec.y    = node.getAttributeValue<float>("bBox_min_y");
        bBoxMinVec.z    = node.getAttributeValue<float>("bBox_min_z");
        
        bBoxMaxVec.x    = node.getAttributeValue<float>("bBox_max_x");
        bBoxMaxVec.y    = node.getAttributeValue<float>("bBox_max_y");
        bBoxMaxVec.z    = node.getAttributeValue<float>("bBox_max_z");
        
        setBoundingBox( bBoxMinVec, bBoxMaxVec );
    }        
    
    void showBoundingBox( bool val = true ) { mShowBoundingBox = val; }
    
    bool isShowBoundingBox() { return mShowBoundingBox; }
    
protected:
    
    void renderBoundingBox()
    {
        if ( !mShowBoundingBox )
            return;
        
        ci::ColorA  boxCol = mClip->getColor();
        
        // draw stroke
        ci::gl::color( boxCol );
        ci::gl::drawStrokedCube(mBoundingBox);
        
        boxCol.a = 0.1f;
        
        // draw cube
        ci::gl::color( boxCol );
        ci::gl::drawCube( mBoundingBox.getCenter(), mBoundingBox.getSize() );
        
        ci::gl::color( ci::ColorA::white() );
    }
    
    void calcActiveFixtures()
    {
        mActiveFixtureIndices.clear();
        
        ci::Vec3f minVec = mBoundingBox.getMin();
        ci::Vec3f maxVec = mBoundingBox.getMax();
        
        ci::Rectf rectXY( minVec.xy(), maxVec.xy() );
        ci::Rectf rectXZ( minVec.xz(), maxVec.xz() );
        
        ci::Vec3f pos3f;
        
        for( size_t k=0; k < mFixtures.size(); k++ )
        {
            pos3f = mFixtures[k]->getPos();
            
            if ( rectXY.contains( pos3f.xy() ) && rectXZ.contains( pos3f.xz() ) )
                mActiveFixtureIndices.push_back( k );
        }
    }
    
    void renderSquares( float height )
    {   
        ci::Vec3f   pos;
        float       size = 0.1f;
        FixtureRef  fixture;
        
        for ( size_t k=0; k < mActiveFixtureIndices.size(); k++ )
        {
            fixture = mFixtures[mActiveFixtureIndices[k]];
            pos = fixture->getPos();
            
            ci::gl::color(  mClip->getColor() * fixture->getValue() );
            glBegin(GL_QUADS);
            ci::gl::vertex( pos + ci::Vec3f(- size , height, - size ) );
            ci::gl::vertex( pos + ci::Vec3f(+ size , height, - size ) );
            ci::gl::vertex( pos + ci::Vec3f(+ size , height, + size ) );
            ci::gl::vertex( pos + ci::Vec3f(- size , height, + size ) );
            glEnd();
            
            ci::gl::color(  mClip->getColor() );
            glBegin(GL_LINE_LOOP);
            ci::gl::vertex( pos + ci::Vec3f(- size , height, - size ) );
            ci::gl::vertex( pos + ci::Vec3f(+ size , height, - size ) );
            ci::gl::vertex( pos + ci::Vec3f(+ size , height, + size ) );
            ci::gl::vertex( pos + ci::Vec3f(- size , height, + size ) );
            glEnd();
        }
        
        ci::gl::color( ci::Color::white() );
    }
    
    std::vector<int>                mActiveFixtureIndices;
    std::vector<FixtureRef>         mFixtures;
    ci::AxisAlignedBox3f            mBoundingBox;
    bool                            mShowBoundingBox;
    
};

#endif