#include "../include/gui/gui_editor.h"

class TextEditor{
private:
    enum class Color{
        comment,    // green
        instruction,    // red
        _register,   // blue
        label,  // yellow
        unknown // white
    };

    struct Symbol{
        char c;
        bool comment = false;
        Color color = Color::unknown;

        Symbol(char c) : c{c} {}
        Symbol(char c, bool commented) : c{c}, comment{commented} {
            color = Color::comment;
        }
    };

    typedef std::vector<Symbol> Line;

    std::vector<Line> document;

    struct CursorPosition{
        size_t line_number;
        size_t col_number;
    };

    
    CursorPosition cursor{0,0};

    void UpdateCursor(){

    }

    CursorPosition GetCursorPosition(){
        UpdateCursor();
        return cursor;
    }

    void SetCursorPosition(size_t line_number, size_t col_number){
        cursor.line_number = line_number;
        cursor.col_number = col_number;
    }
    
    void InsertChar(ImWchar c){
        CursorPosition cursor = GetCursorPosition();

        char buffer[7];
        int len = ImTextCharToUtf8(buffer, 7, c);
        buffer[len] = '\0';

        size_t line_number = cursor.line_number;

        int col = cursor.col_number;

        for(int i=0;i<len;i++, col++){
            document[line_number].insert(document[line_number].begin() + cursor.col_number, Symbol(buffer[i]));
        }

        SetCursorPosition(line_number, col);
        return;
    }

    inline int ImTextCharToUtf8(char* buf, int buf_size, unsigned int c)
    {
        if (c < 0x80)
        {
            buf[0] = (char)c;
            return 1;
        }
        if (c < 0x800)
        {
            if (buf_size < 2) return 0;
            buf[0] = (char)(0xc0 + (c >> 6));
            buf[1] = (char)(0x80 + (c & 0x3f));
            return 2;
        }
        if (c < 0x10000)
        {
            if (buf_size < 3) return 0;
            buf[0] = (char)(0xe0 + (c >> 12));
            buf[1] = (char)(0x80 + ((c >> 6) & 0x3f));
            buf[2] = (char)(0x80 + ((c ) & 0x3f));
            return 3;
        }
        if (c <= 0x10FFFF)
        {
            if (buf_size < 4) return 0;
            buf[0] = (char)(0xf0 + (c >> 18));
            buf[1] = (char)(0x80 + ((c >> 12) & 0x3f));
            buf[2] = (char)(0x80 + ((c >> 6) & 0x3f));
            buf[3] = (char)(0x80 + ((c ) & 0x3f));
            return 4;
        }
        // Invalid code point, the max unicode is 0x10FFFF
        return 0;
    }


public:

    void HandleKeyboard(){
        ImGuiIO& io = ImGui::GetIO();

        while(!io.InputQueueCharacters.empty()){
            ImWchar c = io.InputQueueCharacters.back();
            InsertChar(c);
            io.InputQueueCharacters.pop_back();
        }
    }

    void HandleMouse(){

    }

    void Render(){
        
    }
};



void editor_main(){
    static std::string editor_text = "Sample text.\nMultiple lines";

    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 size = {window_size.x * 0.99f, window_size.y * 0.99f};
    ImGui::BeginChild("Editor", size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {
        ImVec2 size{-FLT_MIN, ImGui::GetTextLineHeight() * 100.0f};
        
        ImVec2 text_area_size = {window_size.x * 0.95f, window_size.y * 0.99f};
        ImVec2 text_area_pos = {window_size.x * 0.04f, 0.0f};
        ImGui::SetCursorPos(text_area_pos);

        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushFont(io.Fonts->AddFontFromFileTTF("../../../imgui/misc/fonts/Karla-Regular.ttf", 20.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // loading gray color for editor bg

        ImGui::InputTextMultiline("##editortext", &editor_text, text_area_size, ImGuiInputTextFlags_AllowTabInput);

        ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    ImGui::EndChild();
}