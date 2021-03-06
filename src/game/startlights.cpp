// This file is part of Dust Racing 2D.
// Copyright (C) 2012 Jussi Lind <jussi.lind@iki.fi>
//
// Dust Racing 2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// Dust Racing 2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dust Racing 2D. If not, see <http://www.gnu.org/licenses/>.

#include "startlights.hpp"
#include "inputhandler.hpp"

Startlights::Startlights()
: m_state(Init)
, m_counter(0)
, m_stepsPerState(60)
{
    m_stateToFunctionMap[Init]      = std::bind(&Startlights::stateInit,      this);
    m_stateToFunctionMap[Appear]    = std::bind(&Startlights::stateAppear,    this);
    m_stateToFunctionMap[FirstRow]  = std::bind(&Startlights::stateFirstRow,  this);
    m_stateToFunctionMap[SecondRow] = std::bind(&Startlights::stateSecondRow, this);
    m_stateToFunctionMap[ThirdRow]  = std::bind(&Startlights::stateThirdRow,  this);
    m_stateToFunctionMap[Go]        = std::bind(&Startlights::stateGo,        this);
    m_stateToFunctionMap[Disappear] = std::bind(&Startlights::stateDisappear, this);
    m_stateToFunctionMap[End]       = std::bind(&Startlights::stateEnd,       this);

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
    m_timer.setInterval(1000 / m_stepsPerState);
}

bool Startlights::timeElapsed(MCUint limit)
{
    if (++m_counter > limit)
    {
        m_counter = 0;
        return true;
    }
    return false;
}

void Startlights::updateAnimation()
{
    m_stateToFunctionMap[m_state]();
}

void Startlights::beginAnimation()
{
    m_state = Init;
    m_timer.start();
}

void Startlights::setDimensions(MCUint width, MCUint height)
{
    m_width  = width;
    m_height = height;
}

Startlights::State Startlights::state() const
{
    return m_state;
}

const MCVector3dF & Startlights::pos() const
{
    return m_pos;
}

MCFloat Startlights::glowScale() const
{
    return m_glowScale;
}

void Startlights::stateInit()
{
    const MCUint second = m_stepsPerState;

    m_pos = MCVector3dF(m_width / 2, 3 * m_height / 2, 0);
    m_animation.init(
         m_pos,
         m_pos,
         MCVector3dF(m_pos.i(), m_height / 2, 0),
         second / 3);
    m_state = Appear;
    m_glowScale = 1.0;
    InputHandler::setEnabled(false);
}

void Startlights::stateAppear()
{
    const MCUint second = m_stepsPerState;

    m_animation.update();
    if (timeElapsed(second))
    {
        m_state = FirstRow;
        emit messageRequested("3");
    }
}

void Startlights::stateFirstRow()
{
    const MCUint second = m_stepsPerState;

    if (timeElapsed(second))
    {
        m_state = SecondRow;
        emit messageRequested("2");
    }
}

void Startlights::stateSecondRow()
{
    const MCUint second = m_stepsPerState;

    if (timeElapsed(second))
    {
        m_state = ThirdRow;
        emit messageRequested("1");
    }
}

void Startlights::stateThirdRow()
{
    const MCUint second = m_stepsPerState;

    if (timeElapsed(second))
    {
        m_state = Go;
        emit messageRequested(QObject::tr("GO!!!"));
        emit raceStarted();
        InputHandler::setEnabled(true);
    }
}

void Startlights::stateGo()
{
    const MCUint second = m_stepsPerState;

    if (timeElapsed(second))
    {
        m_state = Disappear;
        m_animation.init(
            m_pos,
            m_pos,
            MCVector3dF(m_pos.i(), 3 * m_height / 2, 0),
            second / 3);
    }

    m_glowScale *= 0.75;
}

void Startlights::stateDisappear()
{
    if (m_animation.update())
    {
        m_state = End;
    }
}

void Startlights::stateEnd()
{
    m_timer.stop();
    emit animationEnded();
}
