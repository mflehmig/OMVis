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
#include "Util/Util.hpp"
#include "Util/ObjectAttribute.hpp"
#include "Model/OMVisualizerFMU.hpp"
#include "View/OMVisScene.hpp"
#include "Control/OMVisManager.hpp"
#include "Control/JoystickDevice.hpp"

#include <SDL.h>

#include <iostream>

namespace OMVIS
{
    namespace Model
    {
        /*-----------------------------------------
         * CONSTRUCTORS
         *---------------------------------------*/

        OMVisualizerFMU::OMVisualizerFMU(const std::string& fileName, const std::string& dirPath)
                : OMVisualizerAbstract(fileName, dirPath),
                  _fmu(new FMU()),
                  _simSettings(new SimSettings()),
                  _inputData(new InputData()),
                  _joysticks()
        {
            LOGGER_WRITE(std::string("Initialize joysticks"), Util::LC_LOADER, Util::LL_INFO);
            initJoySticks();
        }

        /*-----------------------------------------
         * INITIALIZATION METHODS
         *---------------------------------------*/

        /// \todo: Set the error variable isOk.
        int OMVisualizerFMU::loadFMU(const std::string& modelFile, const std::string& dirPath)
        {
            int isOk(0);
            //setup fmu-simulation stuff
            //_simSettings = new SimSettings;
            //std::string fmuFileName = dir + model + ".fmu";

            //load and initialize fmu
            _fmu->load(modelFile, dirPath);
            LOGGER_WRITE(std::string("OMVisualizerFMU::loadFMU: FMU was successfully loaded."), Util::LC_LOADER, Util::LL_DEBUG);

            _fmu->initialize(_simSettings);
            LOGGER_WRITE(std::string("OMVisualizerFMU::loadFMU: FMU was successfully initialized."), Util::LC_LOADER, Util::LL_DEBUG);

            _inputData->initializeInputs(_fmu->getFMU());
            _inputData->printValues();
            //assign interactive inputs
            //for (unsigned int i = 0; i < inputs.n_inputs; i++){
            //string key = "";
            //std::cout<<"assign input "<<i<<" :"<<std::endl;
            //getline(cin,key);
            //int keyInt = getchar();
            //std::cout<<"the key is "<<keyInt<<" !"<<std::endl;
            //}
            return isOk;
        }

        int OMVisualizerFMU::initData()
        {
            int isOk(0);
            isOk = OMVisualizerAbstract::initData();
            isOk += loadFMU(_baseData->getModelName(), _baseData->getDirName());
            _simSettings->setTend(_omvManager->getEndTime());
            _simSettings->setHdef(0.001);

            return isOk;
        }

        void OMVisualizerFMU::resetInputs()
        {
            _inputData->resetInputValues();
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

        /*-----------------------------------------
         * GETTERS and SETTERS
         *---------------------------------------*/

        std::string OMVisualizerFMU::getType() const
        {
            return "fmu";
        }

        const FMU* OMVisualizerFMU::getFMU() const
        {
            return _fmu.get();
        }

        std::shared_ptr<InputData> OMVisualizerFMU::getInputData()
        {
            return _inputData;
        }

        /*-----------------------------------------
         * SIMULATION METHODS
         *---------------------------------------*/

        void OMVisualizerFMU::simulate(Control::OMVisManager& omvm)
        {
            while (omvm.getSimTime() < omvm.getRealTime() + omvm.getHVisual() && omvm.getSimTime() < omvm.getEndTime())
            {
                omvm.setSimTime(simulateStep(omvm.getSimTime()));
            }
        }

        double OMVisualizerFMU::simulateStep(const double time)
        {
            //tcur = settings.tstart;
            //hcur = settings.hdef;
            int zero_crossning_event = 0;
            _fmu->prepareSimulationStep(time);

            /* Check if an event indicator has triggered */
            bool zeroCrossingEvent = _fmu->checkForTriggeredEvent();

            /* Handle any events */
            if (_simSettings->getCallEventUpdate() || zeroCrossingEvent || _fmu->itsEventTime())
            {
                _fmu->handleEvents(_simSettings->getIntermediateResults());
            }

            /* Updated next time step */
            _fmu->updateNextTimeStep(_simSettings->getHdef());

            /* last step */
            _fmu->updateTimes(_simSettings->getTend());

            //set inputs
            for (size_t i = 0; i < _numJoysticks; ++i)
            {
                _joysticks[i]->detectContinuousInputEvents(_inputData);
                _inputData->setInputsInFMU(_fmu->getFMU());
                //std::cout << "JOY" << i << " XDir " <<_joysticks[i]->getXDir() <<" YDir "<< _joysticks[i]->getYDir() << std::endl;
            }

            //_inputData.printValues();
            //X2 MF: On my system, this line is needed in order to get the keyboard input working
            _inputData->setInputsInFMU(_fmu->getFMU());

            /* Solve system */
            _fmu->solveSystem();

            //print out some values for debugging:
            //std::cout<<"DO EULER at "<< _fmu->_fmuData._tcur<<std::endl;
            //fmi1_import_variable_t* var = fmi1_import_get_variable_by_name(_fmul._fmu, "prismatic.s");
            //const fmi1_value_reference_t vr  = fmi1_import_get_variable_vr(var);
            //double value = -1.0;
            //fmi1_import_get_real(_fmul._fmu, &vr, 1, &value);
            //std::cout<<"value "<<value<<std::endl;

            // integrate a step with euler
            _fmu->doEulerStep();

            /* Set states */
            _fmu->setContinuousStates();

            /* Step is complete */
            _fmu->completedIntegratorStep(&_simSettings->_callEventUpdate);

            //vw: since we are detecting changing inputs, we have to keep the values during the steps. do not reset it
            //X2 MF: On my system, this line is needed in order to get the keyboard inpot working
            _inputData->resetDiscreteInputValues();
            return _fmu->getFMUData()->_tcur;
        }

        void OMVisualizerFMU::initializeVisAttributes(const double time)
        {
            _fmu->initialize(_simSettings);
            _omvManager->setVisTime(_omvManager->getStartTime());
            _omvManager->setSimTime(_omvManager->getStartTime());
            updateVisAttributes(_omvManager->getVisTime());
        }

        int OMVisualizerFMU::updateVisAttributes(const double time)
        {
            int isOk(0);

            // Update all shapes
            OMVIS::Util::rAndT rT;
            osg::ref_ptr<osg::Node> child = nullptr;
            try
            {
                fmi1_import_t* fmu = _fmu->getFMU();
                ShapeObject shape;
                for (std::vector<Model::ShapeObject>::size_type i = 0; i != _baseData->_shapes.size(); ++i)
                {
                    shape = _baseData->_shapes[i];

                    // get the values for the scene graph objects
                    Util::updateObjectAttributeFMU(&shape._length, time, fmu);
                    Util::updateObjectAttributeFMU(&shape._width, time, fmu);
                    Util::updateObjectAttributeFMU(&shape._height, time, fmu);

                    Util::updateObjectAttributeFMU(&shape._lDir[0], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._lDir[1], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._lDir[2], time, fmu);

                    Util::updateObjectAttributeFMU(&shape._wDir[0], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._wDir[1], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._wDir[2], time, fmu);

                    Util::updateObjectAttributeFMU(&shape._r[0], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._r[1], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._r[2], time, fmu);

                    Util::updateObjectAttributeFMU(&shape._rShape[0], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._rShape[1], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._rShape[2], time, fmu);

                    Util::updateObjectAttributeFMU(&shape._T[0], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[1], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[2], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[3], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[4], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[5], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[6], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[7], time, fmu);
                    Util::updateObjectAttributeFMU(&shape._T[8], time, fmu);

                    rT = Util::rotation(osg::Vec3f(shape._r[0].exp, shape._r[1].exp, shape._r[2].exp), osg::Vec3f(shape._rShape[0].exp, shape._rShape[1].exp, shape._rShape[2].exp), osg::Matrix3(shape._T[0].exp, shape._T[1].exp, shape._T[2].exp, shape._T[3].exp, shape._T[4].exp, shape._T[5].exp, shape._T[6].exp, shape._T[7].exp, shape._T[8].exp),
                                        osg::Vec3f(shape._lDir[0].exp, shape._lDir[1].exp, shape._lDir[2].exp), osg::Vec3f(shape._wDir[0].exp, shape._wDir[1].exp, shape._wDir[2].exp), shape._length.exp, shape._width.exp, shape._height.exp, shape._type);

                    shape._mat = Util::assemblePokeMatrix(shape._mat, rT._T, rT._r);

                    //update the shapes
                    _nodeUpdater->_shape = shape;
                    //_baseData->_visAttr.dumpVisAttributes();

                    //get the scene graph nodes and stuff
                    child = _viewerStuff->getScene().getRootNode()->getChild(i);  // the transformation
                    child->accept(*_nodeUpdater);

                }  //end for
            }  // end try

            catch (std::exception& e)
            {
                LOGGER_WRITE(std::string("Something went wrong in OMVisualizer::updateVisAttributes at time point ") + std::to_string(time) + std::string(" ."), Util::LC_SOLVER, Util::LL_WARNING);
                isOk = 1;
            }
            return isOk;
        }

        void OMVisualizerFMU::updateScene(const double time)
        {
            _omvManager->updateTick();            //for real-time measurement

            _omvManager->setSimTime(_omvManager->getVisTime());
            double nextStep = _omvManager->getVisTime() + _omvManager->getHVisual();

            double vis1 = _omvManager->getRealTime();
            while (_omvManager->getSimTime() < nextStep)
            {
                //std::cout<<"simulate "<<omvManager->_simTime<<" to "<<nextStep<<std::endl;
                //_inputData.printValues();
                _omvManager->setSimTime(simulateStep(_omvManager->getSimTime()));
            }
            _omvManager->updateTick();                     //for real-time measurement
            _omvManager->setRealTimeFactor(_omvManager->getHVisual() / (_omvManager->getRealTime() - vis1));
            updateVisAttributes(_omvManager->getVisTime());
        }

    }  // End namespace Model
}  // End namespace OMVIS
