/*
 *  QLiveGuiTrackControl.h
 *
 *  Created by Andrea Cuius on 08/08/2011.
 *  Nocte Copyright 2011 . All rights reserved.
 *
 *  www.nocte.co.uk
 *
 */

#ifndef QLIVEGUI_TRACK_CONTROL
#define QLIVEGUI_TRACK_CONTROL

#pragma once

#include "QLiveObject.h"

#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/LabelClickable.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/HorizontalSlider.h"
#include "Gwen/Controls/RadioButtonController.h"


class QLiveGuiTrackControl : public Gwen::Controls::Base {

public:
    
	QLiveGuiTrackControl( nocte::QLiveTrackRef track, ci::Vec2f size, Gwen::Controls::Base *parent )
    : Gwen::Controls::Base(parent)
    {
        SetSize( size.x, size.y );
        SetMargin( Gwen::Margin( 15, 15 , 0, 0 ) ); // left, top, right, bottom
        Dock( Gwen::Pos::Left );
        
        std::vector<nocte::QLiveClipRef>    clips = track->getClips();
        nocte::QLiveClipRef                 clip;
        
        // Label
        Gwen::Controls::Label *label = new Gwen::Controls::Label( this );
        label->SetText( track->getName() );
        label->SizeToContents();
        label->Dock( Gwen::Pos::Top );
        label->SetTextColorOverride( cigwen::toGwen( track->getColor() ) );
        
        // Brightness
        Gwen::Controls::HorizontalSlider *brightness = new Gwen::Controls::HorizontalSlider( this );
        brightness->SetSize( size.x, 20 );
        brightness->Dock( Gwen::Pos::Top );
        brightness->onValueChanged.Add( this, &QLiveGuiTrackControl::onBrightnessChange );

        // Clips
        Gwen::Controls::RadioButtonController* rc = new Gwen::Controls::RadioButtonController( this );

        for( auto i=0; i < clips.size(); i++ )
        {
            clip = clips[i];
            rc->AddOption( clip->getName() );
        }
        
        rc->SetSize( size.x, size.y - 40 );
        rc->Dock( Gwen::Pos::Top );
        
        rc->onSelectionChange.Add( this, &QLiveGuiTrackControl::onClipChange );

    }
    
	virtual ~QLiveGuiTrackControl() {}

    
private:
    
    void onBrightnessChange( Gwen::Controls::Base* pControl )
    {
//        Gwen::Controls::HorizontalSlider* slider = ( Gwen::Controls::HorizontalSlider* ) pControl;
//        Gwen::Controls::LabeledRadioButton* pSelected = rc->GetSelected();
//        UnitPrint( Utility::Format( L"RadioButton changed (using 'OnChange' event)\n Chosen Item: '%ls'", pSelected->GetLabel()->GetText().GetUnicode().c_str() ) );
    }

    
    void onClipChange( Gwen::Controls::Base* pControl )
    {
//        Gwen::Controls::RadioButtonController* rc = ( Gwen::Controls::RadioButtonController* ) pControl;
//        Gwen::Controls::LabeledRadioButton* pSelected = rc->GetSelected();
//        UnitPrint( Utility::Format( L"RadioButton changed (using 'OnChange' event)\n Chosen Item: '%ls'", pSelected->GetLabel()->GetText().GetUnicode().c_str() ) );
    }

};


#endif



