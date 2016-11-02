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

#include "Model/ShapeObject.hpp"
#include "Util/Visualize.hpp"

#include <iostream>

namespace OMVIS
{
    namespace Model
    {

        /*-----------------------------------------
         * CONSTRUCTORS
         *---------------------------------------*/

        ShapeObject::ShapeObject()
                : _id("noID"),
                  _type("box"),
			      _fileName("noFile"),
                  _length(ShapeObjectAttribute(0.1)),
                  _width(ShapeObjectAttribute(0.1)),
                  _height(ShapeObjectAttribute(0.1)),
                  _r{ShapeObjectAttribute(0.1), ShapeObjectAttribute(0.1), ShapeObjectAttribute(0.1)},
                  _rShape{ShapeObjectAttribute(0.0), ShapeObjectAttribute(0.0), ShapeObjectAttribute(0.0)},
                  _lDir{ShapeObjectAttribute(1.0), ShapeObjectAttribute(0.0), ShapeObjectAttribute(0.0)},
                  _wDir{ShapeObjectAttribute(0.0), ShapeObjectAttribute(1.0), ShapeObjectAttribute(0.0)},
                  _color{ShapeObjectAttribute(255.0), ShapeObjectAttribute(255.0), ShapeObjectAttribute(255.0)},
                  _T{ShapeObjectAttribute(0.0), ShapeObjectAttribute(0.0), ShapeObjectAttribute(1.0),
                     ShapeObjectAttribute(1.0), ShapeObjectAttribute(0.0), ShapeObjectAttribute(0.0),
                     ShapeObjectAttribute(0.0), ShapeObjectAttribute(1.0), ShapeObjectAttribute(0.0)},
                  _specCoeff(ShapeObjectAttribute(0.7)),
                  _mat(osg::Matrix(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)),
                  _extra(0.0)
        {
        }

        /*-----------------------------------------
         * PRINT METHODS
         *---------------------------------------*/

        void ShapeObject::dumpVisAttributes() const
        {
            std::cout << "id " << _id << std::endl;
            std::cout << "type " << _type << std::endl;
			std::cout << "fileName " << _fileName << std::endl;
            std::cout << "length " << _length.getValueString() << std::endl;
            std::cout << "width " << _width.getValueString() << std::endl;
            std::cout << "height " << _height.getValueString() << std::endl;
            std::cout << "lDir " << _lDir[0].getValueString() << ", " << _lDir[1].getValueString() << ", " << _lDir[2].getValueString() << ", " << std::endl;
            std::cout << "wDir " << _wDir[0].getValueString() << ", " << _wDir[1].getValueString() << ", " << _wDir[2].getValueString() << ", " << std::endl;
            std::cout << "r " << _r[0].getValueString() << ", " << _r[1].getValueString() << ", " << _r[2].getValueString() << ", " << std::endl;
            std::cout << "r_shape " << _rShape[0].getValueString() << ", " << _rShape[1].getValueString() << ", " << _rShape[2].getValueString() << ", " << std::endl;
            std::cout << "T0 " << _T[0].getValueString() << ", " << _T[1].getValueString() << ", " << _T[2].getValueString() << ", " << std::endl;
            std::cout << "   " << _T[3].getValueString() << ", " << _T[4].getValueString() << ", " << _T[5].getValueString() << ", " << std::endl;
            std::cout << "   " << _T[6].getValueString() << ", " << _T[7].getValueString() << ", " << _T[8].getValueString() << ", " << std::endl;
            std::cout << "color " << _color[0].getValueString() << ", " << _color[1].getValueString() << ", " << _color[2].getValueString() << ", " << std::endl;
            std::cout << "mat " << _mat(0, 0) << ", " << _mat(0, 1) << ", " << _mat(0, 2) << ", " << _mat(0, 3) << std::endl;
            std::cout << "    " << _mat(1, 0) << ", " << _mat(1, 1) << ", " << _mat(1, 2) << ", " << _mat(1, 3) << std::endl;
            std::cout << "    " << _mat(2, 0) << ", " << _mat(2, 1) << ", " << _mat(2, 2) << ", " << _mat(2, 3) << std::endl;
            std::cout << "    " << _mat(3, 0) << ", " << _mat(3, 1) << ", " << _mat(3, 2) << ", " << _mat(3, 3) << std::endl;
            std::cout << "extra " << _extra.getValueString() << std::endl;
        }

    } // End namespace Model
} // End namespace OMVis
