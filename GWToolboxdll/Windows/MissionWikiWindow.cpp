#include "stdafx.h"

#include <GWCA/Constants/Constants.h>
#include <GWCA/GameEntities/Map.h>
#include <GWCA/Managers/MapMgr.h>

#include <Modules/Resources.h>
#include <Utils/GuiUtils.h>
#include <Utils/TextUtils.h>
#include <Utils/FontLoader.h>

#include "MissionWikiWindow.h"

namespace {
    std::string strip_html_tags(std::string html)
    {
        while (true) {
            const auto start = html.find('<');
            if (start == std::string::npos) break;
            const auto end = html.find('>', start);
            if (end == std::string::npos) break;
            html.erase(start, (end + 1) - start);
        }
        return html;
    }

    std::string decode_html_entities(std::string text)
    {
        text = TextUtils::str_replace_all(text, "&nbsp;", " ");
        text = TextUtils::str_replace_all(text, "&amp;", "&");
        text = TextUtils::str_replace_all(text, "&lt;", "<");
        text = TextUtils::str_replace_all(text, "&gt;", ">");
        text = TextUtils::str_replace_all(text, "&quot;", "\"");
        text = TextUtils::str_replace_all(text, "&#39;", "'");
        return text;
    }
}

void MissionWikiWindow::Initialize()
{
    ToolboxWindow::Initialize();
}

void MissionWikiWindow::Update(float)
{
    if (!visible) return;

    const auto current_map = GW::Map::GetMapID();
    const auto instance_type = GW::Map::GetInstanceType();
    
    // Auto-fetch when entering a mission (explorable areas only for now)
    if (auto_fetch_on_map_change && 
        current_map != last_map_id && 
        instance_type == GW::Constants::InstanceType::Explorable) {
        
        last_map_id = current_map;
        FetchCurrentMission();
    }
}

void MissionWikiWindow::FetchCurrentMission()
{
    if (is_fetching) return;
    
    const auto map_info = GW::Map::GetCurrentMapInfo();
    if (!map_info) return;

    GuiUtils::EncString map_name;
    map_name.language(GW::Constants::Language::English);
    map_name.reset(map_info->name_id);
    mission_name = map_name.string();
    
    if (mission_name.empty()) return;

    is_fetching = true;
    content_ready = false;
    objectives.clear();
    bonus_objectives.clear();
    walkthrough_tips.clear();
    skill_recommendations.clear();
    notes.clear();
    map_texture = nullptr;

    const std::string wiki_url = GuiUtils::WikiUrl(mission_name);
    Resources::Download(wiki_url, OnFetchedWikiPage, this);
}

void MissionWikiWindow::OnFetchedWikiPage(bool success, const std::string& response, void* context)
{
    auto* window = static_cast<MissionWikiWindow*>(context);
    window->is_fetching = false;
    
    if (success) {
        Resources::EnqueueWorkerTask([window, html = response] { 
            window->ParseContent(html); 
        });
    }
}

void MissionWikiWindow::ParseContent(const std::string& html)
{
    // Extract map image from infobox - simplified pattern
    const auto map_pos = html.find("Map.jpg");
    if (map_pos != std::string::npos) {
        // Find src=" before Map.jpg
        const auto src_start = html.rfind("src=\"", map_pos);
        if (src_start != std::string::npos) {
            const auto url_start = src_start + 5; // length of "src=\""
            const auto url_end = html.find("\"", url_start);
            if (url_end != std::string::npos) {
                map_image_url = html.substr(url_start, url_end - url_start);
                if (map_image_url.find("http") != 0) {
                    map_image_url = "https://wiki.guildwars.com" + map_image_url;
                }
                map_texture = Resources::GetGuildWarsWikiImage(map_image_url.c_str(), 0, false);
            }
        }
    }

    // Extract primary objectives - simplified parsing
    auto obj_pos = html.find("id=\"Objectives\"");
    if (obj_pos != std::string::npos) {
        auto ul_start = html.find("<ul>", obj_pos);
        auto ul_end = html.find("</ul>", ul_start);
        if (ul_start != std::string::npos && ul_end != std::string::npos) {
            auto list_html = html.substr(ul_start, ul_end - ul_start);
            size_t li_pos = 0;
            while ((li_pos = list_html.find("<li>", li_pos)) != std::string::npos) {
                auto li_end = list_html.find("</li>", li_pos);
                if (li_end != std::string::npos) {
                    auto obj = list_html.substr(li_pos + 4, li_end - li_pos - 4);
                    obj = strip_html_tags(obj);
                    obj = decode_html_entities(obj);
                    obj = TextUtils::trim(obj);
                    if (!obj.empty()) objectives.push_back(obj);
                    li_pos = li_end;
                }
                else break;
            }
        }
    }

    // Extract bonus objectives
    auto bonus_pos = html.find("id=\"Bonus\"");
    if (bonus_pos != std::string::npos) {
        auto ul_start = html.find("<ul>", bonus_pos);
        auto ul_end = html.find("</ul>", ul_start);
        if (ul_start != std::string::npos && ul_end != std::string::npos) {
            auto list_html = html.substr(ul_start, ul_end - ul_start);
            size_t li_pos = 0;
            while ((li_pos = list_html.find("<li>", li_pos)) != std::string::npos) {
                auto li_end = list_html.find("</li>", li_pos);
                if (li_end != std::string::npos) {
                    auto obj = list_html.substr(li_pos + 4, li_end - li_pos - 4);
                    obj = strip_html_tags(obj);
                    obj = decode_html_entities(obj);
                    obj = TextUtils::trim(obj);
                    if (!obj.empty()) bonus_objectives.push_back(obj);
                    li_pos = li_end;
                }
                else break;
            }
        }
    }

    // Extract walkthrough section
    auto walk_pos = html.find("id=\"Walkthrough\"");
    if (walk_pos != std::string::npos) {
        auto p_start = html.find("<p>", walk_pos);
        auto p_end = html.find("</p>", p_start);
        if (p_start != std::string::npos && p_end != std::string::npos) {
            auto text = html.substr(p_start + 3, p_end - p_start - 3);
            text = strip_html_tags(text);
            text = decode_html_entities(text);
            // Split into sentences
            size_t pos = 0;
            while ((pos = text.find(". ", pos)) != std::string::npos) {
                auto sentence = text.substr(0, pos + 1);
                sentence = TextUtils::trim(sentence);
                if (!sentence.empty() && sentence.length() > 20) {
                    walkthrough_tips.push_back(sentence);
                }
                text = text.substr(pos + 2);
                pos = 0;
            }
            if (!text.empty()) {
                text = TextUtils::trim(text);
                if (text.length() > 20) walkthrough_tips.push_back(text);
            }
        }
    }

    // Extract skill recommendations
    auto skills_pos = html.find("id=\"Skill_recommendations\"");
    if (skills_pos != std::string::npos) {
        auto ul_start = html.find("<ul>", skills_pos);
        auto ul_end = html.find("</ul>", ul_start);
        if (ul_start != std::string::npos && ul_end != std::string::npos) {
            auto list_html = html.substr(ul_start, ul_end - ul_start);
            size_t li_pos = 0;
            while ((li_pos = list_html.find("<li>", li_pos)) != std::string::npos) {
                auto li_end = list_html.find("</li>", li_pos);
                if (li_end != std::string::npos) {
                    auto skill = list_html.substr(li_pos + 4, li_end - li_pos - 4);
                    skill = strip_html_tags(skill);
                    skill = decode_html_entities(skill);
                    skill = TextUtils::trim(skill);
                    if (!skill.empty()) skill_recommendations.push_back(skill);
                    li_pos = li_end;
                }
                else break;
            }
        }
    }

    // Extract notes section
    auto notes_pos = html.find("id=\"Notes\"");
    if (notes_pos != std::string::npos) {
        auto ul_start = html.find("<ul>", notes_pos);
        auto ul_end = html.find("</ul>", ul_start);
        if (ul_start != std::string::npos && ul_end != std::string::npos) {
            auto list_html = html.substr(ul_start, ul_end - ul_start);
            size_t li_pos = 0;
            while ((li_pos = list_html.find("<li>", li_pos)) != std::string::npos) {
                auto li_end = list_html.find("</li>", li_pos);
                if (li_end != std::string::npos) {
                    auto note = list_html.substr(li_pos + 4, li_end - li_pos - 4);
                    note = strip_html_tags(note);
                    note = decode_html_entities(note);
                    note = TextUtils::trim(note);
                    if (!note.empty()) notes.push_back(note);
                    li_pos = li_end;
                }
                else break;
            }
        }
    }
    
    content_ready = true;
}

void MissionWikiWindow::Draw(IDirect3DDevice9*)
{
    if (!visible) return;

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(Name(), GetVisiblePtr(), GetWinFlags())) {
        if (ImGui::Button("Fetch Current Mission")) {
            FetchCurrentMission();
        }
        
        ImGui::Separator();
        
        if (is_fetching) {
            ImGui::TextUnformatted("Loading...");
        }
        else if (content_ready) {
            // Mission title
            ImGui::PushFont(FontLoader::GetFont(FontLoader::FontSize::header2));
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", mission_name.c_str());
            ImGui::PopFont();
            
            ImGui::Separator();
            
            // Map image
            if (map_texture && *map_texture) {
                const float available_width = ImGui::GetContentRegionAvail().x;
                const float img_width = std::min(available_width, 400.0f);
                ImGui::Image(*map_texture, ImVec2(img_width, img_width * 0.75f));
                ImGui::Spacing();
            }
            
            // Primary objectives
            if (!objectives.empty()) {
                ImGui::PushFont(FontLoader::GetFont(FontLoader::FontSize::text));
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Objectives:");
                ImGui::PopFont();
                for (const auto& obj : objectives) {
                    ImGui::BulletText("%s", obj.c_str());
                }
                ImGui::Spacing();
            }
            
            // Bonus objectives
            if (!bonus_objectives.empty()) {
                ImGui::PushFont(FontLoader::GetFont(FontLoader::FontSize::text));
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Bonus:");
                ImGui::PopFont();
                for (const auto& obj : bonus_objectives) {
                    ImGui::BulletText("%s", obj.c_str());
                }
                ImGui::Spacing();
            }

            // Walkthrough
            if (!walkthrough_tips.empty()) {
                ImGui::Separator();
                ImGui::PushFont(FontLoader::GetFont(FontLoader::FontSize::text));
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Strategy:");
                ImGui::PopFont();
                for (const auto& tip : walkthrough_tips) {
                    ImGui::TextWrapped("%s", tip.c_str());
                    ImGui::Spacing();
                }
            }

            // Skill recommendations
            if (!skill_recommendations.empty()) {
                ImGui::Separator();
                ImGui::PushFont(FontLoader::GetFont(FontLoader::FontSize::text));
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 1.0f, 1.0f), "Recommended Skills:");
                ImGui::PopFont();
                for (const auto& skill : skill_recommendations) {
                    ImGui::BulletText("%s", skill.c_str());
                }
                ImGui::Spacing();
            }

            // Notes
            if (!notes.empty()) {
                ImGui::Separator();
                ImGui::PushFont(FontLoader::GetFont(FontLoader::FontSize::text));
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.5f, 1.0f), "Tips:");
                ImGui::PopFont();
                for (const auto& note : notes) {
                    ImGui::BulletText("%s", note.c_str());
                }
            }
        }
    }
    ImGui::End();
}

void MissionWikiWindow::DrawSettingsInternal()
{
    ToolboxWindow::DrawSettingsInternal();
    ImGui::Checkbox("Auto-fetch on map change", &auto_fetch_on_map_change);
    ImGui::TextDisabled("Automatically fetch mission info when entering a new mission");
}

void MissionWikiWindow::LoadSettings(ToolboxIni* ini)
{
    ToolboxWindow::LoadSettings(ini);
    LOAD_BOOL(auto_fetch_on_map_change);
}

void MissionWikiWindow::SaveSettings(ToolboxIni* ini)
{
    ToolboxWindow::SaveSettings(ini);
    SAVE_BOOL(auto_fetch_on_map_change);
}
