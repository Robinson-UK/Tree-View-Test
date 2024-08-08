
#pragma once



// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//  <summary> Subclass tree view so it handles our required behaviours. </summary>
// 
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class Tree_View : public ATL::CWindowImpl<Tree_View, ATL::CWindow>
{
public:
    
    
    
    
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// 
	// CONSTRUCTORS/DESTRUCTOR
	//
	// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    Tree_View();




    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    // ATL/Win32
    //
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#define WM_GET_GDIPLUS_BITMAP (WM_USER + 1)
        
    DECLARE_WND_SUPERCLASS(NULL, WC_TREEVIEW)

    BEGIN_MSG_MAP(Tree_View)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, Wm_Mouse_Left_Button_Down)
        MESSAGE_HANDLER(WM_MOUSEMOVE, Wm_Mouse_Move)
        MESSAGE_HANDLER(WM_LBUTTONUP, Wm_Mouse_Left_Button_Up)
        MESSAGE_HANDLER(WM_TIMER, Wm_Timer)
        MESSAGE_HANDLER(WM_CAPTURECHANGED, Wm_Capture_Changed)
        MESSAGE_HANDLER(WM_DPICHANGED, Wm_Dpi_Changed)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, Wm_Mouse_Left_Button_Double_Click)
        REFLECTED_NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, Wm_Tree_Custom_Draw)
        REFLECTED_NOTIFY_CODE_HANDLER(TVN_BEGINDRAGW, Wm_Tree_Begin_Drag)
    END_MSG_MAP()




    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    //  PUBLIC METHODS
    // 
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------------------
    // 
    //  <summary>	WM_CREATE gets called before we can handle it, so use an Init function here. </summary>
    // 
    // ------------------------------------------------------------------------------------------------------------------

    void Initialise();

protected:

    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    //  PROTECTED METHODS
    // 
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    LRESULT Wm_Capture_Changed(UINT message, WPARAM wParam, LPARAM lParam, BOOL & handled);
    LRESULT Wm_Tree_Custom_Draw(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & handled);
    LRESULT Wm_Mouse_Left_Button_Down(UINT message, WPARAM wParam, LPARAM lParam, BOOL & handled);
    LRESULT Wm_Mouse_Move(UINT message, WPARAM wParam, LPARAM lParam, BOOL & handled);
    LRESULT Wm_Mouse_Left_Button_Up(UINT message, WPARAM wParam, LPARAM lParam, BOOL & handled);
    LRESULT Wm_Mouse_Left_Button_Double_Click(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    LRESULT Wm_Tree_Begin_Drag(int ctrl, LPNMHDR pNMHDR, BOOL & handled);
    LRESULT Wm_Timer(UINT message, WPARAM wParam, LPARAM lParam, BOOL & handled);
    LRESULT Wm_Dpi_Changed(UINT message, WPARAM wParam, LPARAM lParam, BOOL & bHandled);
    
private:

    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    //  PRIVATE METHODS
    // 
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------------------------------------
    // 
    //  <summary>	Tree-related methods. </summary>
    // 
    // ------------------------------------------------------------------------------------------------------------------

    void Tree_Move_All_Children(HTREEITEM parent_old, HTREEITEM parent_new);
    void Tree_End_Drag();
    void Tree_Remember_Expanded_State(HTREEITEM item);
    void Tree_Restore_Expanded_State(HTREEITEM item);
    bool Tree_Is_Descendant(HTREEITEM parent, HTREEITEM child);

    
    
    
    // ------------------------------------------------------------------------------------------------------------------
    // 
    //  <summary>	High DPI related methods. </summary>
    // 
    // ------------------------------------------------------------------------------------------------------------------

    
    void DPI_Initialize();
    void DPI_Set_Font_And_Row_Height(UINT dpi);

    UINT DPI_Get(HWND hwnd);




    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    //  PRIVATE VARIABLES
    // 
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    // High DPI variables.

    int My_Initial_DPI;
    int My_Initial_Row_Height;
    int My_Initial_Indent;

    LOGFONT My_Initial_Font;




    // Drag-drop variables.

    HTREEITEM  My_Drag_Item = nullptr;
    HTREEITEM  My_Drop_Item = nullptr;
    HIMAGELIST My_Drag_Image = nullptr;
    HTREEITEM  My_Drag_Over_Item = nullptr;

    UINT My_Initial_Drag_Over_State = 0;

    std::unordered_map<HTREEITEM, bool> My_Expanded_States;

    bool My_Is_Dragging = false;




    // Timer variables.  We use timers for expand/collapse on drag over and also scroll top/bottom on drag over.

    static const UINT EXPAND_TIMER_INTERVAL = 1000;
    static const UINT EXPAND_TIMER_ID = 1;
    static const UINT SCROLL_TIMER_ID = 2;
    static const UINT SCROLL_TIMER_INTERVAL = 100;
    static const int Initial_Scroll_Zone_Height = 12;

    int My_Scaled_Scroll_Zone_Height = Initial_Scroll_Zone_Height;

    bool My_Is_Scroll_Up = false;
    bool My_Is_Scroll_Down = false;
};
