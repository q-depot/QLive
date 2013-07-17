/*
 *  QLiveGUI.cpp
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 *  Requires Gwen : https://github.com/garrynewman/GWEN
 *
 */

#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"

#include "QLiveGui.h"
#include "QLiveGuiTrackControl.h"

//#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace nocte;


void QLiveGUI::init()
{
    // TODO: find a better way..
//	fs::path rootPath = getAppPath().parent_path().parent_path().parent_path().parent_path().parent_path().parent_path();
//	addAssetDirectory( rootPath / "assets" );
    
	mRenderer = new cigwen::GwenRendererGl();
	mRenderer->Init();
    
	Gwen::Skin::TexturedBase* skin = new Gwen::Skin::TexturedBase( mRenderer );
	skin->Init( "DefaultSkinQLiveGui.png" );
//    skin->m_colBGDark = Gwen::Color( 15, 15, 15 );
    
	mCanvas = new Gwen::Controls::Canvas( skin );
	mCanvas->SetSize( getWindowWidth(), getWindowHeight() );
	mGwenInput = cigwen::GwenInput::create( mCanvas );
    
    std::vector<QLiveTrackRef> tracks = mLive->getTracks();
    
    if ( tracks.empty() )
        return;
    
    int margin  = 15;
    
    int n = 0;
    for( auto k=0; k < tracks.size(); k++ )
        if ( !boost::starts_with( tracks[k]->getName(), "_") )          // IGNORE all the params in the tracks that starts with "_"
            n++;
    
    QLiveGuiTrackControl    *trackControl;
    Vec2f                   size;
    
    size.x              = ( getWindowWidth() - ( n + 1 ) * margin ) / n;
    size.y              = 150;
    
    for( auto k=0; k < tracks.size(); k++ )
    {
        if ( boost::starts_with( tracks[k]->getName(), "_") )          // IGNORE all the params in the tracks that starts with "_"
            continue;
        
        trackControl = new QLiveGuiTrackControl( mLive, tracks[k], size, mCanvas );
    }
    
}








/*
 void guiEvent(ciUIEvent *event)
 {
 std::string name = event->widget->getName();
 std::string meta = event->widget->getMeta();
 
 if(name == "Master")
 {
 int trackIdx = boost::lexical_cast<int>( event->widget->getMeta() );
 ciUISlider *slider = (ciUISlider *) event->widget;
 mLive->setTrackVolume( trackIdx, slider->getScaledValue() );
 }
 
 else if ( boost::find_first( meta, "clip") )
 {
 ciUIToggle *toggle = (ciUIToggle *) event->widget;
 
 std::vector<std::string>    splitValues;
 std::string                 clipMeta;
 bool                        toggleVal   = toggle->getValue();
 
 boost::split( splitValues, meta, boost::is_any_of("_") );
 
 int trackIdx    = boost::lexical_cast<int>( splitValues[1] );
 int clipIdx     = boost::lexical_cast<int>( splitValues[2] );
 
 toggle->setValue( !toggle->getValue() );    // trigger back the toogle, QLive sets the value, this is to avoid flickering
 
 if ( toggleVal )
 mLive->playClip( trackIdx, clipIdx );               // play clip
 else
 mLive->stopClip( trackIdx, clipIdx );               // play clip
 }
 
 else if ( boost::find_first( meta, "param") )
 {
 // param_TRACKIDX_DEVICEIDX_PARAMIDX
 
 ciUISlider *slider = (ciUISlider *) event->widget;
 
 std::vector<std::string> splitValues;
 boost::split( splitValues, meta, boost::is_any_of("_") );
 
 int trackIdx    = boost::lexical_cast<int>( splitValues[1] );
 int deviceIdx   = boost::lexical_cast<int>( splitValues[2] );
 int paramIdx    = boost::lexical_cast<int>( splitValues[3] );
 
 mLive->setParam( trackIdx, deviceIdx, paramIdx, slider->getScaledValue() );
 }
 
 }
 */