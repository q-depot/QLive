#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"

#include "QLive.h"
#include "BasicModule.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace nocte;


class BasicSampleApp : public AppBasic {
  public:
	void setup();
	void prepareSettings( Settings *settings );
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
    
	QLive                   *mLive;
	
	vector<QLiveModule*>	mModules;
    
};

void BasicSampleApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize(1200, 800);
	settings->setFrameRate(60.0f);
}

void BasicSampleApp::setup()
{
	mLive = new QLive();
    
    mModules.push_back( BasicModule( mLive ) );
}

void BasicSampleApp::mouseDown( MouseEvent event )
{
}

void BasicSampleApp::update()
{
    float * values;
    
    for( size_t k=0; k < mModules.size(); k++ )
        mModules->update( values );
}

void BasicSampleApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
    
    gl::enableAlphaBlending();
    
	mLive->renderDebug();

	mLive->renderAnalyzer();
    
    gl::drawString( toString( getAverageFps() ), Vec2f(15,15) );
    
    for( size_t k=0; k < mModules.size(); k++ )
        mModules->render();
}


CINDER_APP_BASIC( BasicSampleApp, RendererGl )
