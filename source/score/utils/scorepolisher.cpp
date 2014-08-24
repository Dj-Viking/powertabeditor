/*
  * Copyright (C) 2014 Cameron White
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

#include "scorepolisher.h"

#include <map>
#include <score/score.h>
#include <score/voiceutils.h>
#include <score/utils.h>

static int getDefaultNoteSpacing(const boost::rational<int> &duration)
{
    return std::max(2 * boost::rational_cast<int>(duration), 1);
}

template <typename T>
static void shiftItemsAtPosition(const T &items, int position, int newPosition,
                                 std::unordered_set<const void *> &knownItems)
{
    for (auto &item : ScoreUtils::findInRange(items, position, position))
    {
        if (knownItems.find(&item) != knownItems.end())
            continue;

        knownItems.insert(&item);
        item.setPosition(newPosition);
    }
}

static void polishSystem(System &system)
{
    // Format each bar separately.
    for (Barline &leftBar : system.getBarlines())
    {
        Barline *rightBar = system.getNextBarline(leftBar.getPosition());
        if (!rightBar)
            break;

        std::unordered_map<const Position *, boost::rational<int>> timestamps;
        std::map<boost::rational<int>, int> timestampPositions;
        int maxPosition = 0;

        // For each timestamp, compute the maximum position at that timestamp
        // for any staff.
        for (const Staff &staff : system.getStaves())
        {
            for (const Voice &voice : staff.getVoices())
            {
                boost::rational<int> timestamp;
                int currentPosition = 0;

                for (const Position &position : ScoreUtils::findInRange(
                         voice.getPositions(), leftBar.getPosition(),
                         rightBar->getPosition()))
                {
                    boost::rational<int> duration =
                        VoiceUtils::getDurationTime(voice, position);

                    timestampPositions[timestamp] = std::max(
                        timestampPositions[timestamp], currentPosition);

                    currentPosition = timestampPositions[timestamp] +
                                      getDefaultNoteSpacing(duration);
                    timestamps[&position] = timestamp;
                    timestamp += duration;
                }

                maxPosition = std::max(maxPosition, currentPosition);
            }
        }

        // Adjust!
        const int startPos =
            (leftBar.getPosition() == 0) ? 0 : leftBar.getPosition() + 1;
        const int oldEndPos = rightBar->getPosition();
        const int endPos = startPos + maxPosition;

        if (endPos > oldEndPos)
        {
            SystemUtils::shift(system, rightBar->getPosition(),
                               endPos - rightBar->getPosition());
        }
        else
            rightBar->setPosition(endPos);

        std::unordered_set<const void *> knownItems;
        for (Staff &staff : system.getStaves())
        {
            for (Voice &voice : staff.getVoices())
            {
                for (Position &pos :
                     ScoreUtils::findInRange(voice.getPositions(),
                                             leftBar.getPosition(), oldEndPos))
                {
                    // Since we're moving around irregular groups, we need to
                    // have precomputed the durations of each position.
                    boost::rational<int> timestamp = timestamps[&pos];
                    const int currentPosition = pos.getPosition();
                    const int newPosition =
                        startPos + timestampPositions[timestamp];

                    // Move any irregular groups, etc that start at this
                    // position. If the group moves forward, we need to be
                    // careful not to try to move it again on a later iteration.
                    shiftItemsAtPosition(voice.getIrregularGroupings(),
                                         currentPosition, newPosition,
                                         knownItems);
                    shiftItemsAtPosition(staff.getDynamics(), currentPosition,
                                         newPosition, knownItems);
                    shiftItemsAtPosition(system.getTextItems(), currentPosition,
                                         newPosition, knownItems);
                    shiftItemsAtPosition(system.getChords(), currentPosition,
                                         newPosition, knownItems);
                    shiftItemsAtPosition(system.getTempoMarkers(),
                                         currentPosition, newPosition,
                                         knownItems);
                    shiftItemsAtPosition(system.getDirections(),
                                         currentPosition, newPosition,
                                         knownItems);
                    shiftItemsAtPosition(system.getPlayerChanges(),
                                         currentPosition, newPosition,
                                         knownItems);

                    pos.setPosition(newPosition);
                }
            }
        }
    }
}

void ScoreUtils::polishScore(Score &score)
{
    for (System &system : score.getSystems())
        polishSystem(system);
}