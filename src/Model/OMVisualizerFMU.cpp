/*
 * Copyright (C) 2016, Volker Waurich
 *
 * This file is part of OMVis.
 *
 * OMVis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OMVis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OMVis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Util/Logger.hpp"
#include "Model/OMVisualizerFMU.hpp"
#include "View/OMVisScene.hpp"
#include "Control/OMVisManager.hpp"
#include "Control/JoystickDevice.hpp"

#include <SDL.h>

#include <iostream>

namespace Model
{

	OMVisualizerFMU::OMVisualizerFMU(const std::string modelName, const std::string modelPath)
		: OMVisualizerAbstract(modelName, modelPath),
		_fmu(),
		_simSettings(new SimSettings()),
		_inputData(),
		_joysticks()
    {
	    LOGGER_WRITE(std::string("Initialize joysticks"), Util::LC_LOADER, Util::LL_INFO);
		initJoySticks();
    }

    void OMVisualizerFMU::initJoySticks()
    {
        //Initialize SDL
        if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
            LOGGER_WRITE(std::string("SDL could not be initialized."), Util::LC_LOADER, Util::LL_ERROR);

        //Check for joysticks
        _numJoysticks = SDL_NumJoysticks();
        if (SDL_NumJoysticks() < 1)
            LOGGER_WRITE(std::string("No joysticks connected!"), Util::LC_LOADER, Util::LL_WARNING);
        else
        {
            LOGGER_WRITE(std::string("Found ") + std::to_string(SDL_NumJoysticks()) + std::string(" joystick(s)"), Util::LC_LOADER, Util::LL_INFO);
            //Load joystick
            std::cout << "START LOADING JOYSTICKS!!!!!!!!!" << _numJoysticks << std::endl;

            for (size_t i = 0; i < _numJoysticks; ++i)
            {
                std::cout << "LOAD JOYSTICKS!!!!!!!!!" << i << std::endl;

                Control::JoystickDevice* newJoyStick = new Control::JoystickDevice(i);
                _joysticks.push_back(newJoyStick);

                if (newJoyStick == nullptr)
                    LOGGER_WRITE(std::string("Unable to open joystick! SDL Error: ") + SDL_GetError(), Util::LC_LOADER, Util::LL_INFO);
            }
        }
    }

    void OMVisualizerFMU::initData()
    {
        OMVisualizerAbstract::initData();
        loadFMU(_baseData->_modelName, _baseData->_dirName);
        _simSettings->setTend(_omvManager->_endTime);
        _simSettings->setHdef(0.001);
    }

    void OMVisualizerFMU::loadFMU(const std::string model, const std::string dir)
    {
        //setup fmu-simulation stuff
        //_simSettings = new SimSettings;
        std::string fmuFileName = dir + model + ".fmu";

        //load and initialize fmu
        _fmu.load(dir.c_str(), fmuFileName.c_str());
        LOGGER_WRITE(std::string("OMVisualizerFMU::loadFMU: FMU was successfully loaded."), Util::LC_LOADER, Util::LL_DEBUG);

        _fmu.initialize(_simSettings);
        LOGGER_WRITE(std::string("OMVisualizerFMU::loadFMU: FMU was successfully initialized."), Util::LC_LOADER, Util::LL_DEBUG);

        _inputData.initializeInputs(_fmu._fmu);
        _inputData.printValues();
        //assign interactive inputs
        //for (unsigned int i = 0; i < inputs.n_inputs; i++){
        //string key = "";
        //std::cout<<"assign input "<<i<<" :"<<std::endl;
        //getline(cin,key);
        //int keyInt = getchar();
        //std::cout<<"the key is "<<keyInt<<" !"<<std::endl;
        //}
    }

    void OMVisualizerFMU::simulate(Control::OMVisManager& omvm)
    {
        while (omvm._simTime < omvm._realTime + omvm._hVisual && omvm._simTime < omvm._endTime)
        {
            //<<"simulate "<<omvManager.simTime<<endl;
            //omv.fmuData.inputs.printValues();
            omvm._simTime = simulateStep(omvm._simTime);
        }
    }

    double OMVisualizerFMU::simulateStep(const double time)
    {
        //tcur = settings.tstart;
        //hcur = settings.hdef;
        int zero_crossning_event = 0;
        _fmu._fmuData._fmiStatus = fmi1_import_set_time(_fmu._fmu, time);
        _fmu._fmuData._fmiStatus = fmi1_import_get_event_indicators(_fmu._fmu, _fmu._fmuData._eventIndicators, _fmu._fmuData._nEventIndicators);

        /* Check if an event indicator has triggered */
        for (size_t k = 0; k < _fmu._fmuData._nEventIndicators; ++k)
        {
            if (_fmu._fmuData._eventIndicators[k] * _fmu._fmuData._eventIndicatorsPrev[k] < 0)
            {
                zero_crossning_event = 1;
                LOGGER_WRITE(std::string("OMVisualizerFMU::loadFMU: Event occurred at ") + std::to_string(_fmu._fmuData._tcur), Util::LC_CTR, Util::LL_DEBUG);
                //std::cout << "AN EVENT at " << _fmuData._tcur << std::endl;
                break;
            }
        }

        /* Handle any events */
        if (_simSettings->getCallEventUpdate() || zero_crossning_event || (_fmu._fmuData._eventInfo.upcomingTimeEvent && _fmu._fmuData._tcur == _fmu._fmuData._eventInfo.nextEventTime))
        {
            std::cout << "HANDLE EVENT at " << _fmu._fmuData._tcur << std::endl;
            _fmu._fmuData._fmiStatus = fmi1_import_eventUpdate(_fmu._fmu, _simSettings->getIntermediateResults(), &_fmu._fmuData._eventInfo);
            _fmu._fmuData._fmiStatus = fmi1_import_get_continuous_states(_fmu._fmu, _fmu._fmuData._states, _fmu._fmuData._nStates);
            _fmu._fmuData._fmiStatus = fmi1_import_get_event_indicators(_fmu._fmu, _fmu._fmuData._eventIndicators, _fmu._fmuData._nEventIndicators);
            _fmu._fmuData._fmiStatus = fmi1_import_get_event_indicators(_fmu._fmu, _fmu._fmuData._eventIndicatorsPrev, _fmu._fmuData._nEventIndicators);
        }
        /* Updated next time step */
        if (_fmu._fmuData._eventInfo.upcomingTimeEvent)
        {
            if (_fmu._fmuData._tcur + _simSettings->getHdef() < _fmu._fmuData._eventInfo.nextEventTime)
                _fmu._fmuData._hcur = _simSettings->getHdef();
            else
                _fmu._fmuData._hcur = _fmu._fmuData._eventInfo.nextEventTime - _fmu._fmuData._tcur;
        }
        else
            _fmu._fmuData._hcur = _simSettings->getHdef();

        /* increase with step size*/
        _fmu._fmuData._tcur += _fmu._fmuData._hcur;

        /* last step */
        if (_fmu._fmuData._tcur > _simSettings->getTend() - _fmu._fmuData._hcur / 1e16)
        {
            _fmu._fmuData._tcur -= _fmu._fmuData._hcur;
            _fmu._fmuData._hcur = _simSettings->getTend() - _fmu._fmuData._tcur;
            _fmu._fmuData._tcur = _simSettings->getTend();
        }

        //set inputs
		for (size_t i = 0; i < _numJoysticks; ++i)
		{
			_joysticks[i]->detectContinuousInputEvents(_inputData);
			_inputData.setInputsInFMU(_fmu._fmu);
			//std::cout << "JOY" << i << " XDir " <<_joysticks[i]->getXDir() <<" YDir "<< _joysticks[i]->getYDir() << std::endl;
		}

		//X2 MF: On my system, this line is needed in order to get the keyboard input working
        _inputData.setInputsInFMU(_fmu._fmu);

        /* Solve system */
        _fmu._fmuData._fmiStatus = fmi1_import_get_derivatives(_fmu._fmu, _fmu._fmuData._statesDer, _fmu._fmuData._nStates);

        //print out some values for debugging:
        //std::cout<<"DO EULER at "<<data.tcur<<std::endl;
        //fmi1_import_variable_t* var = fmi1_import_get_variable_by_name(_fmul._fmu, "prismatic.s");
        //const fmi1_value_reference_t vr  = fmi1_import_get_variable_vr(var);
        //double value = -1.0;
        //fmi1_import_get_real(_fmul._fmu, &vr, 1, &value);
        //std::cout<<"value "<<value<<std::endl;

        // integrate a step with euler
        for (size_t k = 0; k < _fmu._fmuData._nStates; ++k)
            _fmu._fmuData._states[k] = _fmu._fmuData._states[k] + _fmu._fmuData._hcur * _fmu._fmuData._statesDer[k];

        /* Set states */
        _fmu._fmuData._fmiStatus = fmi1_import_set_continuous_states(_fmu._fmu, _fmu._fmuData._states, _fmu._fmuData._nStates);
        /* Step is complete */
        _fmu._fmuData._fmiStatus = fmi1_import_completed_integrator_step(_fmu._fmu, &_simSettings->_callEventUpdate);

		//vw: since we are detecting changing inputs, we have to keep the values during the steps. do not reset it
        //X2 MF: On my system, this line is needed in order to get the keyboard inpot working
        _inputData.resetInputValues();
        return _fmu._fmuData._tcur;
    }

    void OMVisualizerFMU::resetInputs()
    {
        //reset real input values to 0
        for (size_t r = 0; r < _inputData._data.getNumReal(); ++r)
            _inputData._data._valuesReal[r] = 0.0;
        //reset integer input values to 0
        for (size_t i = 0; i < _inputData._data.getNumInteger(); ++i)
            _inputData._data._valuesInteger[i] = 0;
        //reset boolean input values to 0
        for (size_t b = 0; b < _inputData._data.getNumBoolean(); ++b)
            _inputData._data._valuesBoolean[b] = false;
        //reset string input values to 0
        for (size_t s = 0; s < _inputData._data.getNumString(); ++s)
            _inputData._data._valuesString[s] = "";
    }

    void OMVisualizerFMU::updateVisAttributes(const double time)
    {
        // Update all shapes
        rapidxml::xml_node<>* rootNode = _baseData->_xmlDoc.first_node();
        unsigned int shapeIdx = 0;
        rAndT rT;
        osg::ref_ptr<osg::Node> child = nullptr;
        for (rapidxml::xml_node<>* shapeNode = rootNode->first_node("shape"); shapeNode; shapeNode = shapeNode->next_sibling())
        {
            // get the values for the scene graph objects
            _baseData->_visAttr._type = getShapeType(shapeNode);

            //_baseData->_visAttr._length = getShapeAttrFMU((const char*) "length", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._length = getShapeAttrFMU(std::string("length").c_str(), shapeNode, time, _fmu._fmu);

            _baseData->_visAttr._width = getShapeAttrFMU((const char*) "width", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._height = getShapeAttrFMU((const char*) "height", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._r = getShapeVectorFMU((char*) "r", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._rShape = getShapeVectorFMU((char*) "r_shape", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._lDir = getShapeVectorFMU((char*) "lengthDir", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._wDir = getShapeVectorFMU((char*) "widthDir", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._color = getShapeVectorFMU((char*) "color", shapeNode, time, _fmu._fmu);
            _baseData->_visAttr._T = getShapeMatrixFMU((char*) "T", shapeNode, time, _fmu._fmu);
            rT = staticRotation(_baseData->_visAttr._r, _baseData->_visAttr._rShape, _baseData->_visAttr._T, _baseData->_visAttr._lDir, _baseData->_visAttr._wDir, _baseData->_visAttr._length, _baseData->_visAttr._width, _baseData->_visAttr._height, _baseData->_visAttr._type);
            _baseData->_visAttr._r = rT._r;
            _baseData->_visAttr._T = rT._T;

            _baseData->_visAttr._mat = assemblePokeMatrix(_baseData->_visAttr._mat, _baseData->_visAttr._T, _baseData->_visAttr._r);

            //update the shapes
            _nodeUpdater->_visAttr = _baseData->_visAttr;
			//_nodeUpdater->_visAttr.dumpVisAttributes();

            //get the scene graph nodes and stuff
            child = _viewerStuff->getScene().getRootNode()->getChild(shapeIdx);  // the transformation
            child->accept(*_nodeUpdater);
            ++shapeIdx;
        }
    }

    void OMVisualizerFMU::initializeVisAttributes(const double time)
    {
        _fmu.initialize(_simSettings);
        _omvManager->_visTime = _omvManager->_startTime;
        _omvManager->_simTime = _omvManager->_startTime;
        updateVisAttributes(_omvManager->_visTime);
    }

    void OMVisualizerFMU::updateScene(const double time)
    {
        _omvManager->_simTime = _omvManager->_visTime;
        double nextStep = _omvManager->_visTime + _omvManager->_hVisual;

        while (_omvManager->_simTime < nextStep)
        {
            //std::cout<<"simulate "<<omvManager->_simTime<<" to "<<nextStep<<std::endl;
            //_inputData.printValues();
            _omvManager->_simTime = simulateStep(_omvManager->_simTime);
        }
        updateVisAttributes(_omvManager->_visTime);
    }

    int OMVisualizerFMU::getDataTypeID()
    {
        return 1;
    }

}  // End namespace Model
