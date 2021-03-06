/*
 * Copyright (C) 2016, Volker Waurich
 *
 * This file is part of OMVIS.
 *
 * OMVIS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OMVIS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OMVIS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Model/VisualizerFMU.hpp"
#include "Util/Logger.hpp"
#include "Util/Util.hpp"

#include <SDL.h>

#include <iostream>

namespace OMVIS
{
    namespace Model
    {

        /*-----------------------------------------
         * CONSTRUCTORS
         *---------------------------------------*/

        VisualizerFMU::VisualizerFMU(const std::string& modelFile, const std::string& path)
                : VisualizerAbstract(modelFile, path, VisType::FMU),
                  _fmu(std::make_shared<FMUWrapper>()),
                  _simSettings(std::make_shared<SimSettingsFMU>()),
                  _inputData(std::make_shared<InputData>()),
                  _joysticks()
        {
            LOGGER_WRITE("Initialize joysticks", Util::LC_LOADER, Util::LL_INFO);
            initJoySticks();
        }

        /*-----------------------------------------
         * INITIALIZATION METHODS
         *---------------------------------------*/

        void VisualizerFMU::initData()
        {
            VisualizerAbstract::initData();
            loadFMU(_baseData->getModelFile(), _baseData->getPath());
            _simSettings->setTend(_timeManager->getEndTime());
            _simSettings->setHdef(0.001);
            setVarReferencesInVisAttributes();

            //OMVisualizerFMU::initializeVisAttributes(_omvManager->getStartTime());
        }

        void VisualizerFMU::loadFMU(const std::string& modelFile, const std::string& path)
        {
            //setup fmu-simulation stuff
            //_simSettings = new SimSettings;
            //std::string fmuFileName = dir + model + ".fmu";

            //load and initialize fmu
            _fmu->load(modelFile, path);
            LOGGER_WRITE("VisualizerFMU::loadFMU: FMU was successfully loaded.", Util::LC_LOADER, Util::LL_DEBUG);

            _fmu->initialize(_simSettings);
            LOGGER_WRITE("VisualizerFMU::loadFMU: FMU was successfully initialized.", Util::LC_LOADER, Util::LL_DEBUG);

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
        }

        void VisualizerFMU::resetInputs()
        {
            _inputData->resetInputValues();
        }

        void VisualizerFMU::initJoySticks()
        {
            //Initialize SDL
            if (0 > SDL_Init(SDL_INIT_JOYSTICK))
            {
                LOGGER_WRITE("SDL could not be initialized.", Util::LC_LOADER, Util::LL_ERROR);
            }

            //Check for joysticks
            if (1 > SDL_NumJoysticks())
            {
                LOGGER_WRITE("No joysticks connected!", Util::LC_LOADER, Util::LL_WARNING);
            }
            else
            {
                LOGGER_WRITE("Found " + std::to_string(SDL_NumJoysticks()) + " joystick(s)", Util::LC_LOADER,
                             Util::LL_INFO);

                //Load joystick
                LOGGER_WRITE(std::string("START LOADING JOYSTICKS!!!!!!!!!"), Util::LC_LOADER, Util::LL_INFO);
                Control::JoystickDevice* newJoyStick(nullptr);
                for (int i = 0; i < SDL_NumJoysticks(); ++i)
                {
                    LOGGER_WRITE("LOAD JOYSTICK # " + std::to_string(i), Util::LC_LOADER, Util::LL_INFO);
                    newJoyStick = new Control::JoystickDevice(i);
                    _joysticks.push_back(newJoyStick);

                    if (nullptr == newJoyStick)
                    {
                        LOGGER_WRITE("Unable to open joystick! SDL Error: " + std::string(SDL_GetError()),
                                     Util::LC_LOADER, Util::LL_INFO);
                    }
                }
            }
        }

        /*-----------------------------------------
         * GETTERS and SETTERS
         *---------------------------------------*/

        const FMUWrapper* VisualizerFMU::getFMU() const
        {
            return _fmu.get();
        }

        std::shared_ptr<InputData> VisualizerFMU::getInputData() const
        {
            return _inputData;
        }

        fmi1_value_reference_t VisualizerFMU::getVarReferencesForObjectAttribute(ShapeObjectAttribute* attr)
        {
            fmi1_value_reference_t vr = 0;
            if (!attr->isConst)
            {
                fmi1_import_variable_t* var = fmi1_import_get_variable_by_name(_fmu->getFMU(), attr->cref.c_str());
                vr = fmi1_import_get_variable_vr(var);
            }
            return vr;
        }

        int VisualizerFMU::setVarReferencesInVisAttributes()
        {
            int isOk(0);

            try
            {
                size_t i = 0;
                for (auto& shape : _baseData->_shapes)
                {
                    shape = _baseData->_shapes[i];

                    shape._length.fmuValueRef = getVarReferencesForObjectAttribute(&shape._length);
                    shape._width.fmuValueRef = getVarReferencesForObjectAttribute(&shape._width);
                    shape._height.fmuValueRef = getVarReferencesForObjectAttribute(&shape._height);

                    shape._lDir[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._lDir[0]);
                    shape._lDir[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._lDir[1]);
                    shape._lDir[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._lDir[2]);

                    shape._wDir[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._wDir[0]);
                    shape._wDir[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._wDir[1]);
                    shape._wDir[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._wDir[2]);

                    shape._r[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._r[0]);
                    shape._r[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._r[1]);
                    shape._r[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._r[2]);

                    shape._rShape[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._rShape[0]);
                    shape._rShape[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._rShape[1]);
                    shape._rShape[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._rShape[2]);

                    shape._T[0].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[0]);
                    shape._T[1].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[1]);
                    shape._T[2].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[2]);
                    shape._T[3].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[3]);
                    shape._T[4].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[4]);
                    shape._T[5].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[5]);
                    shape._T[6].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[6]);
                    shape._T[7].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[7]);
                    shape._T[8].fmuValueRef = getVarReferencesForObjectAttribute(&shape._T[8]);

                    //shape.dumpVisAttributes();
                    _baseData->_shapes.at(i) = shape;
                    ++i;
                }  //end for
            }  // end try

            catch (std::exception& e)
            {
                LOGGER_WRITE("Something went wrong in VisualizerFMU::setVarReferencesInVisAttributes", Util::LC_SOLVER,
                             Util::LL_WARNING);
                isOk = 1;
            }
            return isOk;
        }

        void VisualizerFMU::setSimulationSettings(const UserSimSettingsFMU& simSetFMU)
        {
            if (Solver::NONE == simSetFMU.solver)
            {
                throw std::runtime_error("Solver: NONE is not a valid solver.");
            }
            else
            {
                _simSettings->setSolver(simSetFMU.solver);
            }

            if (0.0 >= simSetFMU.simStepSize)
            {
                throw std::runtime_error(
                        "Simulation step size of " + std::to_string(simSetFMU.simStepSize) + " is not valid.");
            }
            else
            {
                _simSettings->setHdef(simSetFMU.simStepSize);
            }

            if (0.0 >= simSetFMU.simEndTime)
            {
                throw std::runtime_error("Solver: Simulation end time <= 0.0 is not valid.");
            }
            else
            {
                _timeManager->setEndTime(simSetFMU.simEndTime);
            }
        }

        UserSimSettingsFMU VisualizerFMU::getCurrentSimSettings() const
        {
            return
            {   _simSettings->getSolver(), _simSettings->getHdef(), 99, _timeManager->getEndTime()};
        }

        /*-----------------------------------------
         * SIMULATION METHODS
         *---------------------------------------*/

        // MF, 17.10.2016: Unused?!
        void VisualizerFMU::simulate(Control::TimeManager& omvm)
        {
            while (omvm.getSimTime() < omvm.getRealTime() + omvm.getHVisual() && omvm.getSimTime() < omvm.getEndTime())
            {
                omvm.setSimTime(simulateStep(omvm.getSimTime()));
            }
        }

        double VisualizerFMU::simulateStep(const double time)
        {
            _fmu->prepareSimulationStep(time);

            /* Check if an event indicator has triggered */
            bool zeroCrossingEvent = _fmu->checkForTriggeredEvent();

            /* Handle any events */
            if ((nullptr != _simSettings->getCallEventUpdate()) || zeroCrossingEvent || _fmu->itsEventTime())
            {
                _fmu->handleEvents(_simSettings->getIntermediateResults());
            }

            /* Updated next time step */
            _fmu->updateNextTimeStep(_simSettings->getHdef());

            /* last step */
            _fmu->updateTimes(_simSettings->getTend());

            // Set inputs.
            for (auto& joystick : _joysticks)
            {
                joystick->detectContinuousInputEvents(_inputData);
            }
            _inputData->setInputsInFMU(_fmu->getFMU());
            //_inputData->printValues();

            /* Solve system */
            _fmu->solveSystem();

            //print out some values for debugging:
            //std::cout<<"DO EULER at "<< _fmu->getFMUData()->_tcur<<std::endl;
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
            _fmu->completedIntegratorStep(_simSettings->getCallEventUpdate());

            //vw: since we are detecting changing inputs, we have to keep the values during the steps. do not reset it
            _inputData->resetDiscreteInputValues();
            return _fmu->getFMUData()->_tcur;
        }

        void VisualizerFMU::initializeVisAttributes(const double /*time*/)
        {
            _fmu->initialize(_simSettings);
            _timeManager->setVisTime(_timeManager->getStartTime());
            _timeManager->setSimTime(_timeManager->getStartTime());
            setVarReferencesInVisAttributes();
            updateVisAttributes(_timeManager->getVisTime());
        }

        void VisualizerFMU::updateVisAttributes(const double time)
        {
            // Update all shapes.
            OMVIS::Util::rAndT rT;
            osg::ref_ptr<osg::Node> child = nullptr;
            try
            {
                fmi1_import_t* fmu = _fmu->getFMU();
                size_t i = 0;
                for (auto& shape : _baseData->_shapes)
                {
                    // Get the values for the scene graph objects
                    updateObjectAttributeFMU(&shape._length, fmu);
                    updateObjectAttributeFMU(&shape._width, fmu);
                    updateObjectAttributeFMU(&shape._height, fmu);

                    updateObjectAttributeFMU(&shape._lDir[0], fmu);
                    updateObjectAttributeFMU(&shape._lDir[1], fmu);
                    updateObjectAttributeFMU(&shape._lDir[2], fmu);

                    updateObjectAttributeFMU(&shape._wDir[0], fmu);
                    updateObjectAttributeFMU(&shape._wDir[1], fmu);
                    updateObjectAttributeFMU(&shape._wDir[2], fmu);

                    updateObjectAttributeFMU(&shape._r[0], fmu);
                    updateObjectAttributeFMU(&shape._r[1], fmu);
                    updateObjectAttributeFMU(&shape._r[2], fmu);

                    updateObjectAttributeFMU(&shape._rShape[0], fmu);
                    updateObjectAttributeFMU(&shape._rShape[1], fmu);
                    updateObjectAttributeFMU(&shape._rShape[2], fmu);

                    updateObjectAttributeFMU(&shape._T[0], fmu);
                    updateObjectAttributeFMU(&shape._T[1], fmu);
                    updateObjectAttributeFMU(&shape._T[2], fmu);
                    updateObjectAttributeFMU(&shape._T[3], fmu);
                    updateObjectAttributeFMU(&shape._T[4], fmu);
                    updateObjectAttributeFMU(&shape._T[5], fmu);
                    updateObjectAttributeFMU(&shape._T[6], fmu);
                    updateObjectAttributeFMU(&shape._T[7], fmu);
                    updateObjectAttributeFMU(&shape._T[8], fmu);
                    rT = Util::rotation(
                            osg::Vec3f(shape._r[0].exp, shape._r[1].exp, shape._r[2].exp),
                            osg::Vec3f(shape._rShape[0].exp, shape._rShape[1].exp, shape._rShape[2].exp),
                            osg::Matrix3(shape._T[0].exp, shape._T[1].exp, shape._T[2].exp, shape._T[3].exp,
                                         shape._T[4].exp, shape._T[5].exp, shape._T[6].exp, shape._T[7].exp,
                                         shape._T[8].exp),
                            osg::Vec3f(shape._lDir[0].exp, shape._lDir[1].exp, shape._lDir[2].exp),
                            osg::Vec3f(shape._wDir[0].exp, shape._wDir[1].exp, shape._wDir[2].exp), shape._length.exp,
                            shape._type);

                    Util::assemblePokeMatrix(shape._mat, rT._T, rT._r);

                    // Update the shapes.
                    _nodeUpdater->_shape = shape;

                    // Get the scene graph nodes and stuff.
                    //_viewerStuff->dumpOSGTreeDebug();
                    child = _viewerStuff->getScene()->getRootNode()->getChild(i);  // the transformation
                    child->accept(*_nodeUpdater);
                    ++i;
                }  //end for
            }  // end try
            catch (std::exception& ex)
            {
                auto msg = "Error in VisualizerFMU::updateVisAttributes at time point " + std::to_string(time) + "\n"
                        + std::string(ex.what());
                LOGGER_WRITE(msg, Util::LC_SOLVER, Util::LL_WARNING);
                throw(msg);
            }
        }

        void VisualizerFMU::updateScene(const double /*time*/)
        {
            _timeManager->updateTick();  //for real-time measurement

            _timeManager->setSimTime(_timeManager->getVisTime());
            double nextStep = _timeManager->getVisTime() + _timeManager->getHVisual();

            double vis1 = _timeManager->getRealTime();
            while (_timeManager->getSimTime() < nextStep)
            {
                //std::cout<<"simulate "<<omvManager->_simTime<<" to "<<nextStep<<std::endl;
                //_inputData.printValues();
                _timeManager->setSimTime(simulateStep(_timeManager->getSimTime()));
            }
            _timeManager->updateTick();                     //for real-time measurement
            _timeManager->setRealTimeFactor(_timeManager->getHVisual() / (_timeManager->getRealTime() - vis1));
            updateVisAttributes(_timeManager->getVisTime());
        }

        // Todo pass by const ref
        void VisualizerFMU::updateObjectAttributeFMU(Model::ShapeObjectAttribute* attr, fmi1_import_t* fmu)
        {
            if (!attr->isConst)
            {
                fmi1_real_t a = attr->exp;
                fmi1_import_get_real(fmu, &attr->fmuValueRef, 1, &a);
                attr->exp = static_cast<float>(a);
            }
        }

    }  // namespace Model
}  // namespace OMVIS
