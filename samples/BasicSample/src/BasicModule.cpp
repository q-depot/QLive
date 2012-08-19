/*
 *  BasicModule.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */


#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

//#include "Logger.h"
//#include "Analyzer.h"
//#include "Scene.h"
//#include "Fixture.h"
#include "QLiveModule.h"
#include "BasicModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace nocte;
//
//extern	ColorA	gColorWhite;
//
//extern	float	gLiveFuffaWidth;
//extern	float	gLiveFuffaHeight;
//extern	float	gLiveFuffaForce;
//extern	int		gLiveFuffaFreqOffset;



BasicModule::BasicModule( QLive *live, QLiveClip *clip ) : QLiveModule( live, clip )
{
    mFreqs.push_back( 2 );  mPoints.push_back( Vec2f::zero() );
    mFreqs.push_back( 10 ); mPoints.push_back( Vec2f::zero() );
    mFreqs.push_back( 15 ); mPoints.push_back( Vec2f::zero() );
    mFreqs.push_back( 20 ); mPoints.push_back( Vec2f::zero() );
    mFreqs.push_back( 25 ); mPoints.push_back( Vec2f::zero() );
    mFreqs.push_back( 30 ); mPoints.push_back( Vec2f::zero() );
    
    /*
     mName = "Fuffa";
     
     // LiveClip format => NAME pointsN offset spacing posX posY 
     vector<string>	splitValues;
     string			clipName = clip->getName();
     boost::split( splitValues, clipName, boost::is_any_of(" ") );
     int nParameters = 5;
     
     if ( (splitValues.size() - 1) % nParameters != 0 )
     {
     //		Logger::log("ERROR > Fuffa, wrong number of parameters! format => NAME pointsN offset spacing posX posY");
     //		Logger::log(clipName);
     exit(-1);
     }
     int objN = (splitValues.size() - 1) / nParameters;
     
     for(int k=0; k < objN; k++)
     {
     Vec2f pos( boost::lexical_cast<float>(splitValues[k * nParameters + 4]), boost::lexical_cast<float>(splitValues[k * nParameters + 5]) );
     
     int nPoints = boost::lexical_cast<float>(splitValues[k * nParameters + 1]);
     int fftOffset	= boost::lexical_cast<float>(splitValues[k * nParameters + 2]);
     int fftSpacing = boost::lexical_cast<float>(splitValues[k * nParameters + 3]);
     
     mBlobs.push_back( FuffaBlob(pos, nPoints, fftOffset, fftSpacing, analyzer) );
     
     mSettingsStr += toString(pos) + " . n " + toString(nPoints) + " . o " + toString(fftOffset) + " . s " + toString(fftSpacing) + "\t";
     }
     */
}


void BasicModule::update(float *values)
{
	if( !updateModulo() )
		return;
    
	// ********************************* //
	// Module implementation starts here //
	// ********************************* //
    
	/*	
     for (int k=0; k < mBlobs.size(); k++)
     mBlobs[k].update();
     
     // update fixtures
     for (int k=0; k < mScene->mFixtures.size(); k++) {
     Fixture *fixture = mScene->mFixtures[k];
     float val = ( containsPoint( fixture->getPos2f() ) ) ? 1.0f : 0.0f; 
     values[k] += val * mBrightness;
     }
     */
    
    float deltaA    = 3.14f / mFreqs.size();
    float maxRadius = 200.0f;
    float x, y;
    
    for( size_t k=0; k < mFreqs.size(); k++ )
    {
        x = cos(k*deltaA) * mLive->getFreqAmplitude( mFreqs[k], 0 );
        y = sin(k*deltaA) * mLive->getFreqAmplitude( mFreqs[k], 0 );
    }
}


void BasicModule::render()
{
	if ( !mIsPlaying )
		return;
	
	// ********************************* //
	// Module implementation starts here //
	// ********************************* //
	
    gl::color( Color( 1.0f, 0.0f, 0.0f ) );
    glBegin( GL_POLYGON );
    for( size_t k=0; k < mPoints.size(); k++ )
        gl::vertex( mPoints[k] );
    glEnd();
    
    
	/*
     gl::pushMatrices();
     
     gl::color( mClip->getColor() );
     
     for (int k=0; k < mBlobs.size(); k++)	
     {
     glBegin(GL_LINE_LOOP);
     for(int i=0; i < mBlobs[k].mPoints.size(); i++)
     gl::vertex( mBlobs[k].getPoint3f(i, height) );
     glEnd();
     }
     gl::popMatrices();
     
     gl::color( gColorWhite );
     */
}
