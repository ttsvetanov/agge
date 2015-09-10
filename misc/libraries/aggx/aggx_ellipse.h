//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#ifndef AGGX_ELLIPSE_INCLUDED
#define AGGX_ELLIPSE_INCLUDED

#include <agge/path.h>

#include "basics.h"

#include <math.h>

namespace aggx
{

    //----------------------------------------------------------------ellipse
    class ellipse
    {
    public:
        ellipse() : 
            m_x(0.0), m_y(0.0), m_rx(1.0), m_ry(1.0), m_scale(1.0), 
            m_num(4), m_step(0), m_cw(false) {}

        ellipse(real x, real y, real rx, real ry, 
                unsigned num_steps=0, bool cw=false) :
            m_x(x), m_y(y), m_rx(rx), m_ry(ry), m_scale(1.0), 
            m_num(num_steps), m_step(0), m_cw(cw) 
        {
            if(m_num == 0) calc_num_steps();
        }

        void init(real x, real y, real rx, real ry, 
                  unsigned num_steps=0, bool cw=false);

        void approximation_scale(real scale);
        void rewind(unsigned path_id);
        unsigned vertex(real* x, real* y);

    private:
        void calc_num_steps();

        real m_x;
        real m_y;
        real m_rx;
        real m_ry;
        real m_scale;
        unsigned m_num;
        unsigned m_step;
        bool m_cw;
    };

    //------------------------------------------------------------------------
    inline void ellipse::init(real x, real y, real rx, real ry, 
                              unsigned num_steps, bool cw)
    {
        m_x = x;
        m_y = y;
        m_rx = rx;
        m_ry = ry;
        m_num = num_steps;
        m_step = 0;
        m_cw = cw;
        if(m_num == 0) calc_num_steps();
    }

    //------------------------------------------------------------------------
    inline void ellipse::approximation_scale(real scale)
    {   
        m_scale = scale;
        calc_num_steps();
    }

    //------------------------------------------------------------------------
    inline void ellipse::calc_num_steps()
    {
        real ra = (fabs(m_rx) + fabs(m_ry)) / 2;
        real da = acos(ra / (ra + 0.125f / m_scale)) * 2;
        m_num = uround(2*pi / da);
    }

    //------------------------------------------------------------------------
    inline void ellipse::rewind(unsigned)
    {
        m_step = 0;
    }

    //------------------------------------------------------------------------
    inline unsigned ellipse::vertex(real* x, real* y)
    {
        if(m_step == m_num) 
        {
            ++m_step;
			return agge::path_command_end_poly | agge::path_flag_close;
        }
		if(m_step > m_num) return agge::path_command_stop;
        real angle = real(m_step) / real(m_num) * 2.0f * pi;
        if(m_cw) angle = 2.0f * pi - angle;
        *x = m_x + cos(angle) * m_rx;
        *y = m_y + sin(angle) * m_ry;
        m_step++;
		return ((m_step == 1) ? agge::path_command_move_to : agge::path_command_line_to);
    }

}

#endif