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


namespace nocte {
    
    class QLiveGUI;
    typedef std::shared_ptr<QLiveGUI> QLiveGUIRef;
    
    class QLiveGuiTrackControl;
    typedef std::shared_ptr<QLiveGuiTrackControl> QLiveGuiTrackControlRef;
    
    
    class QLiveGUI {
        
    public:
        
        static QLiveGUIRef create( QLiveRef live, Gwen::Controls::Canvas *canvas )
        {
            return QLiveGUIRef( new QLiveGUI( live, canvas ) );
        }

        ~QLiveGUI() {}
        
        void init();
        
        void toggle();
        
        void toggleParams();
        
    private:
        
        QLiveGUI( QLiveRef live, Gwen::Controls::Canvas *canvas ) : mLive(live), mCanvas(canvas)
        {
            init();
        }

        
    private:
        
        QLiveRef                    mLive;
        Gwen::Controls::Canvas      *mCanvas;
        
        std::vector<QLiveGuiTrackControlRef>    mControls;
    };
    
}

#endif