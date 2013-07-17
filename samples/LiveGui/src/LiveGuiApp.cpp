
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "QLive.h"
#include "QLiveGUI.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace nocte;


class LiveGuiApp : public AppNative {
public:
    void shutdown();
    void setup();
    void prepareSettings( Settings *settings );
    void mouseDown( MouseEvent event );
    void update();
    void draw();
    
    QLiveRef                mLive;
	QLiveGUIRef             mLiveGUI;
    
};


void LiveGuiApp::shutdown()
{
    mLiveGUI.reset();
    mLive.reset();
}


void LiveGuiApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1400, 900 );
    settings->setFrameRate(60.0f);
}


void LiveGuiApp::setup()
{
    mLive       = QLive::create();
    mLiveGUI    = QLiveGUI::create( mLive );
    //    mModules.push_back( BasicModule( mLive ) );
}


void LiveGuiApp::mouseDown( MouseEvent event )
{
}


void LiveGuiApp::update()
{
    //    float * values;
    //
    //    for( size_t k=0; k < mModules.size(); k++ )
    //        mModules->update( values );
    
    mLiveGUI->update();
}


void LiveGuiApp::draw()
{
    // clear out the window with black
    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );
    
    gl::enableAlphaBlending();
    
//    mLive->renderDebug();
    
//    mLive->renderAnalyzer();
    
    gl::drawString( toString( getAverageFps() ), getWindowSize() - Vec2f( 100, 25 ) );
    
    mLiveGUI->render();
    
    //    for( size_t k=0; k < mModules.size(); k++ )
    //        mModules->render();
}



CINDER_APP_NATIVE( LiveGuiApp, RendererGl )

















