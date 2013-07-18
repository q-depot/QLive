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
        
        static QLiveGUIRef create( QLiveRef live )
        {
            return QLiveGUIRef( new QLiveGUI( live ) );
        }

        ~QLiveGUI() {}
        
        void render()
        {
            if ( mCanvas->Visible() )
                mCanvas->RenderCanvas();
        }
        
        void init();
        
        void toggle()
        {
            mCanvas->SetHidden( mCanvas->Visible() );
        }
        
        void toggleParams();
        
    private:
        
        QLiveGUI( QLiveRef live ) : mLive(live)
        {
            init();
        }

        
    private:
        
        QLiveRef                    mLive;
        
        cigwen::GwenRendererGl                  *mRenderer;
        cigwen::GwenInputRef                    mGwenInput;
        Gwen::Controls::Canvas                  *mCanvas;
        std::vector<QLiveGuiTrackControlRef>    mControls;
    };
    
}

#endif