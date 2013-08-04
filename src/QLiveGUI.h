/*
 *  QLiveGUI.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 *  Requires Gwen : https://github.com/garrynewman/GWEN
 *
 */


#ifndef QLIVE_GUI
#define QLIVE_GUI

#pragma once

#include "cinder/app/AppBasic.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "QLive.h"

#include "cigwen/GwenRendererGl.h"
#include "cigwen/GwenInput.h"
#include "cigwen/CinderGwen.h"


class QLiveGUI;
typedef std::shared_ptr<QLiveGUI> QLiveGUIRef;

class QLiveGuiTrackControl;
typedef std::shared_ptr<QLiveGuiTrackControl> QLiveGuiTrackControlRef;


class QLiveGUI {
    
public:
    
    static QLiveGUIRef create( QLiveRef live, Gwen::Controls::Canvas *canvas, ci::Rectf rect )
    {
        return QLiveGUIRef( new QLiveGUI( live, canvas, rect ) );
    }

    ~QLiveGUI() {}
    
    void init( ci::Rectf rect );
    
    void toggle();
    
    void toggleParams();
    
private:
    
    QLiveGUI( QLiveRef live, Gwen::Controls::Canvas *canvas, ci::Rectf rect ) : mLive(live), mCanvas(canvas)
    {
        init(rect);
    }

    
private:
    
    QLiveRef                    mLive;
    Gwen::Controls::Canvas      *mCanvas;
    
    std::vector<QLiveGuiTrackControlRef>    mControls;
};


#endif