#include "../plugin_sdk/plugin_sdk.hpp"
#include "tryndamere.h"

namespace tryndamere
{
    // Define the colors that will be used in on_draw()
    //
#define W_DRAW_COLOR (MAKE_COLOR ( 227, 203, 20, 255 ))  //Red Green Blue Alpha
#define E_DRAW_COLOR (MAKE_COLOR ( 235, 12, 223, 255 ))  //Red Green Blue Alpha

// To declare a spell, it is necessary to create an object and registering it in load function
    script_spell* q = nullptr;
    script_spell* w = nullptr;
    script_spell* e = nullptr;
    script_spell* r = nullptr;

    // Declaration of menu objects
    TreeTab* main_tab = nullptr;

    namespace draw_settings
    {
        TreeEntry* draw_range_w = nullptr;
        TreeEntry* draw_range_e = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* q_myhero_hp_under = nullptr;
        TreeEntry* q_only_when_no_enemies_nearby = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_target_above_range = nullptr;
        TreeEntry* w_target_hp_under = nullptr;
        TreeEntry* w_only_when_e_ready = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* r_myhero_hp_under = nullptr;
        TreeEntry* r_only_when_enemies_nearby = nullptr;
    }

    namespace harass
    {
        TreeEntry* use_e = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* use_e = nullptr;
        TreeEntry* e_only_when_minions_more_than = nullptr;
    }

    namespace jungleclear
    {
        TreeEntry* use_e = nullptr;
    }

    namespace fleemode
    {
        TreeEntry* use_e;
    }


    // Event handler functions
    void on_update();
    void on_draw();
    void on_before_attack(game_object_script sender, bool* process);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void e_logic();
    bool r_logic();

    // Enum is used to define myhero region 
    enum Position
    {
        Line,
        Jungle
    };

    Position my_hero_region;

    void load()
    {
        // Registering a spells
        //
        q = plugin_sdk->register_spell(spellslot::q, 0);
        w = plugin_sdk->register_spell(spellslot::w, 850);
        e = plugin_sdk->register_spell(spellslot::e, 660);
        r = plugin_sdk->register_spell(spellslot::r, 0);


        // Create a menu according to the description in the "Menu Section"
        //
        main_tab = menu->create_tab("tryndamere", "Tryndamere");
        main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
        {
            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
            {
                combo::use_q = combo->add_checkbox(myhero->get_model() + ".comboUseQ", "Use Q", true);
                combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
                auto q_config = combo->add_tab(myhero->get_model() + ".comboQConfig", "Q Config");
                {
                    combo::q_myhero_hp_under = q_config->add_slider(myhero->get_model() + ".comboQMyheroHpUnder", "Myhero HP is under (in %)", 20, 0, 100);
                    combo::q_only_when_no_enemies_nearby = q_config->add_checkbox(myhero->get_model() + ".comboQOnlyWhenNoEnemiesNearby", "Only when no enemies are nearby", true);
                }
                combo::use_w = combo->add_checkbox(myhero->get_model() + ".comboUseW", "Use W on escaping enemies", true);
                auto w_config = combo->add_tab(myhero->get_model() + ".comboWConfig", "W Config");
                {
                    combo::w_target_above_range = w_config->add_slider(myhero->get_model() + ".comboWTargetAboveRange", "Target is above range", 500, 0, 800);
                    combo::w_target_hp_under = w_config->add_slider(myhero->get_model() + ".comboWTargetHpUnder", "Target HP is under (in %)", 80, 0, 100);
                    combo::w_only_when_e_ready = w_config->add_checkbox(myhero->get_model() + ".comboWOnlyWhenEReady", "Use W only when E is ready", true);
                }
                combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".comboUseE", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                combo::use_r = combo->add_checkbox(myhero->get_model() + ".comboUseR", "Use R", true);
                combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());
                auto r_config = combo->add_tab(myhero->get_model() + ".comboRConfig", "R Config");
                {
                    combo::r_myhero_hp_under = r_config->add_slider(myhero->get_model() + ".comboRMyheroHpUnder", "Myhero HP is under (in %)", 20, 0, 100);
                    combo::r_only_when_enemies_nearby = r_config->add_checkbox(myhero->get_model() + ".comboROnlyWhenEnemiesNearby", "Only when enemies are nearby", false);
                }
            }

            auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
            {
                harass::use_e = harass->add_checkbox(myhero->get_model() + ".harassUseE", "Use E", true);
                harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
                laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclearUseE", "Use E", true);
                laneclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto e_config = laneclear->add_tab(myhero->get_model() + ".comboEConfig", "E Config");
                {
                    laneclear::e_only_when_minions_more_than = e_config->add_slider(myhero->get_model() + ".laneclearUseEOnlyWhenMinionsMoreThan", "Use only when minions more than", 3, 0, 5);
                }
            }

            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
                jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclearrUseE", "Use E", true);
                jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }


            auto fleemode = main_tab->add_tab(myhero->get_model() + ".fleemode", "Flee Mode");
            {
                fleemode::use_e = fleemode->add_checkbox(myhero->get_model() + ".fleemodeUseE", "Use E to ran away", true);
                fleemode::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }

            auto draw_settings = main_tab->add_tab(myhero->get_model() + ".drawings", "Drawings Settings");
            {
                draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".drawingW", "Draw W range", true);
                draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".drawingE", "Draw E range", true);
                draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            }
        }

        // To add a new event you need to define a function and call add_calback
        //
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);
        event_handler<events::on_before_attack_orbwalker>::add_callback(on_before_attack);
    }

    void unload()
    {
        // Always remove all declared spells
        //
        plugin_sdk->remove_spell(q);
        plugin_sdk->remove_spell(w);
        plugin_sdk->remove_spell(e);
        plugin_sdk->remove_spell(r);

        // VERY important to remove always ALL events
        //
        event_handler<events::on_update>::remove_handler(on_update);
        event_handler<events::on_draw>::remove_handler(on_draw);
        event_handler<events::on_before_attack_orbwalker>::remove_handler(on_before_attack);
    }

    // Main update script function
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }

        if (!r_logic())
        {
            q_logic();
        }

        // Very important if can_move ( extra_windup ) 
        // Extra windup is the additional time you have to wait after the aa
        // Too small time can interrupt the attack
        if (orbwalker->can_move(0.05f))
        {
            //Checking if the user has combo_mode() (Default SPACE)
            if (orbwalker->combo_mode())
            {
                if (w->is_ready() && combo::use_w->get_bool())
                {
                    w_logic();
                }

                if (e->is_ready() && combo::use_e->get_bool())
                {
                    e_logic();
                }
            }

            //Checking if the user has selected harass() (Default C)
            if (orbwalker->harass())
            {
                if (e->is_ready() && harass::use_e->get_bool())
                {
                    // Get a target from a given range
                    auto target = target_selector->get_target(e->range(), damage_type::physical);

                    // Always check an object is not a nullptr!
                    if (target != nullptr)
                    {
                        if (!myhero->is_under_enemy_turret())
                        {
                            if (e->cast(target))
                            {
                                return;
                            }
                        }
                    }
                }
            }

            // Checking if the user has selected flee_mode() (Default Z)
            if (orbwalker->flee_mode())
            {
                if (q->is_ready() && fleemode::use_e->get_bool())
                {
                    if (e->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
                    {
                        return;
                    }
                }
            }

            // Checking if the user has selected lane_clear_mode() (Default V)
            if (orbwalker->lane_clear_mode())
            {

                // Gets enemy minions from the entitylist
                auto lane_minions = entitylist->get_enemy_minions();

                // Gets jugnle mobs from the entitylist
                auto monsters = entitylist->get_jugnle_mobs_minions();

                // You can use this function to delete minions that aren't in the specified range
                lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(e->range() + 300);
                    }), lane_minions.end());

                // You can use this function to delete monsters that aren't in the specified range
                monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
                    {
                        return !x->is_valid_target(e->range() + 300);
                    }), monsters.end());

                //std::sort -> sort monsters by max helth
                std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
                    {
                        return a->get_max_health() > b->get_max_health();
                    });

                // Set the correct region where myhero is
                if (!lane_minions.empty())
                {
                    my_hero_region = Position::Line;
                }
                else if (!monsters.empty())
                {
                    my_hero_region = Position::Jungle;
                }

                if (!lane_minions.empty() && lane_minions.size() >= laneclear::e_only_when_minions_more_than->get_int())
                {
                    if (e->is_ready() && laneclear::use_e->get_bool())
                    {
                        if (myhero->is_under_enemy_turret())
                        {
                            if (myhero->count_enemies_in_range(e->range()) == 0)
                            {
                                if (e->cast(lane_minions.at(0)))
                                {
                                    return;
                                }
                            }
                        }
                        else
                        {
                            if (e->cast(lane_minions.at(0)))
                            {
                                return;
                            }
                        }
                    }
                }


                if (!monsters.empty())
                {
                    // Logic responsible for monsters
                    if (e->is_ready() && jungleclear::use_e->get_bool())
                    {
                        if (e->cast(monsters.at(0)))
                            return;
                    }
                }
            }
        }
    }

#pragma region q_logic
    void q_logic()
    {
        if (q->is_ready() && combo::use_q->get_bool())
        {
            //debug for get tryndamere ult buff name
            //for (auto&& buff : myhero->get_bufflist())
            //{
            //    if (buff->is_valid() && buff->is_alive())
            //    {
            //        console->print("[ShadowAIO] [Debug] Buff name %s", buff->get_name_cstr());
            //    }
            //}
            if (!myhero->has_buff(buff_hash("UndyingRage")))
            {
                if (myhero->get_health_percent() < combo::q_myhero_hp_under->get_int())
                {
                    if (combo::r_only_when_enemies_nearby->get_bool() && myhero->count_enemies_in_range(850) == 0)
                    {
                        if (q->cast())
                        {
                            return;
                        }
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        if (w->is_ready() && combo::use_w->get_bool())
        {
            // Get a target from a given range
            auto target = target_selector->get_target(w->range(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr)
            {
                if (target->get_health_percent() < combo::w_target_hp_under->get_int())
                {
                    if (myhero->count_enemies_in_range(combo::w_target_above_range->get_int()) == 0)
                    {
                        if (w->cast())
                        {
                            return;
                        }
                    }
                }
            }
        }
    }
#pragma endregion

#pragma region e_logic
    void e_logic()
    {
        // Get a target from a given range
        auto target = target_selector->get_target(e->range(), damage_type::physical);

        // Always check an object is not a nullptr!
        if (target != nullptr)
        {
            if (combo::w_only_when_e_ready->get_bool() && !e->is_ready())
            {
                return;
            }
            e->cast(target);
        }
    }
#pragma endregion

#pragma region r_logic
    bool r_logic()
    {
        if (r->is_ready() && combo::use_r->get_bool())
        {
            if (!myhero->has_buff(buff_hash("UndyingRage")) && myhero->get_health_percent() < combo::r_myhero_hp_under->get_int())
            {
                if (combo::r_only_when_enemies_nearby->get_bool() && myhero->count_enemies_in_range(850) == 0)
                {
                    return false;
                }
                if (r->cast())
                {
                    return true;
                }
            }
        }
        return false;
    }
#pragma endregion

    void on_before_attack(game_object_script sender, bool* process)
    {
    }

    void on_draw()
    {

        if (myhero->is_dead())
        {
            return;
        }

        // Draw W range
        if (w->is_ready() && draw_settings::draw_range_w->get_bool())
            draw_manager->add_circle(myhero->get_position(), w->range(), W_DRAW_COLOR);

        // Draw E range
        if (e->is_ready() && draw_settings::draw_range_e->get_bool())
            draw_manager->add_circle(myhero->get_position(), e->range(), E_DRAW_COLOR);
    }
};