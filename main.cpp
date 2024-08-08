
#include "stdafx.h"

#include "tree view.h"




// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//  <summary> ATL module. </summary>
// 
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CMyAtlModule : public ATL::CAtlExeModuleT<CMyAtlModule> {};

CMyAtlModule _AtlModule;




// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//  <summary> Main window class. </summary>
// 
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

class CMainWindow : public ATL::CWindowImpl<CMainWindow>
{
public:




    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    // ATL/Win32
    //
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    DECLARE_WND_CLASS(_T("ATLSimpleTreeView"))

    BEGIN_MSG_MAP(CMainWindow)
        MESSAGE_HANDLER(WM_CREATE, Wm_Create)
        MESSAGE_HANDLER(WM_DESTROY, Wm_Destroy)
        MESSAGE_HANDLER(WM_SIZE, Wm_Size)
        MESSAGE_HANDLER(WM_DPICHANGED, Wm_Dpi_Changed)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()




protected:

    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    //  PROTECTED METHODS
    // 
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


    LRESULT Wm_Create(UINT, WPARAM, LPARAM, BOOL&)
    {
        INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_TREEVIEW_CLASSES };

        InitCommonControlsEx(&icex);

        RECT rect;

        GetClientRect(&rect);

        auto tree_view = CreateWindowEx(0, 
                                        WC_TREEVIEW,
                                        _T("Tree View"),
                                        WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_LINESATROOT | TVS_FULLROWSELECT,
                                        10,
                                        10,
                                        rect.right,
                                        rect.bottom,
                                        m_hWnd,
                                        nullptr,
                                        GetModuleHandle(nullptr),
                                        nullptr);
        
        TreeView_SetExtendedStyle(tree_view, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
        TreeView_SetItemHeight(tree_view, 32);

        Initialize_Tree_View_Items(tree_view);

        My_Tree_View.SubclassWindow(tree_view);
        My_Tree_View.Initialise();

        return 0;
    }




    LRESULT Wm_Destroy(UINT, WPARAM, LPARAM, BOOL&)
    {
        PostQuitMessage(0);
        return 0;
    }




    LRESULT Wm_Size(UINT, WPARAM, LPARAM, BOOL&)
    {
        auto area = RECT {};

        GetClientRect(&area);

        ::SetWindowPos(My_Tree_View.m_hWnd, nullptr, 0, 0, area.right, area.bottom, SWP_NOZORDER);

        return 0;
    }




    LRESULT Wm_Dpi_Changed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        // Propagate the WM_DPICHANGED message to the TreeView subclass
        
        if (My_Tree_View.m_hWnd != nullptr)
        {
            ::SendMessage(My_Tree_View.m_hWnd, uMsg, wParam, lParam);
        }

        bHandled = FALSE; // Let the default processing happen

        return 0;
    }

private:

    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    //  PRIVATE METHODS
    // 
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    void Initialize_Tree_View_Items(HWND hwndTV)
    {
        // A random set of nodes.

        auto rd = std::random_device();
        auto gen = std::mt19937(rd());
        auto dis_depth = std::uniform_int_distribution<>(0, 4);
        auto dis_choice = std::uniform_int_distribution<>(0, 100);
        auto node_stack = std::vector<HTREEITEM>();

        node_stack.push_back(TVI_ROOT);

        auto node_count = 1;

        while (node_count <= 50)
        {
            auto current_depth = dis_depth(gen);
            auto parent = TVI_ROOT;

            if (current_depth > 0 && !node_stack.empty())
            {
                auto index = dis_choice(gen) % node_stack.size();

                parent = node_stack[index];
            }

            auto node_name = std::wstring(L"Node") + std::to_wstring(node_count);

            auto tvis = TVINSERTSTRUCT {};

            tvis.hParent = parent;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT;
            tvis.item.pszText = const_cast<LPWSTR>(node_name.c_str());

            auto new_item = reinterpret_cast<HTREEITEM>(SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)&tvis));

            if (current_depth < 4)
            {
                node_stack.push_back(new_item);
            }

            node_count++;
        }
    }
    
    
    
    
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 
    //  PRIVATE VARIABLES
    // 
    // --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    Tree_View My_Tree_View;
};




int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPTSTR, int show)
{
    instance;
    show;




    // Set the DPI awareness (this test program doesn't have a manifest).

    if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) == FALSE)
    {
        // Handle the error if needed.

        MessageBox(NULL, _T("Failed to set DPI awareness"), _T("Error"), MB_OK | MB_ICONERROR);
    }




    // Init common controls.

    InitCommonControls();




    // Compute initial window position.

    auto s_w = GetSystemMetrics(SM_CXSCREEN);
    auto s_h = GetSystemMetrics(SM_CYSCREEN);
    auto w_w = s_w / 2;  // Adjusted to half size for better visibility
    auto w_h = s_h / 2;  // Adjusted to half size for better visibility
    auto w_x = (s_w - w_w) / 2;
    auto w_y = (s_h - w_h) / 2;




    // Create the main window.

    CMainWindow wnd;

    if (nullptr == wnd.Create(nullptr, ATL::CWindow::rcDefault, _T("TreeView Test"), WS_OVERLAPPEDWINDOW))
    {
        return 1;
    } 
    



    // Resize and reposition the window.

    wnd.SetWindowPos(nullptr, w_x, w_y, w_w, w_h, SWP_NOZORDER | SWP_SHOWWINDOW);
    wnd.ShowWindow(show);
    wnd.UpdateWindow();




    // Run the message loop.

    _AtlModule.RunMessageLoop();
    _AtlModule.Term();

    return 0;
}
