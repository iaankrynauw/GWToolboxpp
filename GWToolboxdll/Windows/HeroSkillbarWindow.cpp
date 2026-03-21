#include "stdafx.h"

#include <GWCA/Constants/Constants.h>
#include <GWCA/Constants/Skills.h>

#include <GWCA/GameEntities/Agent.h>
#include <GWCA/GameEntities/Party.h>
#include <GWCA/GameEntities/Skill.h>

#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/EffectMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>
#include <GWCA/Managers/MapMgr.h>
#include <GWCA/Managers/PartyMgr.h>
#include <GWCA/Managers/SkillbarMgr.h>
#include <GWCA/Managers/StoCMgr.h>
#include <GWCA/Managers/UIMgr.h>
#include <GWCA/Packets/StoC.h>

#include <Modules/Resources.h>
#include <ImGuiAddons.h>
#include <Timer.h>
#include <Windows/HeroSkillbarWindow.h>

namespace {

    // ---- Settings ----
    int   medium_threshold    = 5000;
    int   short_threshold     = 2500;
    Color color_long          = Colors::ARGB(50, 0, 255, 0);
    Color color_medium        = Colors::ARGB(50, 255, 255, 0);
    Color color_short         = Colors::ARGB(80, 255, 0, 0);
    Color color_border        = Colors::ARGB(100, 255, 255, 255);
    Color color_text_recharge = Colors::White();
    Color color_text_stats    = Colors::ARGB(200, 200, 200, 200);
    Color color_cast_bar      = Colors::ARGB(200, 55, 153, 30);
    Color color_hp_bar        = Colors::ARGB(200, 200, 30, 30);
    Color color_energy_bar    = Colors::ARGB(200, 30, 100, 200);
    bool  display_skill_overlay = true;
    bool  show_cast_bar         = true;
    bool  show_hp_bar           = true;
    bool  show_energy_bar       = true;
    bool  show_effects          = true;
    bool  show_skill_stats      = true;
    float effect_icon_ratio    = 0.45f;
    float bar_height            = 4.f;

    // ---- Data ----
    constexpr size_t MAX_HEROES    = 7;
    constexpr size_t SKILLS_PER_HERO = 8;

    struct CastState {
        GW::Constants::SkillID skill_id = GW::Constants::SkillID::No_Skill;
        clock_t start_time = 0;
        float   duration   = 0.f;
    };

    struct SkillState {
        char  cooldown[16]{};
        Color overlay_color{};
        uint32_t use_count = 0;
    };

    struct HeroRow {
        uint32_t agent_id   = 0;
        uint32_t hero_index = 0;
        GW::Constants::SkillID skill_ids[SKILLS_PER_HERO]{};
        SkillState skills[SKILLS_PER_HERO]{};
        CastState  cast{};
        float hp     = 0.f;
        float energy = 0.f;
        uint32_t total_skills_used = 0;
        bool valid = false;
    };

    HeroRow hero_rows[MAX_HEROES]{};
    size_t  hero_count    = 0;
    bool    pending_refresh = true;

    GW::HookEntry MapLoaded_Entry;
    GW::HookEntry PartyHeroAdd_Entry;
    GW::HookEntry UIMessage_Entry;
    GW::HookEntry GenericValueSelf_Entry;
    GW::HookEntry GenericValueTarget_Entry;

    // ---- Helpers ----

    void cooldown_to_string(char arr[16], uint32_t cd)
    {
        if (cd == 0 || cd > 1'800'000u) { arr[0] = 0; return; }
        if (cd >= 600u) snprintf(arr, 16, "%u", (cd + 999) / 1000);
        else            snprintf(arr, 16, "%.1f", cd / 1000.f);
    }

    Color uptime_to_color(uint32_t uptime)
    {
        if (uptime > static_cast<uint32_t>(medium_threshold)) return color_long;
        if (uptime > static_cast<uint32_t>(short_threshold)) {
            const float diff = static_cast<float>(medium_threshold - short_threshold);
            const float frac = 1.f - static_cast<float>(medium_threshold - uptime) / diff;
            int a[4], b[4], out[4];
            Colors::ConvertU32ToInt4(color_long, a);
            Colors::ConvertU32ToInt4(color_medium, b);
            for (int i = 0; i < 4; i++) out[i] = static_cast<int>((1.f - frac) * b[i] + frac * a[i]);
            return Colors::ConvertInt4ToU32(out);
        }
        if (uptime > 0) {
            const float frac = uptime / static_cast<float>(short_threshold);
            int a[4], b[4], out[4];
            Colors::ConvertU32ToInt4(color_medium, a);
            Colors::ConvertU32ToInt4(color_short, b);
            for (int i = 0; i < 4; i++) out[i] = static_cast<int>((1.f - frac) * b[i] + frac * a[i]);
            return Colors::ConvertInt4ToU32(out);
        }
        return 0x00000000;
    }

    uint32_t get_longest_effect_remaining(uint32_t agent_id, GW::Constants::SkillID skill_id)
    {
        const GW::EffectArray* effects = GW::Effects::GetAgentEffects(agent_id);
        if (!effects) return 0;
        const GW::Skill* sd = GW::SkillbarMgr::GetSkillConstantData(skill_id);
        if (sd && sd->type == GW::Constants::SkillType::Hex) return 0;
        uint32_t best = 0;
        for (const GW::Effect& effect : *effects) {
            if (effect.skill_id == skill_id) {
                const uint32_t remaining = effect.GetTimeRemaining();
                if (remaining > best) best = remaining;
            }
        }
        return best;
    }

    HeroRow* GetHeroRowByAgentId(uint32_t agent_id)
    {
        for (size_t i = 0; i < hero_count; i++) {
            if (hero_rows[i].agent_id == agent_id) return &hero_rows[i];
        }
        return nullptr;
    }

    bool RefreshHeroRows()
    {
        if (!GW::PartyMgr::GetIsPartyLoaded() || !GW::Map::GetIsMapLoaded()) return false;
        const GW::PartyInfo* info = GW::PartyMgr::GetPartyInfo();
        if (!info || !info->heroes.valid()) return false;
        const GW::AgentLiving* player = GW::Agents::GetControlledCharacter();
        if (!player) return false;

        hero_count = 0;
        for (const GW::HeroPartyMember& hero : info->heroes) {
            if (hero_count >= MAX_HEROES) break;
            if (hero.owner_player_id != player->player_number) continue;
            if (!GW::Agents::GetAgentEncName(hero.agent_id)) return false;

            HeroRow& row = hero_rows[hero_count];
            row.agent_id   = hero.agent_id;
            row.hero_index = hero_count;
            row.valid      = false;

            const GW::Skillbar* skillbar = GW::SkillbarMgr::GetSkillbar(hero.agent_id);
            if (skillbar && skillbar->IsValid()) {
                for (size_t s = 0; s < SKILLS_PER_HERO; s++)
                    row.skill_ids[s] = skillbar->skills[s].skill_id;
                row.valid = true;
            }
            hero_count++;
        }
        return true;
    }

    void OnUIMessage(GW::HookStatus*, GW::UI::UIMessage msg_id, void* wparam, void*)
    {
        if (msg_id != GW::UI::UIMessage::kAgentSkillStartedCast) return;
        const auto* pkt = static_cast<GW::UI::UIPacket::kAgentSkillStartedCast*>(wparam);
        if (!pkt) return;
        HeroRow* row = GetHeroRowByAgentId(pkt->agent_id);
        if (!row) return;
        row->cast.skill_id  = pkt->skill_id;
        row->cast.start_time = TIMER_INIT();
        row->cast.duration  = pkt->duration;
    }

    void SkillCallback(uint32_t value_id, uint32_t caster_id, uint32_t target_id,
                       uint32_t value, bool no_target)
    {
        uint32_t agent_id = caster_id;
        switch (value_id) {
            case GW::Packet::StoC::GenericValueID::instant_skill_activated:
            case GW::Packet::StoC::GenericValueID::skill_activated:
            case GW::Packet::StoC::GenericValueID::skill_finished:
            case GW::Packet::StoC::GenericValueID::attack_skill_activated:
            case GW::Packet::StoC::GenericValueID::attack_skill_finished:
                if (!no_target) agent_id = target_id;
                break;
            default: return;
        }
        if (value == 0) return;
        HeroRow* row = GetHeroRowByAgentId(agent_id);
        if (!row) return;
        const auto skill_id = static_cast<GW::Constants::SkillID>(value);
        for (size_t s = 0; s < SKILLS_PER_HERO; s++) {
            if (row->skill_ids[s] == skill_id) {
                row->skills[s].use_count++;
                row->total_skills_used++;
                return;
            }
        }
    }

    void MapLoadedCallback(GW::HookStatus*, GW::Packet::StoC::MapLoaded*)
    {
        pending_refresh = true;
        // Reset skill usage stats on map change
        for (auto& row : hero_rows) {
            row.total_skills_used = 0;
            for (auto& skill : row.skills) skill.use_count = 0;
        }
        hero_count = 0;
    }

    void PartyHeroAddCallback(GW::HookStatus*, GW::Packet::StoC::PartyHeroAdd*)
    {
        pending_refresh = true;
    }

} // namespace

void HeroSkillbarWindow::Initialize()
{
    ToolboxWindow::Initialize();
    GW::StoC::RegisterPostPacketCallback<GW::Packet::StoC::MapLoaded>(&MapLoaded_Entry, MapLoadedCallback);
    GW::StoC::RegisterPostPacketCallback<GW::Packet::StoC::PartyHeroAdd>(&PartyHeroAdd_Entry, PartyHeroAddCallback);
    GW::UI::RegisterUIMessageCallback(&UIMessage_Entry, GW::UI::UIMessage::kAgentSkillStartedCast, OnUIMessage);

    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValue>(
        &GenericValueSelf_Entry, [](const GW::HookStatus*, const GW::Packet::StoC::GenericValue* pkt) {
            SkillCallback(pkt->value_id, pkt->agent_id, 0, pkt->value, true);
        });
    GW::StoC::RegisterPacketCallback<GW::Packet::StoC::GenericValueTarget>(
        &GenericValueTarget_Entry, [](const GW::HookStatus*, const GW::Packet::StoC::GenericValueTarget* pkt) {
            SkillCallback(pkt->Value_id, pkt->caster, pkt->target, pkt->value, false);
        });
}

void HeroSkillbarWindow::Terminate()
{
    ToolboxWindow::Terminate();
    GW::StoC::RemoveCallback<GW::Packet::StoC::MapLoaded>(&MapLoaded_Entry);
    GW::StoC::RemoveCallback<GW::Packet::StoC::PartyHeroAdd>(&PartyHeroAdd_Entry);
    GW::UI::RemoveUIMessageCallback(&UIMessage_Entry);
    GW::StoC::RemoveCallback<GW::Packet::StoC::GenericValue>(&GenericValueSelf_Entry);
    GW::StoC::RemoveCallback<GW::Packet::StoC::GenericValueTarget>(&GenericValueTarget_Entry);
}

void HeroSkillbarWindow::Update(float)
{
    if (pending_refresh && RefreshHeroRows())
        pending_refresh = false;

    if (GW::Map::GetInstanceType() == GW::Constants::InstanceType::Loading) return;

    for (size_t h = 0; h < hero_count; h++) {
        HeroRow& row = hero_rows[h];
        if (!row.valid) continue;
        const GW::Skillbar* skillbar = GW::SkillbarMgr::GetSkillbar(row.agent_id);
        if (!skillbar || !skillbar->IsValid()) continue;

        for (size_t s = 0; s < SKILLS_PER_HERO; s++)
            row.skill_ids[s] = skillbar->skills[s].skill_id;

        for (size_t s = 0; s < SKILLS_PER_HERO; s++) {
            const uint32_t recharge = skillbar->skills[s].GetRecharge();
            cooldown_to_string(row.skills[s].cooldown, recharge);
            if (display_skill_overlay) {
                const uint32_t remaining = get_longest_effect_remaining(row.agent_id, row.skill_ids[s]);
                row.skills[s].overlay_color = uptime_to_color(remaining);
            } else {
                row.skills[s].overlay_color = 0;
            }
            if (recharge > 0 && !(row.skills[s].overlay_color & IM_COL32_A_MASK))
                row.skills[s].overlay_color = Colors::ARGB(80, 0, 0, 0);
        }

        if (const GW::MapAgent* ma = GW::Agents::GetMapAgentByID(row.agent_id)) {
            row.hp     = ma->max_health > 0.f ? std::clamp(ma->cur_health / ma->max_health, 0.f, 1.f) : 0.f;
            row.energy = ma->max_energy > 0.f ? std::clamp(ma->cur_energy / ma->max_energy, 0.f, 1.f) : 0.f;
        }

        const GW::Agent* raw_agent = GW::Agents::GetAgentByID(row.agent_id);
        const GW::AgentLiving* agent = raw_agent ? raw_agent->GetAsAgentLiving() : nullptr;
        if (agent) {
            const auto current_skill = static_cast<GW::Constants::SkillID>(agent->skill);
            if (current_skill != GW::Constants::SkillID::No_Skill) {
                if (row.cast.skill_id != current_skill) {
                    row.cast.skill_id   = current_skill;
                    row.cast.start_time = TIMER_INIT();
                    const GW::Skill* sd = GW::SkillbarMgr::GetSkillConstantData(current_skill);
                    row.cast.duration   = (sd && sd->activation > 0.f) ? sd->activation : 0.25f;
                }
            } else {
                row.cast = {};
            }
        }
    }
}

void HeroSkillbarWindow::Draw(IDirect3DDevice9*)
{
    if (!visible || !GW::Map::GetIsMapLoaded()) return;

    const float  spacing  = ImGui::GetStyle().ItemSpacing.x;
    const float  bar_h    = bar_height;

    ImGui::SetNextWindowCenter(ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400.f, 350.f), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags() & ~ImGuiWindowFlags_AlwaysAutoResize)) { ImGui::End(); return; }

    // Derive square icon size from available window width
    const float avail_w   = ImGui::GetContentRegionAvail().x;
    const float icon_side = std::max(8.f, (avail_w - spacing * (SKILLS_PER_HERO - 1)) / SKILLS_PER_HERO);
    const ImVec2 icon_sz  = {icon_side, icon_side};

    if (hero_count == 0) {
        ImGui::TextDisabled("No heroes in party");
        ImGui::End();
        return;
    }

    auto* draw_list = ImGui::GetWindowDrawList();

    for (size_t h = 0; h < hero_count; h++) {
        HeroRow& row = hero_rows[h];
        ImGui::PushID(static_cast<int>(h));

        if (!row.valid) {
            ImGui::TextDisabled("(loading)");
            ImGui::PopID();
            continue;
        }

        for (size_t s = 0; s < SKILLS_PER_HERO; s++) {
            const GW::Constants::SkillID skill_id = row.skill_ids[s];
            ImGui::PushID(static_cast<int>(s));

            if (skill_id == GW::Constants::SkillID::No_Skill) {
                ImGui::Dummy(icon_sz);
            }
            else {
                const ImVec2 pos = ImGui::GetCursorScreenPos();
                auto* tex_ptr = Resources::GetSkillImage(skill_id);
                IDirect3DTexture9* tex = tex_ptr ? *tex_ptr : nullptr;
                if (tex) ImGui::Image(tex, icon_sz);
                else     ImGui::Dummy(icon_sz);

                // Casting highlight
                if (row.cast.skill_id == skill_id && row.cast.duration > 0.f) {
                    const float progress = std::clamp(TIMER_DIFF(row.cast.start_time) / 1000.f / row.cast.duration, 0.f, 1.f);
                    const ImVec2 fill_tl = {pos.x, pos.y + icon_sz.y * (1.f - progress)};
                    draw_list->AddRectFilled(fill_tl, {pos.x + icon_sz.x, pos.y + icon_sz.y}, Colors::ARGB(80, 55, 153, 30));
                }

                // Cooldown overlay
                const Color& oc = row.skills[s].overlay_color;
                if (oc & IM_COL32_A_MASK)
                    draw_list->AddRectFilled(pos, {pos.x + icon_sz.x, pos.y + icon_sz.y}, oc);

                draw_list->AddRect(pos, {pos.x + icon_sz.x, pos.y + icon_sz.y}, color_border);

                // Cooldown text (center)
                if (row.skills[s].cooldown[0]) {
                    const ImVec2 tsz = ImGui::CalcTextSize(row.skills[s].cooldown);
                    draw_list->AddText({pos.x + icon_sz.x / 2.f - tsz.x / 2.f, pos.y + icon_sz.y / 2.f - tsz.y / 2.f},
                                       color_text_recharge, row.skills[s].cooldown);
                }

                // Skill usage stats overlays
                if (show_skill_stats && row.skills[s].use_count > 0) {
                    // Count (bottom-right)
                    char count_str[16];
                    snprintf(count_str, sizeof(count_str), "%u", row.skills[s].use_count);
                    const ImVec2 csz = ImGui::CalcTextSize(count_str);
                    draw_list->AddText({pos.x + icon_sz.x - csz.x - 2.f, pos.y + icon_sz.y - csz.y - 1.f},
                                       color_text_stats, count_str);

                    // Percentage (top-left)
                    if (row.total_skills_used > 0) {
                        const float pct = static_cast<float>(row.skills[s].use_count) / static_cast<float>(row.total_skills_used) * 100.f;
                        char pct_str[16];
                        snprintf(pct_str, sizeof(pct_str), "%.0f%%", pct);
                        draw_list->AddText({pos.x + 2.f, pos.y + 1.f}, color_text_stats, pct_str);
                    }
                }

                if (ImGui::IsItemHovered()) {
                    const GW::Skill* sd = GW::SkillbarMgr::GetSkillConstantData(skill_id);
                    if (sd) ImGui::SetTooltip("Slot %u", static_cast<uint32_t>(s + 1));
                }
                if (ImGui::IsItemClicked()) {
                    const auto action = static_cast<GW::UI::ControlAction>(
                        static_cast<uint32_t>(GW::UI::ControlAction_Hero1Skill1) + row.hero_index * 8 + s);
                    GW::GameThread::Enqueue([action] { GW::UI::Keypress(action); });
                }
            }

            if (s < SKILLS_PER_HERO - 1) ImGui::SameLine();
            ImGui::PopID();
        }

        // Bars
        const float bar_w = avail_w;

        if (show_hp_bar) {
            const ImVec2 bar_pos = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(bar_pos, {bar_pos.x + bar_w, bar_pos.y + bar_h}, Colors::ARGB(80, 0, 0, 0));
            draw_list->AddRectFilled(bar_pos, {bar_pos.x + bar_w * row.hp, bar_pos.y + bar_h}, color_hp_bar);
            ImGui::Dummy({bar_w, bar_h});
        }

        if (show_energy_bar) {
            const ImVec2 bar_pos = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(bar_pos, {bar_pos.x + bar_w, bar_pos.y + bar_h}, Colors::ARGB(80, 0, 0, 0));
            draw_list->AddRectFilled(bar_pos, {bar_pos.x + bar_w * row.energy, bar_pos.y + bar_h}, color_energy_bar);
            ImGui::Dummy({bar_w, bar_h});
        }

        if (show_cast_bar) {
            const ImVec2 bar_pos = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(bar_pos, {bar_pos.x + bar_w, bar_pos.y + bar_h}, Colors::ARGB(80, 0, 0, 0));
            if (row.cast.duration > 0.f) {
                const float progress = std::clamp(TIMER_DIFF(row.cast.start_time) / 1000.f / row.cast.duration, 0.f, 1.f);
                draw_list->AddRectFilled(bar_pos, {bar_pos.x + bar_w * progress, bar_pos.y + bar_h}, color_cast_bar);
            }
            ImGui::Dummy({bar_w, bar_h});
        }

        // Effect/buff icons - DISABLED FOR DEBUGGING
        if (show_effects) {
            const float eff_side = icon_side * effect_icon_ratio;
            const ImVec2 eff_sz = {eff_side, eff_side};
            const GW::AgentEffects* ae = GW::Effects::GetAgentEffectsArray(row.agent_id);

            auto draw_effect_icon = [&](GW::Constants::SkillID sid, float duration, uint32_t time_remaining_ms) {
                if (sid == GW::Constants::SkillID::No_Skill) return;
                auto* etex_ptr = Resources::GetSkillImage(sid);
                IDirect3DTexture9* etex = etex_ptr ? *etex_ptr : nullptr;
                const ImVec2 epos = ImGui::GetCursorScreenPos();
                if (etex) ImGui::Image(etex, eff_sz);
                else      ImGui::Dummy(eff_sz);
                if (duration > 0.f && time_remaining_ms > 0) {
                    const float prog = std::clamp(time_remaining_ms / 1000.f / duration, 0.f, 1.f);
                    draw_list->AddRectFilled({epos.x, epos.y + eff_sz.y - 3.f},
                                             {epos.x + eff_sz.x * prog, epos.y + eff_sz.y}, color_cast_bar);
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%.1fs", time_remaining_ms / 1000.f);
                }
                ImGui::SameLine();
            };

            if (ae) {
                if (ae->buffs.valid())
                    for (const GW::Buff& buff : ae->buffs)
                        draw_effect_icon(buff.skill_id, 0.f, 0);
                if (ae->effects.valid())
                    for (const GW::Effect& effect : ae->effects)
                        draw_effect_icon(effect.skill_id, effect.duration, effect.GetTimeRemaining());
            }
            ImGui::NewLine();
        }

        ImGui::PopID();
    }

    ImGui::End();
}

void HeroSkillbarWindow::LoadSettings(ToolboxIni* ini)
{
    ToolboxWindow::LoadSettings(ini);
    LOAD_BOOL(display_skill_overlay);
    LOAD_BOOL(show_cast_bar);
    LOAD_BOOL(show_hp_bar);
    LOAD_BOOL(show_energy_bar);
    LOAD_BOOL(show_effects);
    LOAD_BOOL(show_skill_stats);
    LOAD_FLOAT(effect_icon_ratio);
    LOAD_FLOAT(bar_height);
    LOAD_UINT(medium_threshold);
    LOAD_UINT(short_threshold);
    LOAD_COLOR(color_long);
    LOAD_COLOR(color_medium);
    LOAD_COLOR(color_short);
    LOAD_COLOR(color_border);
    LOAD_COLOR(color_text_recharge);
    LOAD_COLOR(color_text_stats);
    LOAD_COLOR(color_cast_bar);
    LOAD_COLOR(color_hp_bar);
    LOAD_COLOR(color_energy_bar);
}

void HeroSkillbarWindow::SaveSettings(ToolboxIni* ini)
{
    ToolboxWindow::SaveSettings(ini);
    SAVE_BOOL(display_skill_overlay);
    SAVE_BOOL(show_cast_bar);
    SAVE_BOOL(show_hp_bar);
    SAVE_BOOL(show_energy_bar);
    SAVE_BOOL(show_effects);
    SAVE_BOOL(show_skill_stats);
    SAVE_FLOAT(effect_icon_ratio);
    SAVE_FLOAT(bar_height);
    SAVE_UINT(medium_threshold);
    SAVE_UINT(short_threshold);
    SAVE_COLOR(color_long);
    SAVE_COLOR(color_medium);
    SAVE_COLOR(color_short);
    SAVE_COLOR(color_border);
    SAVE_COLOR(color_text_recharge);
    SAVE_COLOR(color_text_stats);
    SAVE_COLOR(color_cast_bar);
    SAVE_COLOR(color_hp_bar);
    SAVE_COLOR(color_energy_bar);
}

void HeroSkillbarWindow::DrawSettingsInternal()
{
    ImGui::DragFloat("Bar height", &bar_height, 0.5f, 2.f, 16.f, "%.f px");
    ImGui::Checkbox("Show HP bar", &show_hp_bar);
    if (show_hp_bar) { ImGui::SameLine(); Colors::DrawSettingHueWheel("HP color", &color_hp_bar); }
    ImGui::Checkbox("Show energy bar", &show_energy_bar);
    if (show_energy_bar) { ImGui::SameLine(); Colors::DrawSettingHueWheel("Energy color", &color_energy_bar); }
    ImGui::Checkbox("Show cast bar", &show_cast_bar);
    if (show_cast_bar) { ImGui::SameLine(); Colors::DrawSettingHueWheel("Cast color", &color_cast_bar); }
    ImGui::Checkbox("Show hero effects", &show_effects);
    if (show_effects) { ImGui::SameLine(); ImGui::DragFloat("Effect icon ratio", &effect_icon_ratio, 0.01f, 0.2f, 0.8f, "%.2f"); }
    ImGui::Checkbox("Show skill usage stats", &show_skill_stats);
    if (show_skill_stats) { ImGui::SameLine(); Colors::DrawSettingHueWheel("Stats text color", &color_text_stats); }
    ImGui::Checkbox("Paint skills according to effect duration", &display_skill_overlay);
    Colors::DrawSettingHueWheel("Border color", &color_border);
    Colors::DrawSettingHueWheel("Cooldown text color", &color_text_recharge);
}
