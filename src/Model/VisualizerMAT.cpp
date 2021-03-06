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

#include "Model/VisualizerMAT.hpp"
#include "Util/Logger.hpp"
#include "Util/Util.hpp"

namespace OMVIS
{
    namespace Model
    {

        // [MF: 2016-06-13]: This method is not used. Do we need it?
        //        double* omc_get_varValue1(ModelicaMatReader* reader, const char* varName, double time)
        //        {
        //            double val = 0.0;
        //            double* res = &val;
        //            ModelicaMatVariable_t* var = nullptr;
        //            var = omc_matlab4_find_var(reader, varName);
        //            if (var == nullptr)
        //                LOGGER_WRITE(std::string("Could not get variable ") + varName + std::string(" from result file."), Util::LC_CTR, Util::LL_ERROR);
        //            else
        //                omc_matlab4_val(res, reader, var, time);
        //
        //            return res;
        //        }

        /*-----------------------------------------
         * CONSTRUCTORS
         *---------------------------------------*/

        VisualizerMAT::VisualizerMAT(const std::string& modelFile, const std::string& path)
                : VisualizerAbstract(modelFile, path, VisType::MAT),
                  _matReader()
        {
        }

        /*-----------------------------------------
         * INITIALIZATION METHODS
         *---------------------------------------*/

        void VisualizerMAT::initData()
        {
            VisualizerAbstract::initData();
            readMat(_baseData->getModelFile(), _baseData->getPath());
            _timeManager->setStartTime(omc_matlab4_startTime(&_matReader));
            _timeManager->setEndTime(omc_matlab4_stopTime(&_matReader));
        }

        void VisualizerMAT::initializeVisAttributes(const double time)
        {
            if (0.0 > time)
            {
                LOGGER_WRITE("Cannot load visualization attributes for time point < 0.0.", Util::LC_LOADER,
                             Util::LL_ERROR);
            }
            updateVisAttributes(time);
        }

        void VisualizerMAT::readMat(const std::string& modelFile, const std::string& path)
        {
            std::string resFileName = path + modelFile;     // + "_res.mat";

            // Check if the MAT file exists.
            if (!Util::fileExists(resFileName))
            {
                auto msg = "Could not find MAT file" + resFileName + ".";
                LOGGER_WRITE(msg, Util::LC_LOADER, Util::LL_ERROR);
                throw std::runtime_error(msg);
            }
            else
            {
                // Read mat file.
                auto ret = omc_new_matlab4_reader(resFileName.c_str(), &_matReader);
                // Check return value.
                if (nullptr != ret)
                {
                    std::string msg(ret);
                    LOGGER_WRITE(msg, Util::LC_LOADER, Util::LL_ERROR);
                    throw std::runtime_error(msg);
                }
            }

            /*
             FILE * fileA = fopen("allVArs.txt", "w+");
             omc_matlab4_print_all_vars(fileA, &matReader);
             fclose(fileA);
             */
        }

        /*-----------------------------------------
         * SIMULATION METHODS
         *---------------------------------------*/

        void VisualizerMAT::updateVisAttributes(const double time)
        {
            // Update all shapes.
            unsigned int shapeIdx = 0;
            OMVIS::Util::rAndT rT;
            osg::ref_ptr<osg::Node> child = nullptr;
            ModelicaMatReader* tmpReaderPtr = &_matReader;
            try
            {
                for (auto& shape : _baseData->_shapes)
                {
                    // Get the values for the scene graph objects
                    updateObjectAttributeMAT(&shape._length, time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._width, time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._height, time, tmpReaderPtr);

                    updateObjectAttributeMAT(&shape._lDir[0], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._lDir[1], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._lDir[2], time, tmpReaderPtr);

                    updateObjectAttributeMAT(&shape._wDir[0], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._wDir[1], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._wDir[2], time, tmpReaderPtr);

                    updateObjectAttributeMAT(&shape._r[0], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._r[1], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._r[2], time, tmpReaderPtr);

                    updateObjectAttributeMAT(&shape._rShape[0], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._rShape[1], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._rShape[2], time, tmpReaderPtr);

                    updateObjectAttributeMAT(&shape._T[0], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[1], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[2], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[3], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[4], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[5], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[6], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[7], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._T[8], time, tmpReaderPtr);

                    updateObjectAttributeMAT(&shape._color[0], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._color[1], time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._color[2], time, tmpReaderPtr);

                    updateObjectAttributeMAT(&shape._specCoeff, time, tmpReaderPtr);
                    updateObjectAttributeMAT(&shape._extra, time, tmpReaderPtr);

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
                    //shape.dumpVisAttributes();

                    // Get the scene graph nodes and stuff.
                    child = _viewerStuff->getScene()->getRootNode()->getChild(shapeIdx);  // the transformation
                    child->accept(*_nodeUpdater);
                    ++shapeIdx;
                }
            }
            catch (std::exception& ex)
            {
                auto msg = "Error in VisualizerMAT::updateVisAttributes at time point " + std::to_string(time) + "\n"
                        + std::string(ex.what());
                LOGGER_WRITE(msg, Util::LC_SOLVER, Util::LL_WARNING);
                throw(msg);
            }
        }

        void VisualizerMAT::updateScene(const double time)
        {
            if (0.0 > time)
            {
                LOGGER_WRITE("Cannot load visualization attributes for time point < 0.0.", Util::LC_SOLVER,
                             Util::LL_ERROR);
            }

            _timeManager->updateTick();  //for real-time measurement
            double visTime = _timeManager->getRealTime();

            updateVisAttributes(time);

            _timeManager->updateTick();  //for real-time measurement
            visTime = _timeManager->getRealTime() - visTime;
            _timeManager->setRealTimeFactor(_timeManager->getHVisual() / visTime);
        }

        void VisualizerMAT::updateObjectAttributeMAT(Model::ShapeObjectAttribute* attr, double time,
                                                     ModelicaMatReader* reader)
        {
            if (!attr->isConst)
            {
                attr->exp = omcGetVarValue(reader, attr->cref.c_str(), time);
            }
        }

        double VisualizerMAT::omcGetVarValue(ModelicaMatReader* reader, const char* varName, double time)
        {
            double val = 0.0;
            ModelicaMatVariable_t* var = nullptr;
            var = omc_matlab4_find_var(reader, varName);
            if (var == nullptr)
            {
                LOGGER_WRITE("Did not get variable from result file. Variable name is " + std::string(varName) + ".",
                             Util::LC_SOLVER, Util::LL_ERROR);
            }
            else
            {
                omc_matlab4_val(&val, reader, var, time);
            }

            return val;
        }

        void VisualizerMAT::setSimulationSettings(const Model::UserSimSettingsMAT& simSetMAT)
        {
            auto newVal = simSetMAT.speedup * _timeManager->getHVisual();
            _timeManager->setHVisual(newVal);
        }

    }  // namespace Model
}  // namespace OMVIS
