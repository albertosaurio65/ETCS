#include "speed_profile.h"
#include "targets.h"
#include "train_data.h"
#include "../MA/movement_authority.h"
#include "fixed_values.h"
#include "national_values.h"
#include <vector>
#include <map>
#include <list>
#include <cmath>
static std::map<distance,double> MRSP;
static std::set<speed_restriction> SSP;
static std::list<TSR> TSRs;
std::optional<speed_restriction> train_speed;
std::optional<speed_restriction> SR_speed;
static std::map<distance, double> gradient;
void delete_back_info()
{
    const distance mindist = d_minsafefront(0)-L_TRAIN-D_keep_information; //For unlinked balise groups, change this, losing efficiency
    for (auto it = SSP.lower_bound(speed_restriction(0,mindist,mindist,false)); it!=SSP.begin(); --it) {
        auto prev = it;
        --prev;
        if (prev->get_end()<mindist) {
            SSP.erase(SSP.begin(), it);
            break;
        }
    }
    {
        auto it = gradient.upper_bound(mindist);
        if (it != gradient.begin())
            gradient.erase(gradient.begin(), --it);
    }
    TSRs.remove_if([mindist](TSR t) {
        return t.restriction.get_end()<mindist;
    });
}
void recalculate_MRSP()
{
    delete_back_info();
    MRSP.clear();
    std::set<speed_restriction> restrictions;
    restrictions.insert(SSP.begin(), SSP.end());
    for (auto it=TSRs.begin(); it!=TSRs.end(); ++it)
        restrictions.insert(it->restriction);
    if (train_speed)
        restrictions.insert(*train_speed);
    if (SR_speed)
        restrictions.insert(*SR_speed);
    if (MA)
        restrictions.insert(signal_speeds.begin(), signal_speeds.end());
    std::set<distance> critical_points;
    for (auto it = restrictions.begin(); it != restrictions.end(); ++it) {
        critical_points.insert(it->get_start());
        critical_points.insert(it->get_end());
    }
    for (auto it = critical_points.begin(); it != --critical_points.end(); ++it) {
        double spd=400;
        for (auto it2 = restrictions.begin(); it2 != restrictions.end(); ++it2) {
            if (it2->get_start()<=*it && it2->get_end()>*it && it2->get_speed()<spd)
                spd = it2->get_speed();
        }
        if (MRSP.size()==0 || (--MRSP.upper_bound(*it))->second!=spd)
            MRSP[*it] = spd;
    }
    set_supervised_targets();
}
std::map<distance,double> get_MRSP()
{
    return MRSP;
}
void update_SSP(std::vector<SSP_element> nSSP)
{
    std::set<speed_restriction> rest;
    for (auto it=nSSP.begin(); it!=--nSSP.end(); ++it) {
        auto next = it;
        next++;
        rest.insert(speed_restriction(it->get_speed(cant_deficiency,other_train_categories), it->start, next->start, true));
    }
    auto it_start = SSP.lower_bound(*rest.begin());
    SSP.erase(it_start, SSP.end());
    SSP.insert(rest.begin(), rest.end());
    recalculate_MRSP();
}
std::set<speed_restriction> get_SSP()
{
    return SSP;
}
void update_gradient(std::map<distance, double> grad)
{
    auto it_start = gradient.lower_bound(grad.begin()->first);
    gradient.erase(it_start, gradient.end());
    gradient.insert(grad.begin(), grad.end());
}
std::map<distance, double> get_gradient()
{
    return gradient;
}
void set_train_max_speed(double vel)
{
    train_speed = speed_restriction(V_train, ::distance(std::numeric_limits<double>::lowest()), ::distance(std::numeric_limits<double>::max()), false);
    recalculate_MRSP();
}
void insert_TSR(TSR rest)
{
    revoke_TSR(rest.id);
    TSRs.push_back(rest);
    recalculate_MRSP();
}
void revoke_TSR(int id_tsr)
{
    std::list<std::list<TSR>::iterator> revocable;
    for (auto it=TSRs.begin(); it!=TSRs.end(); ++it) {
        if (it->id == id_tsr && it->revocable) {
            revocable.push_back(it);
        }
    }
    for (auto it : revocable) {
        TSRs.erase(it);
    }
    recalculate_MRSP();
}
speed_restriction get_PBD_restriction(double d_PBD, distance start, distance end, bool EB, double g)
{
    double V_pbd=0;
    double V_test = 1;
    double V_max_appr = std::max(std::sqrt(2*2*d_PBD),600/3.6);
    PBD_target pbd_ebd(distance(d_PBD), true, g);
    if (EB) {
        while (V_test<V_max_appr) {
            double dvebi = dV_ebi(V_test);
            double V_delta0PBD = Q_NVINHSMICPERM ? 0 : (V_test+dvebi)*0.008;
            double D_bec = (V_test+dvebi+V_delta0PBD)*(pbd_ebd.T_traction+pbd_ebd.T_berem);
            double d_offset=L_antenna_front+0.001*(V_test+dvebi+V_delta0PBD);
            double V_ebd = pbd_ebd.get_speed_curve(distance(d_offset+D_bec));
            if (std::abs((V_test+dvebi)-(V_ebd-V_delta0PBD))<=1.0/3.6 && d_offset+D_bec<d_PBD) {
                V_pbd = V_test;
                break;
            }
            V_test+=0.5;
        }
    } else {

    }
    return speed_restriction((((int)(V_pbd*3.6))/5)*5/3.6, start, end, false);
}