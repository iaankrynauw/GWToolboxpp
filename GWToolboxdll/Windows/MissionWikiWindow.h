#pragma once

#include <ToolboxWindow.h>

class MissionWikiWindow : public ToolboxWindow {
    MissionWikiWindow() = default;
    ~MissionWikiWindow() override = default;

public:
    static MissionWikiWindow& Instance()
    {
        static MissionWikiWindow instance;
        return instance;
    }

    [[nodiscard]] const char* Name() const override { return "Mission Wiki"; }
    [[nodiscard]] const char* Icon() const override { return ICON_FA_BOOK_OPEN; }

    void Initialize() override;
    void Draw(IDirect3DDevice9* device) override;
    void Update(float delta) override;
    void DrawSettingsInternal() override;
    void LoadSettings(ToolboxIni* ini) override;
    void SaveSettings(ToolboxIni* ini) override;

    void FetchCurrentMission();

private:
    std::string mission_name;
    std::string map_image_url;
    IDirect3DTexture9** map_texture = nullptr;
    std::vector<std::string> objectives;
    std::vector<std::string> bonus_objectives;
    std::vector<std::string> walkthrough_tips;
    std::vector<std::string> skill_recommendations;
    std::vector<std::string> notes;
    bool is_fetching = false;
    bool content_ready = false;
    bool auto_fetch_on_map_change = true;
    GW::Constants::MapID last_map_id = static_cast<GW::Constants::MapID>(0);

    static void OnFetchedWikiPage(bool success, const std::string& response, void* context);
    void ParseContent(const std::string& html);
};
