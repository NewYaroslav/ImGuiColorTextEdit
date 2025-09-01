# ImGuiColorTextEdit
Syntax highlighting text editor for ImGui
Compatible with Dear ImGui 1.92 and later.

![Screenshot](https://github.com/BalazsJako/ImGuiColorTextEdit/wiki/ImGuiTextEdit.png "Screenshot")

Demo project: https://github.com/BalazsJako/ColorTextEditorDemo

## Build Flags

- `IMGUICTE_USE_SDL2`: enable when using SDL2 backend to handle keyboard input.
- `IMGUICTE_ENABLE_SPIRV`: enable to support SPIR-V highlighting, requiring `spirv_cross` and disabling certain non-SPIR-V features.

This started as my attempt to write a relatively simple widget which provides text editing functionality with syntax highlighting. Now there are other contributors who provide valuable additions.

While it relies on Omar Cornut's https://github.com/ocornut/imgui, it does not follow the "pure" one widget - one function approach. Since the editor has to maintain a relatively complex and large internal state, it did not seem to be practical to try and enforce fully immediate mode. It stores its internal state in an object instance which is reused across frames.

The code is (still) work in progress, please report if you find any issues.

# Supported Languages

| Language | Function |
| --- | --- |
| CPlusPlus | `static const LanguageDefinition& CPlusPlus();` |
| HLSL | `static const LanguageDefinition& HLSL();` |
| GLSL | `static const LanguageDefinition& GLSL();` |
| SPIRV | `static const LanguageDefinition& SPIRV();` |
| C | `static const LanguageDefinition& C();` |
| SQL | `static const LanguageDefinition& SQL();` |
| AngelScript | `static const LanguageDefinition& AngelScript();` |
| Lua | `static const LanguageDefinition& Lua();` |
| JSON | `static const LanguageDefinition& JSON();` |
| JSONC | `static const LanguageDefinition& JSONC();` |
| JSONWithHash | `static const LanguageDefinition& JSONWithHash();` |

# JSON editor example

The snippet below demonstrates how to embed **ImGuiColorTextEdit** in a JSON tool and how to tweak the color palette.  The code is condensed for clarity but shows a typical integration pattern.

```cpp
#include "ImGuiColorTextEdit.h"

// Create the editor and enable JSON syntax
TextEditor editor;
auto lang = TextEditor::LanguageDefinition::JSON();
editor.SetLanguageDefinition(lang);

// Optional: create a custom color palette based on the dark theme
TextEditor::Palette customPalette = TextEditor::GetDarkPalette();
customPalette[(int)TextEditor::PaletteIndex::Keyword] = ImVec4(0.86f, 0.40f, 0.24f, 1.0f); // keywords
customPalette[(int)TextEditor::PaletteIndex::String]  = ImVec4(0.90f, 0.76f, 0.18f, 1.0f); // strings
editor.SetPalette(customPalette);

// --- main render loop ---
const float window_width  = window.getSize().x;
const float window_height = window.getSize().y;
auto cpos = editor.GetCursorPosition();

ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
ImGui::SetNextWindowSize(ImVec2(window_width, window_height - indent_basement), ImGuiCond_Always);
ImGui::Begin("CryptoJsonMainMenu", nullptr,
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
    ImGuiWindowFlags_MenuBar);

if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open"))
            file_dialog_open = true;
        if (ImGui::MenuItem("Save", nullptr, false, cj_config.is_init)) {
            auto text = editor.GetText();
            // save text to file
        }
        if (ImGui::MenuItem("Save as..."))
            file_dialog_save = true;
        if (ImGui::MenuItem("Quit", "Alt-F4"))
            break;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        bool readOnly = editor.IsReadOnly();
        if (ImGui::MenuItem("Read-only mode", nullptr, &readOnly))
            editor.SetReadOnly(readOnly);
        ImGui::Separator();

        if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !readOnly && editor.CanUndo()))
            editor.Undo();
        if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !readOnly && editor.CanRedo()))
            editor.Redo();

        ImGui::Separator();

        if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
            editor.Copy();
        if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !readOnly && editor.HasSelection()))
            editor.Cut();
        if (ImGui::MenuItem("Delete", "Del", nullptr, !readOnly && editor.HasSelection()))
            editor.Delete();
        if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !readOnly && ImGui::GetClipboardText() != nullptr))
            editor.Paste();

        ImGui::Separator();

        if (ImGui::MenuItem("Select all"))
            editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Dark palette"))
            editor.SetPalette(TextEditor::GetDarkPalette());
        if (ImGui::MenuItem("Light palette"))
            editor.SetPalette(TextEditor::GetLightPalette());
        if (ImGui::MenuItem("Retro blue palette"))
            editor.SetPalette(TextEditor::GetRetroBluePalette());
        if (ImGui::MenuItem("Custom palette"))
            editor.SetPalette(customPalette);
        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
            editor.IsOverwrite() ? "Ovr" : "Ins",
            editor.CanUndo() ? "*" : " ",
            editor.GetLanguageDefinition().mName.c_str(), cj_config.path.c_str());

editor.Render("TextEditor");
ImGui::End();

ImGui::Begin("##basement", NULL,
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
{
    if (ImGui::Button("Validate")) {
        JsonSax json_sax;
        const std::string text = editor.GetText();
        json::sax_parse(text, &json_sax);

        TextEditor::ErrorMarkers markers;
        for (const auto& err : json_sax.errors) {
            int line = std::count(text.begin(), text.begin() + err.first, '\n');
            markers.insert({line, err.second});
        }
        editor.SetErrorMarkers(markers);
    }

    ImGui::SameLine();

    if (ImGui::Button("Copy")) {
        const std::string text = editor.GetText();
        ImGui::SetClipboardText(text.c_str());
    }
}
ImGui::End();
```

# Main features
 - approximates typical code editor look and feel (essential mouse/keyboard commands work - I mean, the commands _I_ normally use :))
 - undo/redo
 - UTF-8 support
 - works with both fixed and variable-width fonts
 - extensible syntax highlighting for multiple languages
 - identifier declarations: a small piece of description can be associated with an identifier. The editor displays it in a tooltip when the mouse cursor is hovered over the identifier
 - error markers: the user can specify a list of error messages together the line of occurence, the editor will highligh the lines with red backround and display error message in a tooltip when the mouse cursor is hovered over the line
 - large files: there is no explicit limit set on file size or number of lines (below 2GB, performance is not affected when large files are loaded (except syntax coloring, see below)
 - color palette support: you can switch between different color palettes, or even define your own
 - whitespace indicators (TAB, space)
 
# Known issues
 - syntax highligthing of most languages - except C/C++ - is based on std::regex, which is diasppointingly slow. Because of that, the highlighting process is amortized between multiple frames. C/C++ has a hand-written tokenizer which is much faster. 
 
Please post your screenshots if you find this little piece of software useful. :)

# Contribute

If you want to contribute, please refer to CONTRIBUTE file.
