/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "SpikeDetectorEditor.h"
#include "SpikeDisplayEditor.h"
#include "../Visualization/SpikeDetectCanvas.h"
#include "../SpikeDetector.h"
#include "ChannelSelector.h"
#include "../../UI/EditorViewport.h"
#include "../AdvancerNode.h"
#include <stdio.h>



SpikeDetectorEditor::SpikeDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, 300, useDefaultParameterEditors), isPlural(true),spikeDetectorCanvas(nullptr)

{
	tabText = "Spike Detector";
	

    MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font(typeface);

    desiredWidth = 300;

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

	 advancerList = new ComboBox("Advancers");
    advancerList->addListener(this);
    advancerList->setBounds(10,95,130,20);
    addAndMakeVisible(advancerList);

    depthOffsetLabel = new Label("Depth Offset","Depth Offset");
	depthOffsetLabel->setFont(Font("Default", 10, Font::plain));
    depthOffsetLabel->setEditable(false);
    depthOffsetLabel->setBounds(125,115,80,20);
	depthOffsetLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(depthOffsetLabel);

	advancerLabel = new Label("Depth Offset","ADVANCER:");
	advancerLabel->setFont(Font("Default", 10, Font::plain));
    advancerLabel->setEditable(false);
    advancerLabel->setBounds(10,80,80,20);
	advancerLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(advancerLabel);
 	
    depthOffsetEdit = new Label("Depth Offset","0.0");
	depthOffsetEdit->setFont(Font("Default", 10, Font::plain));
    depthOffsetEdit->setEditable(true);
    depthOffsetEdit->setBounds(145,95,40,20);
	depthOffsetEdit->addListener(this);
	depthOffsetEdit->setColour(Label::textColourId, Colours::white);
	depthOffsetEdit->setColour(Label::backgroundColourId, Colours::grey);

    addAndMakeVisible(depthOffsetEdit);

    electrodeList = new ComboBox("Electrode List");
    electrodeList->setEditableText(false);
    electrodeList->setJustificationType(Justification::centredLeft);
    electrodeList->addListener(this);
    electrodeList->setBounds(65,30,130,20);
    addAndMakeVisible(electrodeList);

    numElectrodes = new Label("Number of Electrodes","1");
    numElectrodes->setEditable(true);
    numElectrodes->addListener(this);
    numElectrodes->setBounds(30,30,25,20);
    addAndMakeVisible(numElectrodes);

    upButton = new TriangleButton(1);
    upButton->addListener(this);
    upButton->setBounds(50,30,10,8);
    addAndMakeVisible(upButton);

    downButton = new TriangleButton(2);
    downButton->addListener(this);
    downButton->setBounds(50,40,10,8);
    addAndMakeVisible(downButton);

    plusButton = new UtilityButton("+", titleFont);
    plusButton->addListener(this);
    plusButton->setRadius(3.0f);
    plusButton->setBounds(15,27,14,14);
    addAndMakeVisible(plusButton);

    audioMonitorButton = new UtilityButton("MONITOR", Font("Default", 12, Font::plain));
    audioMonitorButton->addListener(this);
    audioMonitorButton->setRadius(3.0f);
    audioMonitorButton->setBounds(80,65,65,15);
    audioMonitorButton->setClickingTogglesState(true);
    addAndMakeVisible(audioMonitorButton);

	removeElectrodeButton = new UtilityButton("-",font);
    removeElectrodeButton->addListener(this);
    removeElectrodeButton->setBounds(15,45,14,14);
    addAndMakeVisible(removeElectrodeButton);
    
   
    configButton = new UtilityButton("CONFIG",Font("Default", 12, Font::plain));
    configButton->addListener(this);
    configButton->setBounds(10,65,60,15);
    addAndMakeVisible(configButton);

	thresholdSlider = new ThresholdSlider(font);
    thresholdSlider->setBounds(210,25,65,65);
    addAndMakeVisible(thresholdSlider);
    thresholdSlider->addListener(this);
    thresholdSlider->setActive(false);
    Array<double> v;
    thresholdSlider->setValues(v);

    thresholdLabel = new Label("Name","Threshold");
    font.setHeight(10);
    thresholdLabel->setFont(font);
    thresholdLabel->setBounds(208, 85, 95, 15);
    thresholdLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(thresholdLabel);

    // create a custom channel selector
    deleteAndZero(channelSelector);

    channelSelector = new ChannelSelector(true, font);
    addChildComponent(channelSelector);
    channelSelector->setVisible(false);

	channelSelector->activateButtons();
	channelSelector->setRadioStatus(true);
    channelSelector->paramButtonsToggledByDefault(false);
	updateAdvancerList();

    dacAssignmentLabel= new Label("DAC output","DAC output");
	dacAssignmentLabel->setFont(Font("Default", 10, Font::plain));
    dacAssignmentLabel->setEditable(false);
    dacAssignmentLabel->setBounds(210,115,80,20);
	dacAssignmentLabel->setColour(Label::textColourId, Colours::grey);
    addAndMakeVisible(dacAssignmentLabel);

	dacCombo = new ComboBox("DAC Assignment");
    dacCombo->addListener(this);
    dacCombo->setBounds(205,100,70,18);
	dacCombo->addItem("-",1);
	for (int k=0;k<8;k++)
	{
		dacCombo->addItem("DAC"+String(k+1),k+2);
	}
	dacCombo->setSelectedId(1);
    addAndMakeVisible(dacCombo);

}

Visualizer* SpikeDetectorEditor::createNewCanvas()
{

    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    spikeDetectorCanvas = new SpikeDetectCanvas(processor);
	return spikeDetectorCanvas;
}


SpikeDetectorEditor::~SpikeDetectorEditor()
{

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        removeChildComponent(electrodeButtons[i]);
    }

   // deleteAllChildren();
	 

}

void SpikeDetectorEditor::sliderEvent(Slider* slider)
{
    int electrodeNum = -1;

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeNum = i;
            break;
        }
    }

    if (electrodeNum > -1)
    {
        SpikeDetector* processor = (SpikeDetector*) getProcessor();
        processor->setChannelThreshold(electrodeList->getSelectedItemIndex(),
                                       electrodeNum,
                                       slider->getValue());

		//Array<int> dacChannels = processor->getDACassignments;
		int dacChannel = dacCombo->getSelectedId()-2;
		if (dacChannel >= 0)
		{
			// update dac threshold.
			processor->updateDACthreshold(dacChannel, slider->getValue());
		}

    }
	repaint();
	if (canvas!= nullptr)
		canvas->repaint();

}


void SpikeDetectorEditor::buttonEvent(Button* button)
{
	VisualizerEditor::buttonEvent(button);
	   SpikeDetector* processor = (SpikeDetector*) getProcessor();
	
    if (electrodeButtons.contains((cElectrodeButton*) button))
    {
	
        {
			for (int k=0;k<electrodeButtons.size();k++)
			{
				if (electrodeButtons[k] != button)
					electrodeButtons[k]->setToggleState(false,dontSendNotification);
			}
			if (electrodeButtons.size() == 1)
				electrodeButtons[0]->setToggleState(true,dontSendNotification);

            cElectrodeButton* eb = (cElectrodeButton*) button;
            int channelNum = eb->getChannelNum()-1;

            std::cout << "Channel number: " << channelNum << std::endl;
            Array<int> a;
            a.add(channelNum);
            channelSelector->setActiveChannels(a);

            SpikeDetector* processor = (SpikeDetector*) getProcessor();

            thresholdSlider->setActive(true);
            thresholdSlider->setValue(processor->getChannelThreshold(electrodeList->getSelectedItemIndex(),
                                                                     electrodeButtons.indexOf((cElectrodeButton*) button)));


			if (processor->getAutoDacAssignmentStatus())
			{
				processor->assignDACtoChannel(0, channelNum);
				processor->assignDACtoChannel(1, channelNum);
			}
			Array<int> dacAssignmentToChannels = processor->getDACassignments();
			// search for channel[0]. If found, set the combo box accordingly...
			dacCombo->setSelectedId(1,true);
			for (int i=0;i<dacAssignmentToChannels.size();i++)
			{
				if (dacAssignmentToChannels[i] == channelNum)
				{
					dacCombo->setSelectedId(i+2,true);
					break;
				}
			}

        }
    }


    int num = numElectrodes->getText().getIntValue();

    if (button == upButton)
    {
        numElectrodes->setText(String(++num), sendNotification);

        return;

    }
    else if (button == downButton)
    {

        if (num > 1)
            numElectrodes->setText(String(--num), sendNotification);

        return;

    } else if (button == configButton)
	{
		PopupMenu configMenu;
		PopupMenu waveSizeMenu;
		PopupMenu waveSizePreMenu;
		PopupMenu waveSizePostMenu;

		waveSizePreMenu.addItem(1,"8",true,processor->getNumPreSamples() == 8);
		waveSizePreMenu.addItem(2,"16",true,processor->getNumPreSamples() == 16);
		waveSizePostMenu.addItem(3,"32",true,processor->getNumPostSamples() == 32);
		waveSizePostMenu.addItem(4,"64",true,processor->getNumPostSamples() == 64);

		waveSizeMenu.addSubMenu("Pre samples",waveSizePreMenu);
		waveSizeMenu.addSubMenu("Post samples",waveSizePostMenu);
		configMenu.addSubMenu("Waveform size",waveSizeMenu,true);
		configMenu.addItem(5,"Current Channel => Audio",true,processor->getAutoDacAssignmentStatus());
		const int result = configMenu.show();
		switch (result)
		{
		case 1:
			processor->setNumPreSamples(8);
			break;
		case 2:
			processor->setNumPreSamples(16);
			break;
		case 3:
			processor->setNumPostSamples(32);
			break;
		case 4:
			processor->setNumPostSamples(64);
			break;
		case 5:
			processor->seteAutoDacAssignment(!processor->getAutoDacAssignmentStatus());
			refreshElectrodeList();
		}

	}
    else if (button == plusButton)
    {
        // std::cout << "Plus button pressed!" << std::endl;
		//updateAdvancerList();
		PopupMenu probeMenu;
		probeMenu.addItem(1,"Single Electrode");
		probeMenu.addItem(2,"Stereotrode");
		probeMenu.addItem(3,"Tetrode");
		PopupMenu depthprobeMenu;
		depthprobeMenu.addItem(4,"8 ch, 125um");
		depthprobeMenu.addItem(5,"16 ch, 125um");
		depthprobeMenu.addItem(6,"24 ch, 125um");
		depthprobeMenu.addItem(7,"32 ch, 50um");
		probeMenu.addSubMenu("Depth probe", depthprobeMenu,true);

		const int result = probeMenu.show();
        int nChansPerElectrode;
		int nElectrodes;
		double interelectrodeDistance=0;
		double firstElectrodeOffset ;
		int numProbes = numElectrodes->getText().getIntValue();
		String ProbeType;

        switch (result)
        {
			case 0:
				return;
            case 1:
				ProbeType = "Single Electrode";
                nChansPerElectrode = 1;
				nElectrodes = 1;
				firstElectrodeOffset=0;
                break;
            case 2:
				ProbeType = "Stereotrode";
                nChansPerElectrode = 2;
				nElectrodes = 1;
				firstElectrodeOffset = 0;
                break;
            case 3:
				ProbeType = "Tetrode";
                nChansPerElectrode = 4;
				nElectrodes = 1;
				firstElectrodeOffset = 0;
                break;
            case 4:
				ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
				nElectrodes = 8;
				interelectrodeDistance = 0.125;
				firstElectrodeOffset= -0.5;
                break;
            case 5:
				ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
				nElectrodes = 16;
				interelectrodeDistance = 0.125;
				firstElectrodeOffset= -0.5;
                break;
            case 6:
				ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
				nElectrodes = 24;
				interelectrodeDistance = 0.125;
				firstElectrodeOffset= -0.5;
                break;
            case 7:
				ProbeType = "Depth Probe";
                nChansPerElectrode = 1;
				nElectrodes = 32;
				interelectrodeDistance = 0.050;
				firstElectrodeOffset= -0.5;
                break;
        }

	 	processor->addProbes(ProbeType,numProbes, nElectrodes,nChansPerElectrode, firstElectrodeOffset,interelectrodeDistance);
		refreshElectrodeList();
		
        return;

    }
    else if (button == removeElectrodeButton)   // DELETE
    {

        removeElectrode(electrodeList->getSelectedItemIndex());

        //getEditorViewport()->makeEditorVisible(this, true, true);

        return;
    } else if (button == audioMonitorButton)
    {

       channelSelector->clearAudio();

       SpikeDetector* processor = (SpikeDetector*) getProcessor();

        Array<Electrode*> electrodes = processor->getElectrodes();

        for (int i = 0; i < electrodes.size(); i++)
        {
            Electrode* e = electrodes[i];
            e->isMonitored = false;
        }

        Electrode* e = processor->getActiveElectrode();
        e->isMonitored = audioMonitorButton->getToggleState();

        for (int i = 0; i < e->numChannels; i++)
        {
            int channelNum = e->channels[i];
            channelSelector->setAudioStatus(channelNum, audioMonitorButton->getToggleState());

        }

    }



}

void SpikeDetectorEditor::setThresholdValue(int channel, double threshold)
{
	thresholdSlider->setActive(true);
	thresholdSlider->setValue(threshold);
	repaint();
}

void SpikeDetectorEditor::channelChanged(int chan)
{
    //std::cout << "New channel: " << chan << std::endl;
	if (chan <=0)
		return;

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getToggleState())
        {
            electrodeButtons[i]->setChannelNum(chan);
            electrodeButtons[i]->repaint();
			Array<int> a;
			a.add(chan-1);
			channelSelector->setActiveChannels(a);
            SpikeDetector* processor = (SpikeDetector*) getProcessor();
            processor->setChannel(electrodeList->getSelectedItemIndex(),
                                  i,
                                  chan-1);

			// if DAC is selected, update the mapping.
			int dacchannel = dacCombo->getSelectedId()-2;
			if (dacchannel >=0)
			{	
				processor->assignDACtoChannel(dacchannel, chan-1);
			}
			if (processor->getAutoDacAssignmentStatus())
			{
				processor->assignDACtoChannel(0,chan-1);
				processor->assignDACtoChannel(1,chan-1);
				break;
			}

        }
    }

}

int SpikeDetectorEditor::getSelectedElectrode()
{
	return electrodeList->getSelectedId();
}

void SpikeDetectorEditor::setSelectedElectrode(int i)
{
	electrodeList->setSelectedId(i);
}

void SpikeDetectorEditor::refreshElectrodeList(int selected)
{
	electrodeList->clear();

	SpikeDetector* processor = (SpikeDetector*) getProcessor();

	StringArray electrodeNames = processor->getElectrodeNames();

	for (int i = 0; i < electrodeNames.size(); i++)
	{
		electrodeList->addItem(electrodeNames[i], electrodeList->getNumItems()+1);
	}

	if (electrodeList->getNumItems() > 0)
	{
		if (selected == 0)
			selected = electrodeList->getNumItems();

		electrodeList->setSelectedId(selected);
		//        electrodeList->setText(electrodeList->getItemText(electrodeList->getNumItems()-1));
		lastId = electrodeList->getNumItems();
		electrodeList->setEditableText(true);

		drawElectrodeButtons(selected-1);
		Electrode *e = processor->getElectrode( selected - 1);

		int advancerIndex = 0;
		for (int k=0;k<advancerIDs.size();k++)
		{
			if (advancerIDs[k] == e->advancerID)
			{
				advancerIndex = 1+k;
				break;
			}
		}

		advancerList->setSelectedId(advancerIndex);
		depthOffsetEdit->setText(String(e->depthOffsetMM,4),dontSendNotification);

		if (processor->getAutoDacAssignmentStatus())
		{
			processor->assignDACtoChannel(0, e->channels[0]);
			processor->assignDACtoChannel(1, e->channels[0]);
		}
		Array<int> dacAssignmentToChannels = processor->getDACassignments();
		// search for channel[0]. If found, set the combo box accordingly...
		dacCombo->setSelectedId(1,true);
		for (int i=0;i<dacAssignmentToChannels.size();i++)
		{
			if (dacAssignmentToChannels[i] == e->channels[0])
			{
				dacCombo->setSelectedId(i+2,true);
				processor->updateDACthreshold(i+2, e->thresholds[0]);
				break;
			}
		}




	}
	if (spikeDetectorCanvas != nullptr)
		spikeDetectorCanvas->update();
}


void SpikeDetectorEditor::removeElectrode(int index)
{
    std::cout << "Deleting electrode number " << index << std::endl;
    SpikeDetector* processor = (SpikeDetector*) getProcessor();
    processor->removeElectrode(index);
    refreshElectrodeList();

    int newIndex = jmin(index, electrodeList->getNumItems()-1);
    newIndex = jmax(newIndex, 0);

    electrodeList->setSelectedId(newIndex, true);
    electrodeList->setText(electrodeList->getItemText(newIndex));

    if (electrodeList->getNumItems() == 0)
    {
        electrodeButtons.clear();
        electrodeList->setEditableText(false);
    }
}

void SpikeDetectorEditor::labelTextChanged(Label* label)
{
	if (label == depthOffsetEdit)
	{
		// update electrode depth offset.
		Value v = depthOffsetEdit->getTextValue();
		double offset = v.getValue();

		int electrodeIndex = electrodeList->getSelectedId()-1;
		SpikeDetector* processor = (SpikeDetector*) getProcessor();
		if (electrodeIndex >= 0)
			processor->setElectrodeAdvancerOffset(electrodeIndex, offset);

	if (spikeDetectorCanvas != nullptr)
		spikeDetectorCanvas->update();

	}
}


void SpikeDetectorEditor::comboBoxChanged(ComboBox* comboBox)
{
      SpikeDetector* processor = (SpikeDetector*) getProcessor();

    if (comboBox == dacCombo)
	{
		int selection = dacCombo->getSelectedId();
		// modify the dac channel assignment...
		if (selection > 1)
		{
			int selectedSubChannel = -1;
			for (int i = 0; i < electrodeButtons.size(); i++)
			{
				if (electrodeButtons[i]->getToggleState())
				{
					selectedSubChannel = i;
					break;
				}
			}
			Electrode* e = processor->getActiveElectrode();
			if (e != nullptr)
			{
				int dacchannel = selection-2;
				processor->assignDACtoChannel(dacchannel, e->channels[selectedSubChannel]);
			}
		}

	} else if (comboBox == electrodeList)
    {
	     int ID = comboBox->getSelectedId();

        if (ID == 0)
        {
      
            processor->setElectrodeName(lastId, comboBox->getText());
            refreshElectrodeList();

        }
		else
        {
			SpikeDetector* processor = (SpikeDetector*) getProcessor();
            lastId = ID;
			Electrode * e= processor->setCurrentElectrodeIndex(ID-1);
            drawElectrodeButtons(ID-1);
			int advancerIndex = 0;

            audioMonitorButton->setToggleState(e->isMonitored, false);

			for (int k=0;k<advancerIDs.size();k++)
			{
				if (advancerIDs[k] == e->advancerID)
				{
					advancerIndex = 1+k;
					break;
				}
			}
			advancerList->setSelectedId(advancerIndex,false);
			depthOffsetEdit->setText(String(e->depthOffsetMM,4),dontSendNotification);

			if (processor->getAutoDacAssignmentStatus())
			{
				processor->assignDACtoChannel(0, e->channels[0]);
				processor->assignDACtoChannel(1, e->channels[0]);
			}
			Array<int> dacAssignmentToChannels = processor->getDACassignments();
			// search for channel[0]. If found, set the combo box accordingly...
			dacCombo->setSelectedId(1,true);
			for (int i=0;i<dacAssignmentToChannels.size();i++)
			{
				if (dacAssignmentToChannels[i] == e->channels[0])
				{
					dacCombo->setSelectedId(i+2,true);
					break;
				}
			}

        }



	} else if ( comboBox == advancerList)
	{
		// attach advancer to electrode.
		int electrodeIndex = electrodeList->getSelectedId()-1;
		SpikeDetector* processor = (SpikeDetector*) getProcessor();
		int selectedAdvancer = advancerList->getSelectedId() ;
		if (electrodeIndex >= 0 && selectedAdvancer > 0)
			processor->setElectrodeAdvancer(electrodeIndex,advancerIDs[advancerList->getSelectedId()-1]);
		else
			advancerList->setSelectedId(0,dontSendNotification);
	}
	
}

void SpikeDetectorEditor::checkSettings()
{
    electrodeList->setSelectedItemIndex(0);
}

void SpikeDetectorEditor::drawElectrodeButtons(int ID)
{

    SpikeDetector* processor = (SpikeDetector*) getProcessor();

    electrodeButtons.clear();

    int width = 20;
    int height = 15;

    int numChannels = processor->getNumChannels(ID);
    int row = 0;
    int column = 0;

    Array<int> activeChannels;
    Array<double> thresholds;

    for (int i = 0; i < numChannels; i++)
    {
        cElectrodeButton* button = new cElectrodeButton(processor->getChannel(ID,i)+1);
        electrodeButtons.add(button);

        

		if (i == 0) {
			activeChannels.add(processor->getChannel(ID,i));
			thresholds.add(processor->getChannelThreshold(ID,i));
		}

		button->setToggleState(i == 0, false);

		button->setBounds(155+(column++)*width, 60+row*height, width, 15);

		addAndMakeVisible(button);
        button->addListener(this);

        if (column % 2 == 0)
        {
            column = 0;
            row++;
        }

    }

    channelSelector->setActiveChannels(activeChannels);
	
    thresholdSlider->setValues(thresholds);
	thresholdSlider->setActive(true);
	thresholdSlider->setEnabled(true);
	thresholdSlider->setValue(processor->getChannelThreshold(ID,0),dontSendNotification);
	repaint();
	if (spikeDetectorCanvas != nullptr)
		spikeDetectorCanvas->update();
}




void cElectrodeButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::orange);
    else
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::white);

    g.fillRect(0,0,getWidth(),getHeight());

    // g.setFont(buttonFont);
    g.setColour(Colours::black);

    g.drawRect(0,0,getWidth(),getHeight(),1.0);

    if (chan >= 0)
        g.drawText(String(chan),0,0,getWidth(),getHeight(),Justification::centred,true);
}


void ElectrodeEditorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::darkgrey);
    else
        g.setColour(Colours::lightgrey);

    g.setFont(font);

    g.drawText(name,0,0,getWidth(),getHeight(),Justification::left,true);
}


ThresholdSlider::ThresholdSlider(Font f) : Slider("name"), font(f)
{

    setSliderStyle(Slider::Rotary);
    setRange(-400,400.0f,10.0f);
    setValue(-20.0f);
    setTextBoxStyle(Slider::NoTextBox, false, 40, 20);

}

void ThresholdSlider::paint(Graphics& g)
{

    ColourGradient grad = ColourGradient(Colour(40, 40, 40), 0.0f, 0.0f,
                                         Colour(80, 80, 80), 0.0, 40.0f, false);

    Path p;
    p.addPieSegment(3, 3, getWidth()-6, getHeight()-6, 5*double_Pi/4-0.2, 5*double_Pi/4+3*double_Pi/2+0.2, 0.5);

    g.setGradientFill(grad);
    g.fillPath(p);

    String valueString;

    if (isActive)
    {
        p = makeRotaryPath(getMinimum(), getMaximum(), getValue());
        g.setColour(Colour(240,179,12));
        g.fillPath(p);

        valueString = String((int) getValue());
    }
    else
    {

        valueString = "";

        for (int i = 0; i < valueArray.size(); i++)
        {
            p = makeRotaryPath(getMinimum(), getMaximum(), valueArray[i]);
            g.setColour(Colours::lightgrey.withAlpha(0.4f));
            g.fillPath(p);
            valueString = String((int) valueArray.getLast());
        }

    }

    font.setHeight(9.0);
    g.setFont(font);
    int stringWidth = font.getStringWidth(valueString);

    g.setFont(font);

    g.setColour(Colours::darkgrey);
    g.drawSingleLineText(valueString, getWidth()/2 - stringWidth/2, getHeight()/2+3);

}

Path ThresholdSlider::makeRotaryPath(double min, double max, double val)
{

    Path p;

    double start;
    double range;
	if (val > 0)
	{
		start = 0;
		range = (val)/(1.3*max )*double_Pi ;
	}
	if (val < 0) {
		start = -(val)/(1.3*min)*double_Pi ;
		range = 0;
	}
    p.addPieSegment(6,6, getWidth()-12, getHeight()-12, start, range, 0.65);

    return p;

}

void ThresholdSlider::setActive(bool t)
{
    isActive = t;
    repaint();
}

void ThresholdSlider::setValues(Array<double> v)
{
    valueArray = v;
}



void SpikeDetectorEditor::updateAdvancerList()
{
	
	ProcessorGraph *g = getProcessor()->getProcessorGraph();
	Array<GenericProcessor*> p = g->getListOfProcessors();
	for (int k=0;k<p.size();k++)
	{
		if (p[k]->getName() == "Advancers")
		{
			AdvancerNode *node = (AdvancerNode *)p[k];
			if (node != nullptr)
			{
				advancerNames = node->getAdvancerNames();
				advancerIDs =  node->getAdvancerIDs();

				advancerList->clear(dontSendNotification);
				for (int i=0;i<advancerNames.size();i++)
				{
					advancerList->addItem(advancerNames[i],1+i);
				}
			}
		}
	}


        int selectedElectrode = electrodeList->getSelectedId();
		if (selectedElectrode > 0) {
			SpikeDetector* processor = (SpikeDetector*) getProcessor();
			Electrode *e = processor->getElectrode( selectedElectrode-1);
			int advancerIndex = 0;
			for (int k=0;k<advancerIDs.size();k++)
			{
				if (advancerIDs[k] == e->advancerID)
				{
						advancerIndex = 1+k;
					break;
				}
			}
			advancerList->setSelectedId(advancerIndex);
			}
	repaint();
}
