/*
 * This file is part of Heliflight 3D.
 *
 * Heliflight 3D is free software. You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Heliflight 3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#include "build/build_config.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"

#include "config/config.h"
#include "config/config_reset.h"

#include "fc/runtime_config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/governor.h"

#include "rx/rx.h"

#include "pg/pg.h"
#include "pg/pg_ids.h"

#include "sensors/gyro.h"


PG_REGISTER_ARRAY(mixer_t, MIXER_RULE_COUNT, mixerRules, PG_HELI_MIXER, 0);


FAST_RAM_ZERO_INIT int mixerActiveServos = 0;
FAST_RAM_ZERO_INIT int mixerActiveMotors = 0;

FAST_RAM_ZERO_INIT uint8_t mixerRuleCount;
FAST_RAM_ZERO_INIT mixer_t mixer[MIXER_RULE_COUNT];

FAST_RAM_ZERO_INIT float mixerInput[MIXER_INPUT_COUNT];
FAST_RAM_ZERO_INIT float mixerOutput[MIXER_OUTPUT_COUNT];

FAST_RAM_ZERO_INIT int16_t mixerOverride[MIXER_INPUT_COUNT];

FAST_RAM_ZERO_INIT float cyclicTotal = 0;
FAST_RAM_ZERO_INIT float cyclicLimit = 1;


void mixerInit(void)
{
    mixerRuleCount = 0;

    for (int i = 0; i < MIXER_RULE_COUNT; i++) {
        if (mixerRules(i)->oper == MIXER_OP_NUL)
            break;

        if (mixerRules(i)->output < MIXER_OUTPUT_MOTORS)
            mixerActiveServos = MAX(mixerActiveServos, i + 1);
        else
            mixerActiveMotors = MAX(mixerActiveMotors, i - MIXER_OUTPUT_MOTORS + 1);

        memcpy(&mixer[i], mixerRules(i), sizeof(mixer_t));
        mixerRuleCount++;
    }

    for (int i = 0; i < MIXER_INPUT_COUNT; i++) {
        mixerOverride[i] = MIXER_OVERRIDE_OFF;
    }
}

void mixerInitProfile(void)
{
    cyclicLimit = currentPidProfile->pidSumLimit * PID_MIXER_SCALING;
}

void mixerUpdate(void)
{
    if (FLIGHT_MODE(PASSTHRU_MODE)) {
        mixerInput[MIXER_IN_STABILIZED_ROLL]  = rcCommand[ROLL];
        mixerInput[MIXER_IN_STABILIZED_PITCH] = rcCommand[PITCH];
        mixerInput[MIXER_IN_STABILIZED_YAW]   = rcCommand[YAW];
    } else {
        mixerInput[MIXER_IN_STABILIZED_ROLL]  = pidData[FD_ROLL].SumLim  * PID_MIXER_SCALING;
        mixerInput[MIXER_IN_STABILIZED_PITCH] = pidData[FD_PITCH].SumLim * PID_MIXER_SCALING;
        mixerInput[MIXER_IN_STABILIZED_YAW]   = pidData[FD_YAW].SumLim   * PID_MIXER_SCALING;
    }
    
    mixerInput[MIXER_IN_STABILIZED_COLLECTIVE]  = rcCommand[COLLECTIVE];
    
    mixerInput[MIXER_IN_GOVERNOR_MAIN] = govOutput[0];
    mixerInput[MIXER_IN_GOVERNOR_TAIL] = govOutput[1];

    mixerInput[MIXER_IN_RC_ROLL]       = rcData[ROLL]       - rxConfig()->midrc;
    mixerInput[MIXER_IN_RC_PITCH]      = rcData[PITCH]      - rxConfig()->midrc;
    mixerInput[MIXER_IN_RC_YAW]        = rcData[YAW]        - rxConfig()->midrc;
    mixerInput[MIXER_IN_RC_THROTTLE]   = rcData[THROTTLE]   - rxConfig()->midrc;
    mixerInput[MIXER_IN_RC_COLLECTIVE] = rcData[COLLECTIVE] - rxConfig()->midrc;

    for (int i = 0; i < 8; i++)
        mixerInput[MIXER_IN_RC_AUX1+i] = rcData[AUX1+i] - rxConfig()->midrc;

    // Current cyclic deflection
    cyclicTotal = sqrt(mixerInput[MIXER_IN_STABILIZED_ROLL] * mixerInput[MIXER_IN_STABILIZED_ROLL] +
                       mixerInput[MIXER_IN_STABILIZED_PITCH] * mixerInput[MIXER_IN_STABILIZED_PITCH]);

    // Cyclic ring limit reached
    if (cyclicTotal > cyclicLimit) {
        mixerInput[MIXER_IN_STABILIZED_ROLL]  *= cyclicLimit / cyclicTotal;
        mixerInput[MIXER_IN_STABILIZED_PITCH] *= cyclicLimit / cyclicTotal;
    }

    // Input override
    if (!ARMING_FLAG(ARMED)) {
        for (int i = 0; i < MIXER_INPUT_COUNT; i++) {
            if (mixerOverride[i] != MIXER_OVERRIDE_OFF)
                mixerInput[i] = mixerOverride[i];
        }
    }

    // Reset outputs
    for (int i = 0; i < MIXER_OUTPUT_COUNT; i++) {
        mixerOutput[i] = 0;
    }
    
    // Calculate mixer outputs
    for (int i = 0; i < mixerRuleCount; i++) {
        int src = mixer[i].input;
        int dst = mixer[i].output;
        float val = constrain(mixer[i].offset + mixerInput[src] * mixer[i].rate, mixer[i].min, mixer[i].max) / 1000.0f;
        
        switch (mixer[i].oper)
        {
            case MIXER_OP_SET:
                mixerOutput[dst] = val;
                break;
            case MIXER_OP_ADD:
                mixerOutput[dst] += val;
                break;
            case MIXER_OP_MUL:
                mixerOutput[dst] *= val;
                break;
        }
    }
}


float mixerGetInput(uint8_t i)
{
    return mixerInput[i];
}

float mixerGetServoOutput(uint8_t i)
{
    return mixerOutput[i];
}

float mixerGetMotorOutput(uint8_t i)
{
    return (mixerOutput[i + MIXER_OUTPUT_MOTORS] + 1) / 2;
}

float getCyclicDeflection(void)
{
    return MIN(cyclicTotal / cyclicLimit, 1.0f);
}

