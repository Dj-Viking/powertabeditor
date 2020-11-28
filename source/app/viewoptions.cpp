/*
  * Copyright (C) 2015 Cameron White
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
  
#include "viewoptions.h"

static constexpr double MIN_ZOOM = 25;
static constexpr double MAX_ZOOM = 300;

ViewOptions::ViewOptions() : myZoom(100.0)
{
}

void ViewOptions::setZoom(double percent)
{
    myZoom = std::max(MIN_ZOOM, std::min(MAX_ZOOM, percent));
}
