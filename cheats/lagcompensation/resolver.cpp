// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "animation_system.h"
#include "..\ragebot\aim.h"

void resolver::initialize(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch)
{
    player = e;
    player_record = record;

    original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
    original_pitch = math::normalize_pitch(pitch);
}
float NormalizeYaw(float yaw)
{
    if (yaw > 180)
        yaw -= (round(yaw / 360) * 360.f);
    else if (yaw < -180)
        yaw += (round(yaw / 360) * -360.f);

    return yaw;
}
float AngleDiff(float destAngle, float srcAngle) {
    float delta;

    delta = fmodf(destAngle - srcAngle, 360.0f);
    if (destAngle > srcAngle) {
        if (delta >= 180)
            delta -= 360;
    }
    else {
        if (delta <= -180)
            delta += 360;
    }
    return delta;
}
Vector CalcAngle(const Vector& vecSource, const Vector& vecDestination)
{
    Vector qAngles;
    Vector delta = Vector((vecSource[0] - vecDestination[0]), (vecSource[1] - vecDestination[1]), (vecSource[2] - vecDestination[2]));
    float hyp = sqrtf(delta[0] * delta[0] + delta[1] * delta[1]);
    qAngles[0] = (float)(atan(delta[2] / hyp) * (180.0f / M_PI));
    qAngles[1] = (float)(atan(delta[1] / delta[0]) * (180.0f / M_PI));
    qAngles[2] = 0.f;
    if (delta[0] >= 0.f)
        qAngles[1] += 180.f;

    return qAngles;
}
static auto GetSmoothedVelocity = [](float min_delta, Vector a, Vector b) {
    Vector delta = a - b;
    float delta_length = delta.Length();

    if (delta_length <= min_delta) {
        Vector result;
        if (-min_delta <= delta_length) {
            return a;
        }
        else {
            float iradius = 1.0f / (delta_length + FLT_EPSILON);
            return b - ((delta * iradius) * min_delta);
        }
    }
    else {
        float iradius = 1.0f / (delta_length + FLT_EPSILON);
        return b + ((delta * iradius) * min_delta);
    }
};
float ClampYaw(float y) {
    if (y > 180)
    {
        y -= (round(y / 360) * 360.f);
    }
    else if (y < -180)
    {
        y += (round(y / 360) * -360.f);
    }
    return y;
}
void resolver::reset()
{
    player = nullptr;
    player_record = nullptr;

    side = false;
    fake = false;

    was_first_bruteforce = false;
    was_second_bruteforce = false;

    original_goal_feet_yaw = 0.0f;
    original_pitch = 0.0f;
}

enum e_anim_layer {
    ANIMATION_LAYER_AIMMATRIX,
    ANIMATION_LAYER_WEAPON_ACTION,
    ANIMATION_LAYER_WEAPON_ACTION_RECROUCH,
    ANIMATION_LAYER_ADJUST,
    ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL,
    ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB,
    ANIMATION_LAYER_MOVEMENT_MOVE,
    ANIMATION_LAYER_MOVEMENT_STRAFECHANGE,
    ANIMATION_LAYER_WHOLE_BODY,
    ANIMATION_LAYER_FLASHED,
    ANIMATION_LAYER_FLINCH,
    ANIMATION_LAYER_ALIVELOOP,
    ANIMATION_LAYER_LEAN
};


bool freestand_target(player_t* target, float* yaw)
{
    float dmg_left = 0.f;
    float dmg_right = 0.f;

    static auto get_rotated_pos = [](Vector start, float rotation, float distance)
    {
        float rad = DEG2RAD(rotation);
        start.x += cos(rad) * distance;
        start.y += sin(rad) * distance;

        return start;
    };

    const auto local = g_ctx.local();

    if (!local || !target || !local->is_alive())
        return false;

    Vector local_eye_pos = target->get_shoot_position();
    Vector eye_pos = local->get_shoot_position();
    Vector angle = (local_eye_pos, eye_pos);

    auto backwards = target->m_angEyeAngles().y; // angle.y;

    Vector pos_left = get_rotated_pos(eye_pos, angle.y + 90.f, 40.f);
    Vector pos_right = get_rotated_pos(eye_pos, angle.y - 90.f, -40.f);

    const auto wall_left = (local_eye_pos, pos_left,
        nullptr, nullptr, local);

    const auto wall_right = (local_eye_pos, pos_right,
        nullptr, nullptr, local);



    if (dmg_left == 0.f && dmg_right == 0.f)
    {
        *yaw = backwards;
        return false;
    }

    // we can hit both sides, lets force backwards
    if (fabsf(dmg_left - dmg_right) < 10.f)
    {
        *yaw = backwards;
        return false;
    }

    bool direction = dmg_left > dmg_right;
    *yaw = direction ? angle.y - 90.f : angle.y + 90.f;

    return true;
}


void resolver::resolve_yaw()
{


    // lets be real this is the most p thing we ever seen
    float m_flResolveValue;
    int m_flResolveSide;
    AnimationLayer layers[15];
    AnimationLayer moveLayers[3][15];
    int m_flSide;
    bool m_bAnimatePlayer;
    bool m_bAnimsUpdated;
    bool m_bResolve;
    bool m_flPreviousDelta;
    // yee jarvis nanotechnology please.
   // ok sir...
  // fly mode activated

    if (player->is_alive() && !player->is_player())
    {
        if (!(player->m_fFlags() & FL_ONGROUND))
        {
            m_flResolveSide = 0;
        }
        auto standing = layers[3].m_flWeight == 0.0f && layers[3].m_flCycle == 0.0f;
        auto animating = layers[12].m_flWeight * 1000.f;
        auto moving = !animating && (layers[6].m_flWeight * 1000.f) == (layers[6].m_flWeight * 1000.f);

        float m_flSpeed = player->m_vecVelocity().Length2D();
        if (m_flSpeed > 1.1f)
        {
            if (moving)
            {
                float EyeYaw = fabs(layers[6].m_flPlaybackRate - moveLayers[0][6].m_flPlaybackRate);
                float Negative = fabs(layers[6].m_flPlaybackRate - moveLayers[2][6].m_flPlaybackRate);
                float Positive = fabs(layers[6].m_flPlaybackRate - moveLayers[1][6].m_flPlaybackRate);
                if (Positive > EyeYaw || Positive >= Negative || (Positive * 1000.0))
                {
                    if (EyeYaw >= Negative && Positive > Negative && !(Negative * 1000.0))
                    {
                        m_bAnimsUpdated = true;
                        m_bResolve = true;
                        m_flSide = 1;
                    }
                }
                else
                {
                    m_bAnimsUpdated = true;
                    m_bResolve = true;
                    m_flSide = -1;
                }
            }
        }
        else if (standing)
        {
            auto m_flEyeDelta = std::remainderf((player->m_angEyeAngles().y - player->m_flLowerBodyYawTarget()), 360.f) <= 0.f;
            if (2 * m_flEyeDelta)
            {
                if (2 * m_flEyeDelta == 2)
                {
                    m_flSide = -1;
                }
            }
            else
            {
                m_flSide = 1;
            }
            m_bResolve = true;
            m_flPreviousDelta = m_flEyeDelta;
        }
        m_flResolveValue = 58.f;
        player->get_animation_state()->m_flGoalFeetYaw = (player->m_angEyeAngles().y + m_flResolveValue * m_flSide);

    }


}


float ApproachAngle(float target, float value, float speed)
{
    target = (target * 182.04445f) * 0.0054931641f;
    value = (value * 182.04445f) * 0.0054931641f;

    float delta = target - value;

    // Speed is assumed to be positive
    if (speed < 0)
        speed = -speed;

    if (delta < -180.0f)
        delta += 360.0f;
    else if (delta > 180.0f)
        delta -= 360.0f;

    if (delta > speed)
        value += speed;
    else if (delta < -speed)
        value -= speed;
    else
        value = target;

    return value;
}
void  ResolverBurteforce(struct lag_record* m_pLagRecord, int m_iShotsMissed, float m_flGoalFeetYaw, float m_flYawModifier)
{
    float desync_delta; // xmm0_4
    float v4; // xmm1_4
    float v5; // xmm1_4
    float v6; // xmm1_4
    float v7; // xmm1_4
    float v8; // xmm1_4
    float v9; // xmm1_4

    desync_delta = m_flYawModifier * 58.0;
    switch (m_iShotsMissed)
    {
    case 1:
        v4 = m_flGoalFeetYaw + (desync_delta + desync_delta);
        if (v4 > 180.0 || v4 < -180.0)
            (unsigned int(v4 / 360.0) & FL_ONGROUND);
        break;
    case 2:
        v5 = m_flGoalFeetYaw + (desync_delta * 0.5);
        if (v5 > 180.0 || v5 < -180.0)
            (unsigned int(v5 / 360.0) & FL_ONGROUND);
        break;
    case 4:
        v6 = m_flGoalFeetYaw + (desync_delta * -0.5);
        if (v6 > 180.0 || v6 < -180.0)
            (unsigned int(v6 / 360.0) & FL_ONGROUND);
        break;
    case 5:
        v8 = m_flGoalFeetYaw - (desync_delta + desync_delta);
        if (v8 > 180.0 || v8 < -180.0)
            (unsigned int(v8 / 360.0) & FL_ONGROUND);
        break;
    case 7:
        v9 = m_flGoalFeetYaw + 120.0;
        if ((m_flGoalFeetYaw + 120.0) > 180.0 || v9 < -180.0)
            (unsigned int(v9 / 360.0) & FL_ONGROUND);
        break;
    case 8:
        v7 = m_flGoalFeetYaw + -120.0;
        if ((m_flGoalFeetYaw + -120.0) > 180.0 || v7 < -180.0)
            (unsigned int(v7 / 360.0) & FL_ONGROUND);
        break;
    default:
        return;
    }
}




void bruteforce(player_t* e, player_info_t player, const float& goal_feet_yaw, const float& pitch)
{
    player_info_t player_info;

    auto animState = player;
    auto& resolverInfo = player;
    // Rebuild setup velocity to receive flMinBodyYaw and flMaxBodyYaw
    Vector velocity = velocity;
    float spd;
    if (spd > std::powf(1.2f * 260.0f, 2.f)) {
        Vector velocity_normalized = velocity.Normalized();
        velocity = velocity_normalized * (1.2f * 260.0f);
    }
    float m_flChokedTime;
    float v25 = (0.0f, 1.0f);
    float v26 = (0.0f);
    float v27 = m_flChokedTime * 6.0f;
    float v28;

    // clamp
    if ((v25 - v26) <= v27) {
        if (-v27 <= (v25 - v26))
            v28 = v25;
        else
            v28 = v26 - v27;
    }
    else {
        v28 = v26 + v27;
    }
    float m_flFakeGoalFeetYaw;
    float flDuckAmount = (v28, 0.0f, 1.0f);

    Vector animationVelocity = (m_flChokedTime * 2000.0f, velocity, velocity);
    float speed = std::fminf(animationVelocity.Length(), 260.0f);

    bool weapon;

    float flMaxMovementSpeed = 260.0f;
    if (weapon) {
        flMaxMovementSpeed = std::fmaxf(weapon, 0.001f);
    }

    float flRunningSpeed = speed / (flMaxMovementSpeed * 0.520f);
    float flDuckingSpeed = speed / (flMaxMovementSpeed * 0.340f);

    flRunningSpeed = (flRunningSpeed, 0.0f, 1.0f);
    float m_flGroundFractio;
    float flYawModifier = (((m_flGroundFractio * -0.3f) - 0.2f) * flRunningSpeed) + 1.0f;
    if (flDuckAmount > 0.0f) {
        float flDuckingSpeed = (flDuckingSpeed, 0.0f, 1.0f);
        flYawModifier += (flDuckAmount * flDuckingSpeed) * (0.5f - flYawModifier);
    }
    float m_flMinBodyYaw;
    float m_flMaxBodyYaw;
    float flMinBodyYaw = std::fabsf(m_flMinBodyYaw * flYawModifier);
    float flMaxBodyYaw = std::fabsf(m_flMaxBodyYaw * flYawModifier);
    float m_angEyeAngles;
    float flEyeYaw = m_angEyeAngles, yaw;
    float flEyeDiff = std::remainderf(flEyeYaw - m_flFakeGoalFeetYaw, 360.f);

    if (flEyeDiff <= flMaxBodyYaw) {
        if (flMinBodyYaw > flEyeDiff)
            m_flFakeGoalFeetYaw = fabs(flMinBodyYaw) + flEyeYaw;
    }
    else {
        m_flFakeGoalFeetYaw = flEyeYaw - fabs(flMaxBodyYaw);
    }

    m_flFakeGoalFeetYaw = std::remainderf(m_flFakeGoalFeetYaw, 360.f);
    float m_flGroundFraction;
    if (speed > 0.1f || fabs(velocity.z) > 100.0f) {
        m_flFakeGoalFeetYaw = (ApproachAngle,
            flEyeYaw,
            m_flFakeGoalFeetYaw,
            ((m_flGroundFraction * 20.0f) + 30.0f)
            * m_flChokedTime);
    }
    else {
        m_flFakeGoalFeetYaw = (ApproachAngle,

            m_flFakeGoalFeetYaw,
            m_flChokedTime * 100.0f);
    }

    float Left = flEyeYaw - flMinBodyYaw;
    float Right = flEyeYaw + flMaxBodyYaw;

    float resolveYaw;
    int m_iMissedShots;
    switch (m_iMissedShots % 3) {
    case 0: // brute left side
        resolveYaw = Left;
        break;
    case 1: // brute fake side
        resolveYaw = m_flFakeGoalFeetYaw;
        break;
    case 2: // brute right side
        resolveYaw = Right;
        break;
    default:
        break;
    }
    return;
}

















bool IsAdjustingBalances(player_t* player, AnimationLayer* record, AnimationLayer* layer)
{
    AnimationLayer animationLayer[15];
    AnimationLayer m_iLayerCount;
    for (int i = 0; i; i++)
    {
        const int activity = player->sequence_activity(animationLayer[i].m_nSequence);
        if (activity == 979)
        {
            *layer = animationLayer[i];
            return true;
        }
    }
    return false;
}


void update_walk_data(player_t* e)
{
    float previous, m_previous;
    previous = m_previous;

    AnimationLayer anim_layers[15];
    bool s_1 = false,
        s_2 = false,
        s_3 = false;

    for (int i = 0; i < e->animlayer_count(); i++)
    {
        anim_layers[i] = e->get_animlayers()[i];
        if (anim_layers[i].m_nSequence == 26 && anim_layers[i].m_flWeight < 0.4f)
            s_1 = true;
        if (anim_layers[i].m_nSequence == 7 && anim_layers[i].m_flWeight > 0.001f)
            s_2 = true;
        if (anim_layers[i].m_nSequence == 2 && anim_layers[i].m_flWeight == 0)
            s_3 = true;
    }
    float  m_fakewalking;
    if (s_1 && s_2)
        if (s_3)
            m_fakewalking = true;
        else
            m_fakewalking = false;
    else
        m_fakewalking = false;
}


bool c_resolver::has_fake(player_t* entity)
{
    float  index = -1;
    float player_lag_record;

    if (player_lag_record < 2)
        return true;
    float interval_per_tick;
    if (fabs(player_lag_record - player_lag_record) == interval_per_tick)
        return false;

    return true;
}




bool InFakeWalkOld(player_t* player)
{
    bool
        bFakewalking = false,
        stage1 = false,			// stages needed cause we are iterating all layers, eitherwise won't work :)
        stage2 = false,
        stage3 = false;
    AnimationLayer animationLayer[15];
    for (int i = 0;  ; i++)
    {
        if (animationLayer[i].m_nSequence == 26 &&animationLayer[i].m_flWeight < 0.47f)
            stage1 = true;

        if (animationLayer[i].m_nSequence == 7 && animationLayer[i].m_flWeight > 0.001f)
            stage2 = true;

        if (animationLayer[i].m_nSequence == 2 && animationLayer[i].m_flWeight == 0)
            stage3 = true;
    }

    if (stage1 && stage2)
        if (stage3 || (player->m_fFlags() & FL_DUCKING)) // since weight from stage3 can be 0 aswell when crouching, we need this kind of check, cause you can fakewalk while crouching, thats why it's nested under stage1 and stage2
            bFakewalking = true;
        else
            bFakewalking = false;
    else
        bFakewalking = false;

    return bFakewalking;
}


/*

снизу не рабочая херня
спащено из движка сурс но нахер

*/

#define ANIMATIONLAYER_H
#ifdef _WIN32
#pragma once
#endif




class C_AnimationLayer
{
public:


    C_AnimationLayer();
    void Reset();

    void SetOrder(int order);

public:

    bool IsActive(void);


    float GetFadeout(float flCurTime);

    float	m_flLayerAnimtime;
    float	m_flLayerFadeOuttime;
};
#ifdef CLIENT_DLL
#define CAnimationLayer C_AnimationLayer
#endif


inline C_AnimationLayer::C_AnimationLayer()
{
    Reset();
}

inline void C_AnimationLayer::Reset()
{
 float    m_nSequence = 0;
 float    m_flPrevCycle = 0;
   float  m_flWeight = 0;
 float    m_flPlaybackRate = 0;
  float   m_flCycle = 0;
 float   m_flLayerAnimtime = 0;
  float  m_flLayerFadeOuttime = 0;
}


inline void C_AnimationLayer::SetOrder(int order)
{
   int  m_nOrder = order;
}

inline float C_AnimationLayer::GetFadeout(float flCurTime)
{
    float s;

    if (m_flLayerFadeOuttime <= 0.0f)
    {
        s = 0;
    }
    else
    {
        // blend in over 0.2 seconds
        s = 1.0 - (flCurTime - m_flLayerAnimtime) / m_flLayerFadeOuttime;
        if (s > 0 && s <= 1.0)
        {
            // do a nice spline curve
            s = 3 * s * s - 2 * s * s * s;
        }
        else if (s > 1.0f)
        {
            // Shouldn't happen, but maybe curtime is behind animtime?
            s = 1.0f;
        }
    }
    return s;
}



 float layermove()
{
     AnimationLayer m_Layer[15][2];
     float C_BaseAnimatingOverlay;
     (C_BaseAnimatingOverlay, m_Layer[0][2].m_nSequence, FIELD_INTEGER),
         (C_BaseAnimatingOverlay, m_Layer[0][2].m_flCycle, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[0][2].m_flPlaybackRate, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[0][2].m_flWeight, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[1][2].m_nSequence, FIELD_INTEGER),
         (C_BaseAnimatingOverlay, m_Layer[1][2].m_flCycle, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[1][2].m_flPlaybackRate, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[1][2].m_flWeight, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[2][2].m_nSequence, FIELD_INTEGER),
         (C_BaseAnimatingOverlay, m_Layer[2][2].m_flCycle, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[2][2].m_flPlaybackRate, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[2][2].m_flWeight, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[3][2].m_nSequence, FIELD_INTEGER),
         (C_BaseAnimatingOverlay, m_Layer[3][2].m_flCycle, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[3][2].m_flPlaybackRate, FIELD_FLOAT),
         (C_BaseAnimatingOverlay, m_Layer[3][2].m_flWeight, FIELD_FLOAT);
    }


 //самопислогика
 // овнед 
 /*
 
 
 +++++++++++++++++++++++++++++++++
 
 
 B1G PASTER

 
 ++++++++++++++++++++++++++++++++++++
 
 
 
 
 снизу не рабочая херня вронг + юзлесс
 
 */
 void resolver_()
 {

     int animstate;

     animstate = 128;
     
     if (animstate)
     {
     
         *(float*)(animstate + 0x80) = *(float*)(animstate + 128) -45.0;
     *(float*)(animstate + 0x80) = *(float*)(animstate + 128) + 45.0;
     *(float*)(animstate + 0x80) = *(float*)(animstate + 128) + 35.0;
     *(float*)(animstate + 0x80) = *(float*)(animstate + 128) -35.0;
     *(float*)(animstate + 0x80) = *(float*)(animstate + 128) + 60.0;
     *(float*)(animstate + 0x80) = *(float*)(animstate + 128) - 60.0;
     *(float*)(animstate + 0x80) = *(float*)(animstate + 128) + 30.0;
     *(float*)(animstate + 0x80) = *(float*)(animstate + 128) - 30.0;

     }
     float  max_yaw = 180;

     int yaw;
     if (yaw=180)
     {


         max_yaw;
      

     }

     
     if (yaw < 180)
     {
         max_yaw < 180;
     }
 }

float resolver::resolve_pitch()
{
    return original_pitch;
    return original_pitch = 0;
}