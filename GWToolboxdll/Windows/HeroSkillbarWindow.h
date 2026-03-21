#pragma once

#include <Color.h>
#include <ToolboxWindow.h>

class HeroSkillbarWindow final : public ToolboxWindow {
    HeroSkillbarWindow() = default;
    ~HeroSkillbarWindow() override = default;

public:
    static HeroSkillbarWindow& Instance()
    {
        static HeroSkillbarWindow instance;
        return instance;
    }

    [[nodiscard]] const char* Name() const override { return "Hero Skillbar"; }
    [[nodiscard]] const char* Icon() const override { return ICON_FA_USERS; }

    void Initialize() override;
    void Terminate() override;

    void Draw(IDirect3DDevice9* pDevice) override;
    void Update(float delta) override;

    void LoadSettings(ToolboxIni* ini) override;
    void SaveSettings(ToolboxIni* ini) override;
    void DrawSettingsInternal() override;
};
