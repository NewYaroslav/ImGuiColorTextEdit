#pragma once

#ifndef IMGUICTE_ENABLE_SPIRV
#define IMGUICTE_ENABLE_SPIRV 0
#endif

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <thread>
#include <map>
#include <regex>
#include <imgui.h>

#if IMGUICTE_ENABLE_SPIRV
#include <SHADERed/Objects/SPIRVParser.h>
#endif
namespace ImTextEdit {

    struct Coordinates {
        int mLine;   ///< Zero-based line index.
        int mColumn; ///< Zero-based column index.
        Coordinates()
                : mLine(0)
                , mColumn(0)
        {
        }
        Coordinates(int aLine, int aColumn)
                : mLine(aLine)
                , mColumn(aColumn)
        {
            assert(aLine >= 0);
            assert(aColumn >= 0);
        }

        bool operator==(const Coordinates& o) const
        {
            return mLine == o.mLine && mColumn == o.mColumn;
        }

        bool operator!=(const Coordinates& o) const
        {
            return mLine != o.mLine || mColumn != o.mColumn;
        }

        bool operator<(const Coordinates& o) const
        {
            if (mLine != o.mLine)
                return mLine < o.mLine;
            return mColumn < o.mColumn;
        }

        bool operator>(const Coordinates& o) const
        {
            if (mLine != o.mLine)
                return mLine > o.mLine;
            return mColumn > o.mColumn;
        }

        bool operator<=(const Coordinates& o) const
        {
            if (mLine != o.mLine)
                return mLine < o.mLine;
            return mColumn <= o.mColumn;
        }

        bool operator>=(const Coordinates& o) const
        {
            if (mLine != o.mLine)
                return mLine > o.mLine;
            return mColumn >= o.mColumn;
        }
    };

    inline Coordinates Invalid()
    {
        static Coordinates invalid(-1, -1);
        return invalid;
    }

    struct Identifier {
        Identifier() {}
        Identifier(const std::string& declr)
                : mDeclaration(declr)
        {
        }

        Coordinates mLocation;   ///< Location of the identifier.
        std::string mDeclaration;///< Declaration string.
    };

    typedef std::unordered_map<std::string, Identifier> Identifiers;
    typedef std::unordered_set<std::string> Keywords;

    enum class PaletteIndex {
        Default,
        Keyword,
        Number,
        String,
        CharLiteral,
        Punctuation,
        Preprocessor,
        Identifier,
        KnownIdentifier,
        PreprocIdentifier,
        Comment,
        MultiLineComment,
        Background,
        Cursor,
        Selection,
        ErrorMarker,
        Breakpoint,
        BreakpointOutline,
        CurrentLineIndicator,
        CurrentLineIndicatorOutline,
        LineNumber,
        CurrentLineFill,
        CurrentLineFillInactive,
        CurrentLineEdge,
        ErrorMessage,
        BreakpointDisabled,
        UserFunction,
        UserType,
        UniformVariable,
        GlobalVariable,
        LocalVariable,
        FunctionArgument,
        Max
    };

    enum class ShortcutID {
        Undo,
        Redo,
        MoveUp,
        SelectUp,
        MoveDown,
        SelectDown,
        MoveLeft,
        SelectLeft,
        MoveWordLeft,
        SelectWordLeft,
        MoveRight,
        SelectRight,
        MoveWordRight,
        SelectWordRight,
        MoveUpBlock,
        SelectUpBlock,
        MoveDownBlock,
        SelectDownBlock,
        MoveTop,
        SelectTop,
        MoveBottom,
        SelectBottom,
        MoveStartLine,
        SelectStartLine,
        MoveEndLine,
        SelectEndLine,
        ForwardDelete,
        ForwardDeleteWord,
        DeleteRight,
        BackwardDelete,
        BackwardDeleteWord,
        DeleteLeft,
        OverwriteCursor,
        Copy,
        Paste,
        Cut,
        SelectAll,
        AutocompleteOpen,
        AutocompleteSelect,
        AutocompleteSelectActive,
        AutocompleteUp,
        AutocompleteDown,
        NewLine,
        Indent,
        Unindent,
        Find,
        Replace,
        FindNext,
        DebugStep,
        DebugStepInto,
        DebugStepOut,
        DebugContinue,
        DebugJumpHere,
        DebugBreakpoint,
        DebugStop,
        DuplicateLine,
        CommentLines,
        UncommentLines,
        Count
    };

    struct Shortcut {
        bool Alt;
        bool Ctrl;
        bool Shift;

        int Key1;
        int Key2;

        Shortcut(int vk1 = -1, int vk2 = -2, bool alt = false, bool ctrl = false, bool shift = false)
                : Key1(vk1)
                , Key2(vk2)
                , Alt(alt)
                , Ctrl(ctrl)
                , Shift(shift)
        {
        }
    };

    enum class SelectionMode {
        Normal,
        Word,
        Line
    };

    struct LanguageDefinition {
        typedef std::pair<std::string, PaletteIndex> TokenRegexString;
        typedef std::vector<TokenRegexString> TokenRegexStrings;
        typedef bool (*TokenizeCallback)(const char* in_begin, const char* in_end, const char*& out_begin, const char*& out_end,
            PaletteIndex& paletteIndex);

        std::string mName;
        Keywords mKeywords;
        Identifiers mIdentifiers;
        Identifiers mPreprocIdentifiers;
        std::vector<std::string> single_line_comments;
        std::vector<std::pair<std::string,std::string>> block_comments;
        char mPreprocChar;
        bool mAutoIndentation;

        TokenizeCallback mTokenize;

        TokenRegexStrings mTokenRegexStrings;

        bool mCaseSensitive;

        LanguageDefinition()
                : mPreprocChar('#')
                , mAutoIndentation(true)
                , mTokenize(nullptr)
                , mCaseSensitive(true)
        {
        }
    };

    const LanguageDefinition& CPlusPlus();
    const LanguageDefinition& HLSL();
    const LanguageDefinition& GLSL();
    const LanguageDefinition& SPIRV();
    const LanguageDefinition& C();
    const LanguageDefinition& SQL();
    const LanguageDefinition& AngelScript();
    const LanguageDefinition& Lua();
    const LanguageDefinition& JSON();
    const LanguageDefinition& JSONC();
    const LanguageDefinition& JSONWithHash();
    
    /// \brief JSON5 language definition for ImGuiColorTextEdit.
    /// Notes:
    ///  - Regex syntax: std::regex ECMAScript (no non-capturing groups).
    ///  - Order matters: numbers (incl. Infinity/NaN/hex) go before identifiers.
    ///  - Strings support line continuation via backslash-newline (\\\r?\n).
    const LanguageDefinition& JSON5();

    enum class DebugAction
    {
        Step,
        StepInto,
        StepOut,
        Continue,
        Stop
    };

    /// \brief Interactive text editor with syntax highlighting for ImGui.
    /// \note Right-to-left scripts and complex text shaping are not supported.
    class TextEditor {
    public:
        static const int LineNumberSpace = 20;
        static const int DebugDataSpace = 10;

        /// \brief Represents a debugger breakpoint.
        struct Breakpoint {
            int mLine;              ///< Line index where the breakpoint is located.
            bool mEnabled;          ///< Indicates whether the breakpoint is enabled.
            bool mUseCondition;     ///< True if a conditional expression is evaluated.
            std::string mCondition; ///< Conditional expression evaluated at runtime.

            Breakpoint()
                    : mLine(-1)
                    , mEnabled(false)
            {
            }
        };

        typedef std::string String;
        typedef std::map<int, std::string> ErrorMarkers;
        typedef std::array<ImU32, (unsigned)PaletteIndex::Max> Palette;
        typedef uint8_t Char;

        struct Glyph {
            Char mChar;                      ///< Character code point.
            PaletteIndex mColorIndex = PaletteIndex::Default; ///< Highlight color index.
            bool mComment : 1;              ///< True if part of a single-line comment.
            bool mMultiLineComment : 1;     ///< True if part of a multi-line comment.
            bool mPreprocessor : 1;         ///< True if part of a preprocessor block.

            Glyph(Char aChar, PaletteIndex aColorIndex)
                    : mChar(aChar)
                    , mColorIndex(aColorIndex)
                    , mComment(false)
                    , mMultiLineComment(false)
                    , mPreprocessor(false)
            {
            }
        };

        typedef std::vector<Glyph> Line;
        typedef std::vector<Line> Lines;

        /// \brief Create a text editor instance.
        TextEditor();
        /// \brief Destroy the editor instance.
        ~TextEditor();

        /// \brief Set the language definition used for syntax highlighting.
        /// \param aLanguageDef Language definition to apply.
        void SetLanguageDefinition(const LanguageDefinition& aLanguageDef);
        /// \brief Get current language definition.
        /// \return Reference to the active language definition.
        const LanguageDefinition& GetLanguageDefinition() const { return mLanguageDefinition; }

        /// \brief Get the active color palette.
        /// \return Palette describing colors for each \ref PaletteIndex.
        const Palette& GetPalette() const { return mPaletteBase; }
        /// \brief Set the color palette used for rendering.
        /// \param aValue New palette value.
        void SetPalette(const Palette& aValue);

        void SetErrorMarkers(const ErrorMarkers& aMarkers) { mErrorMarkers = aMarkers; }

        bool HasBreakpoint(int line);
        void AddBreakpoint(int line, bool useCondition = false, std::string condition = "", bool enabled = true);
        void RemoveBreakpoint(int line);
        void SetBreakpointEnabled(int line, bool enable);
        Breakpoint& GetBreakpoint(int line);
        inline const std::vector<Breakpoint>& GetBreakpoints() { return mBreakpoints; }
        void SetCurrentLineIndicator(int line, bool displayBar = true);
        inline int GetCurrentLineIndicator() { return mDebugCurrentLine; }

        inline bool IsDebugging() { return mDebugCurrentLine > 0; }

        /// \brief Render the editor inside an ImGui window.
        /// \param aTitle Window title string.
        /// \param aSize Desired size in pixels.
        /// \param aBorder Set to true to draw a border.
        /// \code
        /// ImTextEdit::TextEditor editor;
        /// editor.SetText("int main() { return 0; }");
        /// editor.Render("Code");
        /// \endcode
        void Render(const char* aTitle, const ImVec2& aSize = ImVec2(), bool aBorder = false);

        /// \brief Replace the entire editor contents.
        /// \param aText New text to display.
        void SetText(const std::string& aText);

        /// \brief Retrieve all text from the editor.
        /// \return Complete text buffer.
        std::string GetText() const;

        /// \brief Set editor text from individual lines.
        /// \param aLines Vector where each element represents one line.
        void SetTextLines(const std::vector<std::string>& aLines);

        /// \brief Copy editor text into a vector of lines.
        /// \param out Destination vector receiving one string per line.
        void GetTextLines(std::vector<std::string>& out) const;

        /// \brief Get currently selected text.
        /// \return Selected substring or empty string if nothing is selected.
        std::string GetSelectedText() const;

        /// \brief Get text from the line containing the cursor.
        /// \return Line contents without trailing newline.
        std::string GetCurrentLineText() const;

        /// \brief Get total number of lines in the document.
        /// \return Line count.
        int GetTotalLines() const { return (int)mLines.size(); }

        /// \brief Determine if overwrite mode is active.
        /// \return True when characters replace existing ones.
        bool IsOverwrite() const { return mOverwrite; }

        /// \brief Check whether the editor window has input focus.
        /// \return True if focused.
        bool IsFocused() const { return mFocused; }

        /// \brief Enable or disable read-only mode.
        /// \param aValue True to disallow modifications.
        void SetReadOnly(bool aValue);

        /// \brief Determine if the editor is currently read-only.
        /// \return True if modifications are not permitted.
        bool IsReadOnly() { return mReadOnly || IsDebugging(); }

        /// \brief Check whether the text has changed since the last reset.
        /// \return True if content was modified.
        bool IsTextChanged() const { return mTextChanged; }

        /// \brief Check whether the cursor position has changed.
        /// \return True if the cursor moved.
        bool IsCursorPositionChanged() const { return mCursorPositionChanged; }

        /// \brief Clear the text-changed flag and tracked lines.
        inline void ResetTextChanged()
        {
            mTextChanged = false;
            mChangedLines.clear();
        }

        /// \brief Determine if syntax colorization is enabled.
        /// \return True when colorizer is active.
        bool IsColorizerEnabled() const { return mColorizerEnabled; }

        /// \brief Enable or disable syntax colorization.
        /// \param aValue True to enable the colorizer.
        void SetColorizerEnable(bool aValue);

        /// \brief Get cursor position using configured tab size.
        /// \note GetCursorPosition() returns position assuming a tab equals four spaces.
        Coordinates GetCorrectCursorPosition();
        Coordinates GetCursorPosition() const { return GetActualCursorCoordinates(); }
        /// \brief Move the cursor to a specific coordinate.
        /// \param aPosition Zero-based line and column.
        void SetCursorPosition(const Coordinates& aPosition);

        /// \brief Enable or disable mouse input handling.
        /// \param aValue True to allow mouse interactions.
        inline void SetHandleMouseInputs(bool aValue) { mHandleMouseInputs = aValue; }

        /// \brief Check if mouse inputs are being handled.
        /// \return True when mouse input is enabled.
        inline bool IsHandleMouseInputsEnabled() const { return mHandleKeyboardInputs; }

        /// \brief Enable or disable keyboard input handling.
        /// \param aValue True to allow keyboard interactions.
        inline void SetHandleKeyboardInputs(bool aValue) { mHandleKeyboardInputs = aValue; }

        /// \brief Check if keyboard inputs are being handled.
        /// \return True when keyboard input is enabled.
        inline bool IsHandleKeyboardInputsEnabled() const { return mHandleKeyboardInputs; }

        /// \brief Ignore the parent ImGui child region.
        /// \param aValue True to ignore child status.
        inline void SetImGuiChildIgnored(bool aValue) { mIgnoreImGuiChild = aValue; }

        /// \brief Check whether child region status is ignored.
        /// \return True when child is ignored.
        inline bool IsImGuiChildIgnored() const { return mIgnoreImGuiChild; }

        /// \brief Toggle visualization of whitespace characters.
        /// \param aValue True to display spaces and tabs.
        inline void SetShowWhitespaces(bool aValue) { mShowWhitespaces = aValue; }

        /// \brief Determine if whitespace visualization is enabled.
        /// \return True when spaces and tabs are shown.
        inline bool IsShowingWhitespaces() const { return mShowWhitespaces; }

        /// \brief Insert text at the cursor position.
        /// \param aValue String to insert.
        /// \param indent True to auto-indent inserted text.
        void InsertText(const std::string& aValue, bool indent = false);

        /// \brief Insert text at the cursor position.
        /// \param aValue Null-terminated string to insert.
        /// \param indent True to auto-indent inserted text.
        void InsertText(const char* aValue, bool indent = false);

        /// \brief Move the cursor up by a number of lines.
        /// \param aAmount Number of lines to move.
        /// \param aSelect True to extend the selection.
        void MoveUp(int aAmount = 1, bool aSelect = false);

        /// \brief Move the cursor down by a number of lines.
        /// \param aAmount Number of lines to move.
        /// \param aSelect True to extend the selection.
        void MoveDown(int aAmount = 1, bool aSelect = false);

        /// \brief Move the cursor left.
        /// \param aAmount Number of columns to move.
        /// \param aSelect True to extend the selection.
        /// \param aWordMode When true, move by words instead of characters.
        void MoveLeft(int aAmount = 1, bool aSelect = false, bool aWordMode = false);

        /// \brief Move the cursor right.
        /// \param aAmount Number of columns to move.
        /// \param aSelect True to extend the selection.
        /// \param aWordMode When true, move by words instead of characters.
        void MoveRight(int aAmount = 1, bool aSelect = false, bool aWordMode = false);

        /// \brief Move the cursor to the first line.
        /// \param aSelect True to extend the selection.
        void MoveTop(bool aSelect = false);

        /// \brief Move the cursor to the last line.
        /// \param aSelect True to extend the selection.
        void MoveBottom(bool aSelect = false);

        /// \brief Move the cursor to the beginning of the current line.
        /// \param aSelect True to extend the selection.
        void MoveHome(bool aSelect = false);

        /// \brief Move the cursor to the end of the current line.
        /// \param aSelect True to extend the selection.
        void MoveEnd(bool aSelect = false);

        /// \brief Set the beginning of the selection.
        /// \param aPosition Start coordinate.
        void SetSelectionStart(const Coordinates& aPosition);

        /// \brief Set the end of the selection.
        /// \param aPosition End coordinate.
        void SetSelectionEnd(const Coordinates& aPosition);

        /// \brief Define a selection range.
        /// \param aStart Start coordinate.
        /// \param aEnd End coordinate.
        /// \param aMode Selection behavior.
        void SetSelection(const Coordinates& aStart, const Coordinates& aEnd, SelectionMode aMode = SelectionMode::Normal);

        /// \brief Select the word under the cursor.
        void SelectWordUnderCursor();

        /// \brief Select the entire document.
        void SelectAll();

        /// \brief Determine whether text is currently selected.
        /// \return True when a selection exists.
        bool HasSelection() const;

        /// \brief Copy current selection to the clipboard.
        void Copy();

        /// \brief Cut current selection to the clipboard.
        void Cut();

        /// \brief Paste clipboard contents at the cursor position.
        void Paste();

        /// \brief Delete the current selection.
        void Delete();

        /// \brief Check whether an undo operation is available.
        /// \return True if there are actions to undo.
        bool CanUndo();

        /// \brief Check whether a redo operation is available.
        /// \return True if there are actions to redo.
        bool CanRedo();

        /// \brief Undo a number of actions.
        /// \param aSteps Number of steps to undo.
        void Undo(int aSteps = 1);

        /// \brief Redo a number of actions.
        /// \param aSteps Number of steps to redo.
        void Redo(int aSteps = 1);

        /// \brief Get expressions relevant for debugging on a given line.
        /// \param line Line index to inspect.
        /// \return List of expressions.
        std::vector<std::string> GetRelevantExpressions(int line);

        /// \brief Highlight specific lines.
        /// \param lines Indices of lines to highlight.
        inline void SetHighlightedLines(const std::vector<int>& lines) { mHighlightedLines = lines; }

        /// \brief Clear all highlighted lines.
        inline void ClearHighlightedLines() { mHighlightedLines.clear(); }

        /// \brief Set the width of a tab character.
        /// \param s Number of spaces per tab (0-32).
        inline void SetTabSize(int s) { mTabSize = std::max<int>(0, std::min<int>(32, s)); }

        /// \brief Get the width of a tab character.
        /// \return Number of spaces per tab.
        inline int GetTabSize() { return mTabSize; }

        /// \brief Toggle insertion of spaces instead of tab characters.
        /// \param s True to insert spaces.
        inline void SetInsertSpaces(bool s) { mInsertSpaces = s; }

        /// \brief Check whether spaces are inserted instead of tabs.
        /// \return True when spaces are used.
        inline int GetInsertSpaces() { return mInsertSpaces; }

        /// \brief Enable smart indentation.
        /// \param s True to enable.
        inline void SetSmartIndent(bool s) { mSmartIndent = s; }

        /// \brief Automatically indent pasted text.
        /// \param s True to enable.
        inline void SetAutoIndentOnPaste(bool s) { mAutoindentOnPaste = s; }

        /// \brief Highlight the current line.
        /// \param s True to enable line highlighting.
        inline void SetHighlightLine(bool s) { mHighlightLine = s; }

        /// \brief Automatically insert matching braces.
        /// \param s True to enable brace completion.
        inline void SetCompleteBraces(bool s) { mCompleteBraces = s; }

        /// \brief Enable horizontal scrolling.
        /// \param s True to allow horizontal scrolling.
        inline void SetHorizontalScroll(bool s) { mHorizontalScroll = s; }

        /// \brief Enable predictive autocomplete.
        /// \param s True to enable suggestions.
        inline void SetSmartPredictions(bool s) { mAutocomplete = s; }

        /// \brief Show function declaration tooltip on hover.
        /// \param s True to enable.
        inline void SetFunctionDeclarationTooltip(bool s) { mFunctionDeclarationTooltipEnabled = s; }

        /// \brief Show user function tooltips on hover.
        /// \param s True to enable.
        inline void SetFunctionTooltips(bool s) { mFuncTooltips = s; }

        /// \brief Manually activate or deactivate autocomplete popup.
        /// \param cac True to activate.
        inline void SetActiveAutocomplete(bool cac) { mActiveAutocomplete = cac; }

        /// \brief Display markers on the scrollbar.
        /// \param markers True to enable markers.
        inline void SetScrollbarMarkers(bool markers) { mScrollbarMarkers = markers; }

        /// \brief Show or hide the sidebar.
        /// \param s True to show the sidebar.
        inline void SetSidebarVisible(bool s) { mSidebar = s; }

        /// \brief Enable or disable search UI.
        /// \param s True to allow searching.
        inline void SetSearchEnabled(bool s) { mHasSearch = s; }

        /// \brief Highlight matching brackets.
        /// \param s True to enable bracket highlighting.
        inline void SetHiglightBrackets(bool s) { mHighlightBrackets = s; }

        /// \brief Enable or disable code folding.
        /// \param s True to enable folding.
        inline void SetFoldEnabled(bool s) { mFoldEnabled = s; }

        /// \brief Set UI scale factor.
        /// \param scale Scale multiplier.
        inline void SetUIScale(float scale) { mUIScale = scale; }

        /// \brief Set UI font size in pixels.
        /// \param size Font size in pixels.
        inline void SetUIFontSize(float size) { mUIFontSize = size; }

        /// \brief Set editor font size in pixels.
        /// \param size Font size in pixels.
        inline void SetEditorFontSize(float size) { mEditorFontSize = size; }

        /// \brief Override a default shortcut.
        /// \param id Identifier of the shortcut to override.
        /// \param s  New shortcut definition.
        void SetShortcut(ShortcutID id, Shortcut s);

        /// \brief Toggle display of line numbers in the sidebar.
        /// \param s True to show line numbers.
        inline void SetShowLineNumbers(bool s)
        {
            mShowLineNumbers = s;
            mTextStart = (s ? 20 : 6);
            mLeftMargin = (s ? (DebugDataSpace + LineNumberSpace) : (DebugDataSpace - LineNumberSpace));
        }

        /// \brief Get horizontal offset where text rendering starts.
        /// \return Offset in pixels from the left edge.
        inline int GetTextStart() const { return mShowLineNumbers ? 7 : 3; }

        /// \brief Recompute syntax highlighting for a set of lines.
        /// \param aFromLine Starting line index.
        /// \param aCount Number of lines to colorize, -1 for all remaining lines.
        void Colorize(int aFromLine = 0, int aCount = -1);

        /// \brief Recompute syntax highlighting for an explicit range.
        /// \param aFromLine First line index.
        /// \param aToLine Last line index.
        void ColorizeRange(int aFromLine = 0, int aToLine = 0);

        /// \brief Recompute syntax highlighting for the entire document.
        void ColorizeInternal();

#       if IMGUICTE_ENABLE_SPIRV
        inline void ClearAutocompleteData()
        {
                mACFunctions.clear();
                mACUserTypes.clear();
                mACUniforms.clear();
                mACGlobals.clear();
        }
#       else
        inline void ClearAutocompleteData() {}
#       endif
        inline void ClearAutocompleteEntries()
        {
                mACEntries.clear();
                mACEntrySearch.clear();
        }
#       if IMGUICTE_ENABLE_SPIRV
        inline const std::unordered_map<std::string, ed::SPIRVParser::Function>& GetAutocompleteFunctions() { return mACFunctions; }
        inline const std::unordered_map<std::string, std::vector<ed::SPIRVParser::Variable>>& GetAutocompleteUserTypes() { return mACUserTypes; }
        inline const std::vector<ed::SPIRVParser::Variable>& GetAutocompleteUniforms() { return mACUniforms; }
        inline const std::vector<ed::SPIRVParser::Variable>& GetAutocompleteGlobals() { return mACGlobals; }
        inline void SetAutocompleteFunctions(const std::unordered_map<std::string, ed::SPIRVParser::Function>& funcs)
        {
                mACFunctions = funcs;
        }
        inline void SetAutocompleteUserTypes(const std::unordered_map<std::string, std::vector<ed::SPIRVParser::Variable>>& utypes)
        {
                mACUserTypes = utypes;
        }
        inline void SetAutocompleteUniforms(const std::vector<ed::SPIRVParser::Variable>& unis)
        {
                mACUniforms = unis;
        }
        inline void SetAutocompleteGlobals(const std::vector<ed::SPIRVParser::Variable>& globs)
        {
                mACGlobals = globs;
        }
#       endif
        /// \brief Add a custom autocomplete entry.
        /// \param search Lookup string used for filtering suggestions.
        /// \param display Text displayed in the suggestion list.
        /// \param value  Text inserted when the suggestion is accepted.
        inline void AddAutocompleteEntry(const std::string& search, const std::string& display, const std::string& value)
        {
            mACEntrySearch.push_back(search);
            mACEntries.push_back(std::make_pair(display, value));
        }
        std::function<void(TextEditor*, int)> OnDebuggerJump;
        std::function<void(TextEditor*, DebugAction)> OnDebuggerAction;
        std::function<void(TextEditor*, const std::string&)> OnIdentifierHover;
        std::function<bool(TextEditor*, const std::string&)> HasIdentifierHover;
        std::function<void(TextEditor*, const std::string&)> OnExpressionHover;
        std::function<bool(TextEditor*, const std::string&)> HasExpressionHover;
        std::function<void(TextEditor*, int)> OnBreakpointRemove;
        std::function<void(TextEditor*, int, bool, const std::string&, bool)> OnBreakpointUpdate;

        std::function<void(TextEditor*, const std::string&, Coordinates coords)> OnCtrlAltClick;
        std::function<void(TextEditor*, const std::string&, const std::string&)> RequestOpen;
        std::function<void(TextEditor*)> OnContentUpdate;

        inline void SetPath(const std::string& path) { mPath = path; }
        inline const std::string& GetPath() { return mPath; }

    private:
        std::string mPath;

        typedef std::vector<std::pair<std::regex, PaletteIndex>> RegexList;
        
        struct EditorState
        {
            Coordinates mSelectionStart;
            Coordinates mSelectionEnd;
            Coordinates mCursorPosition;
        };

        class UndoRecord
        {
        public:
            UndoRecord() {}
            ~UndoRecord() {}

            UndoRecord(
                const std::string& aAdded,
                const Coordinates aAddedStart,
                const Coordinates aAddedEnd,

                const std::string& aRemoved,
                const Coordinates aRemovedStart,
                const Coordinates aRemovedEnd,

                TextEditor::EditorState& aBefore,
                TextEditor::EditorState& aAfter);

            void Undo(TextEditor* aEditor);
            void Redo(TextEditor* aEditor);

            std::string mAdded;
            Coordinates mAddedStart;
            Coordinates mAddedEnd;

            std::string mRemoved;
            Coordinates mRemovedStart;
            Coordinates mRemovedEnd;

            EditorState mBefore;
            EditorState mAfter;
        };

        typedef std::vector<UndoRecord> UndoBuffer;

        void ProcessInputs();
        float TextDistanceToLineStart(const Coordinates& aFrom) const;
        void EnsureCursorVisible();
        int GetPageSize() const;
        std::string GetText(const Coordinates& aStart, const Coordinates& aEnd) const;
        Coordinates GetActualCursorCoordinates() const;
        Coordinates SanitizeCoordinates(const Coordinates& aValue) const;
        void Advance(Coordinates& aCoordinates) const;
        void DeleteRange(const Coordinates& aStart, const Coordinates& aEnd);
        int InsertTextAt(Coordinates& aWhere, const char* aValue, bool indent = false);
        void AddUndo(UndoRecord& aValue);
        Coordinates ScreenPosToCoordinates(const ImVec2& aPosition) const;
        Coordinates MousePosToCoordinates(const ImVec2& aPosition) const;
        ImVec2 CoordinatesToScreenPos(const Coordinates& aPosition) const;
        Coordinates FindWordStart(const Coordinates& aFrom) const;
        Coordinates FindWordEnd(const Coordinates& aFrom) const;
        Coordinates FindNextWord(const Coordinates& aFrom) const;
        int GetCharacterIndex(const Coordinates& aCoordinates) const;
        int GetCharacterColumn(int aLine, int aIndex) const;
        int GetLineCharacterCount(int aLine) const;
        int GetLineMaxColumn(int aLine) const;
        bool IsOnWordBoundary(const Coordinates& aAt) const;
        void RemoveLine(int aStart, int aEnd);
        void RemoveLine(int aIndex);
        Line& InsertLine(int aIndex, int column);
        void EnterCharacter(ImWchar aChar, bool aShift);
        void Backspace();
        void DeleteSelection();
        std::string GetWordUnderCursor() const;
        std::string GetWordAt(const Coordinates& aCoords) const;
        ImU32 GetGlyphColor(const Glyph& aGlyph) const;

        Coordinates FindFirst(const std::string& what, const Coordinates& fromWhere);

        void HandleKeyboardInputs();
        void HandleMouseInputs();
        void RenderInternal(const char* aTitle);

        bool mFuncTooltips;

        float mUIScale, mUIFontSize, mEditorFontSize;
        inline float mUICalculateSize(float h)
        {
            return h * (mUIScale + mUIFontSize / 18.0f - 1.0f);
        }
        inline float mEditorCalculateSize(float h)
        {
            return h * (mUIScale + mEditorFontSize / 18.0f - 1.0f);
        }

        bool mFunctionDeclarationTooltipEnabled;
        Coordinates mFunctionDeclarationCoord;
        bool mFunctionDeclarationTooltip;
        std::string mFunctionDeclaration;
        void mOpenFunctionDeclarationTooltip(const std::string& obj, Coordinates coord);

        std::string mBuildFunctionDef(const std::string& func, const std::string& lang);
#       if IMGUICTE_ENABLE_SPIRV
        std::string mBuildVariableType(const ed::SPIRVParser::Variable& var, const std::string& lang);
#       endif

        float mLineSpacing;
        Lines mLines;
        EditorState mState;
        UndoBuffer mUndoBuffer;
        int mUndoIndex;
        int mReplaceIndex;

        bool mSidebar;
        bool mHasSearch;

        char mFindWord[256];
        bool mFindOpened;
        bool mFindJustOpened;
        bool mFindNext;
        bool mFindFocused, mReplaceFocused;
        bool mReplaceOpened;
        char mReplaceWord[256];

        bool mFoldEnabled;
        std::vector<Coordinates> mFoldBegin, mFoldEnd;
        std::vector<int> mFoldConnection;
        std::vector<bool> mFold;
        bool mFoldSorted;
        void mRemoveFolds(const Coordinates& start, const Coordinates& end);
        void mRemoveFolds(std::vector<Coordinates>& folds, const Coordinates& start, const Coordinates& end);
        uint64_t mFoldLastIteration;
        float mLastScroll;

        std::vector<std::string> mACEntrySearch;
        std::vector<std::pair<std::string, std::string>> mACEntries;

        bool mIsSnippet;
        std::vector<Coordinates> mSnippetTagStart, mSnippetTagEnd;
        std::vector<int> mSnippetTagID;
        std::vector<bool> mSnippetTagHighlight;
        int mSnippetTagSelected, mSnippetTagLength, mSnippetTagPreviousLength;
        std::string mAutcompleteParse(const std::string& str, const Coordinates& start);
        void mAutocompleteSelect();

        bool m_requestAutocomplete, m_readyForAutocomplete;
        void m_buildMemberSuggestions(bool* keepACOpened = nullptr);
        void m_buildSuggestions(bool* keepACOpened = nullptr);
            bool mActiveAutocomplete;
            bool mAutocomplete;
#           if IMGUICTE_ENABLE_SPIRV
            std::unordered_map<std::string, ed::SPIRVParser::Function> mACFunctions;
            std::unordered_map<std::string, std::vector<ed::SPIRVParser::Variable>> mACUserTypes;
            std::vector<ed::SPIRVParser::Variable> mACUniforms;
            std::vector<ed::SPIRVParser::Variable> mACGlobals;
#           endif
            std::string mACWord;
            std::vector<std::pair<std::string, std::string>> mACSuggestions;
            int mACIndex;
            bool mACOpened;
        bool mACSwitched;       // if == true then allow selection with enter
        std::string mACObject;  // if mACObject is not empty, it means user typed '.' -> suggest struct members and methods for mACObject
        Coordinates mACPosition;

        std::vector<Shortcut> m_shortcuts;

        bool mScrollbarMarkers;
        std::vector<int> mChangedLines;

        std::vector<int> mHighlightedLines;

        bool mHorizontalScroll;
        bool mCompleteBraces;
        bool mShowLineNumbers;
        bool mHighlightLine;
        bool mHighlightBrackets;
        bool mInsertSpaces;
        bool mSmartIndent;
        bool mFocused;
        int mTabSize;
        bool mOverwrite;
        bool mReadOnly;
        bool mWithinRender;
        bool mScrollToCursor;
        bool mScrollToTop;
        bool mTextChanged;
        bool mColorizerEnabled;
        float mTextStart;                   // position (in pixels) where a code line starts relative to the left of the TextEditor.
        int  mLeftMargin;
        bool mCursorPositionChanged;
        int mColorRangeMin, mColorRangeMax;
        SelectionMode mSelectionMode;
        bool mHandleKeyboardInputs;
        bool mHandleMouseInputs;
        bool mIgnoreImGuiChild;
        bool mShowWhitespaces;
        bool mAutoindentOnPaste;

        Palette mPaletteBase;
        Palette mPalette;
        LanguageDefinition mLanguageDefinition;
        RegexList mRegexList;

        float mDebugBarWidth, mDebugBarHeight;

        bool mDebugBar;
        bool mDebugCurrentLineUpdated;
        int mDebugCurrentLine;
        ImVec2 mUICursorPos, mFindOrigin;
        float mWindowWidth;
        std::vector<Breakpoint> mBreakpoints;
        ImVec2 mRightClickPos;

        int mPopupCondition_Line;
        bool mPopupCondition_Use;
        char mPopupCondition_Condition[512];

        bool mCheckComments;
        ErrorMarkers mErrorMarkers;
        ImVec2 mCharAdvance;
        Coordinates mInteractiveStart, mInteractiveEnd;
        std::string mLineBuffer;
        uint64_t mStartTime;

        Coordinates mLastHoverPosition;
        std::chrono::steady_clock::time_point mLastHoverTime;

        float mLastClick;
    };

    /// \brief Retrieve default keyboard shortcuts.
    const std::vector<Shortcut> GetDefaultShortcuts();
    /// \brief Get built-in dark color palette.
    const TextEditor::Palette& GetDarkPalette();
    /// \brief Get built-in light color palette.
    const TextEditor::Palette& GetLightPalette();
    /// \brief Get built-in retro blue color palette.
    const TextEditor::Palette& GetRetroBluePalette();

} // namespace ImTextEdit
