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

#include "ISCANeditor.h"
#include "../ISCAN.h"
#include <stdio.h>

ISCANeditor::ISCANeditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
	desiredWidth = 220;

    urlLabel = new Label("Device:", "Device:");
    urlLabel->setBounds(20,30,60,25);
    addAndMakeVisible(urlLabel);

	deviceList = new ComboBox("SerialDevices");
	deviceList->setEditableText(false);
	deviceList->setJustificationType(Justification::centredLeft);
	deviceList->addListener(this);
	deviceList->setBounds(80,30,130,20);
	addAndMakeVisible(deviceList);
	refreshDevices();

    setEnabledState(false);

}


void ISCANeditor::refreshDevices()
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();
	StringArray devices = processor->getDeviceNames();
	deviceList->clear();
	deviceList->addItemList(devices,1);
}

void ISCANeditor::buttonEvent(Button* button)
{
	ISCANnode *processor  = (ISCANnode*) getProcessor();

}

void ISCANeditor::comboBoxChanged(ComboBox* comboBox)
{
	int selectedDevice = comboBox->getSelectedId();
	if (selectedDevice > 0)
	{
		ISCANnode *processor  = (ISCANnode*) getProcessor();
		processor->connect(selectedDevice);
	
	}
}

ISCANeditor::~ISCANeditor()
{

}


