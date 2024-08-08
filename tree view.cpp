
#include "stdafx.h"

#include "tree view.h"
    
    
    
    
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
// CONSTRUCTORS/DESTRUCTOR
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

Tree_View::Tree_View()
{

}




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

void
Tree_View::Initialise()
{
    // Get the initial DPI

    My_Initial_DPI = GetDpiForWindow(m_hWnd);




    // Get the initial font of the TreeView.

    auto hFont = reinterpret_cast<HFONT>(SendMessage(m_hWnd, WM_GETFONT, 0, 0));

    GetObject(hFont, sizeof(LOGFONT), &My_Initial_Font);




    // Calculate the initial row height based on the font height or default 32

    My_Initial_Row_Height = max(32, abs(My_Initial_Font.lfHeight));




    // Set the initial indent to the row height.

    My_Initial_Indent = My_Initial_Row_Height;




    // Set the initial font and row height

    DPI_Set_Font_And_Row_Height(My_Initial_DPI);
}




// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// 
//  PROTECTED METHODS
// 
// --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

LRESULT
Tree_View::Wm_Capture_Changed(UINT, WPARAM, LPARAM, BOOL& handled)
{
    if (!My_Is_Dragging)
    {
        handled = FALSE;

        return 0;
    }
    else
    {
        // We were dragging, so end it now.
    
        Tree_End_Drag();

        handled = TRUE;

        return 0;
    }
}




LRESULT
Tree_View::Wm_Tree_Custom_Draw(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL & handled)
{
    // Default unhandled.

    handled = FALSE;




    // Decide which phase of the custom draw we're in.

    auto custom_draw = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);

    switch (custom_draw->nmcd.dwDrawStage)
    {
    case CDDS_PREPAINT:
        {
            // Keep forgetting this and you'll regret it.

            handled = TRUE;
        }

        return CDRF_NOTIFYITEMDRAW;

    case CDDS_ITEMPREPAINT:
        {
            // Get item information about the item to draw.

            auto item = TVITEM {};
            auto label = std::array<wchar_t, 256>();

            item.mask = TVIF_TEXT | TVIF_STATE;
            item.hItem = (HTREEITEM)custom_draw->nmcd.dwItemSpec;
            item.pszText = label.data();
            item.cchTextMax = 256;
            item.stateMask = TVIS_SELECTED;

            TreeView_GetItem(custom_draw->nmcd.hdr.hwndFrom, &item);




            // Get the tree view's indent.

            auto indent = static_cast<LONG>(TreeView_GetIndent(custom_draw->nmcd.hdr.hwndFrom));
            auto level = 0;
            auto current_item = item.hItem;
        
            while ((current_item = TreeView_GetParent(custom_draw->nmcd.hdr.hwndFrom, current_item)) != nullptr)
            {
                level++;
            }




            // Set colors based on selection state.

            auto colour_background = COLORREF{}, colour_text = COLORREF{};

            if (item.state & TVIS_SELECTED)
            {
                colour_background = GetSysColor(COLOR_HIGHLIGHT);
                colour_text = GetSysColor(COLOR_HIGHLIGHTTEXT);
            }
            else
            {
                colour_background = GetSysColor(COLOR_WINDOW);
                colour_text = GetSysColor(COLOR_WINDOWTEXT);
            }




            // Set the text color and background color.

            SetTextColor(custom_draw->nmcd.hdc, colour_text);
            SetBkColor(custom_draw->nmcd.hdc, colour_background);




            // Fill the background for the entire row.

            auto area_full_row = custom_draw->nmcd.rc;

            FillRect(custom_draw->nmcd.hdc, &area_full_row, CreateSolidBrush(colour_background));




            // Indent size.
            
            auto size_icon   = area_full_row.bottom - area_full_row.top;
            auto size_arrow  = size_icon / 2;
            auto size_indent = (level + 1) * indent;

            auto icon_rect = RECT{ size_indent - size_icon,
                                   area_full_row.top,
                                   size_indent,
                                   area_full_row.bottom };

            auto brush_red = CreateSolidBrush(RGB(255, 0, 0));

            FillRect(custom_draw->nmcd.hdc, &icon_rect, brush_red);
            DeleteObject(brush_red);




            // Draw the arrow based on the expanded/collapsed state.
            
            auto arrow_rect = RECT{ icon_rect.left - size_arrow,
                                    area_full_row.top,
                                    icon_rect.left,
                                    area_full_row.bottom };

            auto brush_green = CreateSolidBrush(RGB(0, 255, 0));

            FillRect(custom_draw->nmcd.hdc, &arrow_rect, brush_green);
            DeleteObject(brush_green);




            // For now we're just going to draw coloured boxes where the icons will be, and the label.

            auto arrow_middle_x = (arrow_rect.left + arrow_rect.right) / 2;
            auto arrow_middle_y = (arrow_rect.top + arrow_rect.bottom) / 2;
            auto arrow_width = (arrow_rect.right - arrow_rect.left) * 3 / 4;
            auto arrow_height = (arrow_rect.bottom - arrow_rect.top) / 4;

            {
                auto pen = CreatePen(PS_SOLID, 1, colour_text);
                auto old_pen = SelectObject(custom_draw->nmcd.hdc, pen);
                auto brush = CreateSolidBrush(colour_text);
                auto old_brush = SelectObject(custom_draw->nmcd.hdc, brush);

                // Only if it has a child.

                if (TreeView_GetChild(custom_draw->nmcd.hdr.hwndFrom, item.hItem))
                {
                    if (TreeView_GetItemState(custom_draw->nmcd.hdr.hwndFrom, item.hItem, TVIS_EXPANDED) & TVIS_EXPANDED)
                    {
                        // Draw up arrow.

                        POINT points[] =
                        {
                            { arrow_middle_x, arrow_middle_y - arrow_height / 2 },
                            { arrow_middle_x - arrow_width / 2, arrow_middle_y + arrow_height / 2 },
                            { arrow_middle_x + arrow_width / 2, arrow_middle_y + arrow_height / 2 }
                        };

                        Polygon(custom_draw->nmcd.hdc, points, 3);
                    }
                    else
                    {
                        // Draw down arrow.

                        POINT points[] =
                        {
                            { arrow_middle_x, arrow_middle_y + arrow_height / 2 },
                            { arrow_middle_x - arrow_width / 2, arrow_middle_y - arrow_height / 2 },
                            { arrow_middle_x + arrow_width / 2, arrow_middle_y - arrow_height / 2 }
                        };

                        Polygon(custom_draw->nmcd.hdc, points, 3);
                    }

                    SelectObject(custom_draw->nmcd.hdc, old_pen);
                    SelectObject(custom_draw->nmcd.hdc, old_brush);
                    DeleteObject(pen);
                    DeleteObject(brush);
                }
            }




            // Draw the item text with the correct indent.

            auto area_label = custom_draw->nmcd.rc;
        
            area_label.left += (level + 1) * indent;  // Indent by the retrieved indent width per level plus space for icon.

            DrawText(custom_draw->nmcd.hdc, item.pszText, -1, &area_label, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);




            // If we're the drop target, render a line underneath.
                     
            if (My_Is_Dragging && item.hItem == My_Drop_Item && !Tree_Is_Descendant(My_Drag_Item, My_Drop_Item))
            {
                auto pen = CreatePen(PS_DOT, 1, GetSysColor(COLOR_GRAYTEXT));
                auto pen_old = SelectObject(custom_draw->nmcd.hdc, pen);
                auto bottom_line_y = area_full_row.bottom - 1;

                MoveToEx(custom_draw->nmcd.hdc, area_full_row.left, bottom_line_y, nullptr);
                LineTo(custom_draw->nmcd.hdc, area_full_row.right, bottom_line_y);
                SelectObject(custom_draw->nmcd.hdc, pen_old);
                DeleteObject(pen);
            }




            // Set handled.

            handled = TRUE;

            return CDRF_SKIPDEFAULT;
        }
    }

    return 0;
}




LRESULT
Tree_View::Wm_Mouse_Left_Button_Down(UINT, WPARAM, LPARAM lParam, BOOL& handled)
{
    // Default unhandled.

    handled = FALSE;




    // Perform hit testing in the TreeView control to find the clicked item.

    auto point = POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    auto info = TVHITTESTINFO{};

    info.pt = point;

    auto item = TreeView_HitTest(m_hWnd, &info);




    // Whatever happens we select the item.  The underlying tree view might not, e.g. on item right, even with full row select.

    if (item && (info.flags & (TVHT_ONITEMBUTTON | TVHT_ONITEMINDENT | TVHT_ONITEM | TVHT_ONITEMRIGHT)))
    {       
        TreeView_SelectItem(m_hWnd, item);
    }




    // Get the item rectangle and indent level.

    {
        auto item_area = RECT{};

        if (TreeView_GetItemRect(m_hWnd, item, &item_area, TRUE))
        {
            // Get the tree view's indent.

            auto indent = static_cast<LONG>(TreeView_GetIndent(m_hWnd));
            auto level = 0;
            auto current_item = item;

            while ((current_item = TreeView_GetParent(m_hWnd, current_item)) != nullptr)
            {
                level++;
            }




            // Indent size.
            
            auto size_icon = item_area.bottom - item_area.top;
            auto size_arrow = size_icon / 2;
            auto size_indent = (level + 1) * indent;
            auto icon_rect = RECT { size_indent - size_icon, item_area.top, size_indent, item_area.bottom };
            auto arrow_rect = RECT { icon_rect.left - size_arrow, item_area.top, icon_rect.left, item_area.bottom };




            // Check if the click is inside the arrow area.
            
            if (PtInRect(&arrow_rect, point))
            {
                // Toggle the expanded state.

                if (TreeView_GetItemState(m_hWnd, item, TVIS_EXPANDED) & TVIS_EXPANDED)
                {
                    TreeView_Expand(m_hWnd, item, TVE_COLLAPSE);
                }
                else
                {
                    TreeView_Expand(m_hWnd, item, TVE_EXPAND);
                }
            }
        }
    }

    return 0;
}




LRESULT
Tree_View::Wm_Mouse_Move(UINT, WPARAM, LPARAM lParam, BOOL& handled)
{
    // Default unhandled.

    handled = FALSE;

    
    
    
    // Nothing to do if not dragging.

    if (!My_Is_Dragging)
    {
        return 0;
    }

    
    
    
    // Move the drag image to the new location (relative to the client coordinates).
    
    auto point = POINT{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    ImageList_DragMove(0, point.y);

    
    
    
    // Turn off dragged image so background is refreshed. Note: causes dragged item to flicker somewhat. I don't like this very much.
    
    ImageList_DragShowNolock(FALSE);

    
    
    
    // Perform hit testing in the TreeView control to find the dragged-over item.
    
    auto info = TVHITTESTINFO{};
    
    info.pt = point;
    
    auto item = TreeView_HitTest(m_hWnd, &info);

    if (item != My_Drag_Over_Item)
    {
        My_Drag_Over_Item = item;

        // We have a timer so that if we hover over an item it expands or collapses.

        if (item != nullptr)
        {
            auto state = TreeView_GetItemState(m_hWnd, item, TVIS_EXPANDED);

            My_Initial_Drag_Over_State = (state & TVIS_EXPANDED) ? TVIS_EXPANDED : 0;

            SetTimer(EXPAND_TIMER_ID, EXPAND_TIMER_INTERVAL, nullptr);
        }
        else
        {
            KillTimer(EXPAND_TIMER_ID);

            My_Initial_Drag_Over_State = 0;
        }
    }




    // We choose the drop target if the user dragged over anywhere on the item.

    if (info.flags & TVHT_ONITEM ||
        info.flags & TVHT_ONITEMRIGHT ||
        info.flags & TVHT_ONITEMINDENT ||
        info.flags & TVHT_ONITEMBUTTON)
    {
        My_Drop_Item = item;

        TreeView_SelectDropTarget(m_hWnd, My_Drop_Item);
    }
    else
    {
        // Not over any item.

        My_Drop_Item = nullptr;

        TreeView_SelectDropTarget(m_hWnd, nullptr);
    }




    // Turn on the dragged image.

    ImageList_DragShowNolock(TRUE);

    
    
    
    // Check if the mouse is in the scroll zone.

    auto area_client = RECT {};

    GetClientRect(&area_client);

    My_Is_Scroll_Up = point.y < My_Scaled_Scroll_Zone_Height;
    My_Is_Scroll_Down = point.y > area_client.bottom - My_Scaled_Scroll_Zone_Height;

    if (My_Is_Scroll_Up || My_Is_Scroll_Down)
    {
        SetTimer(SCROLL_TIMER_ID, SCROLL_TIMER_INTERVAL, nullptr);
    }
    else
    {
        KillTimer(SCROLL_TIMER_ID);
    }




    // Don't use default processing.

    handled = TRUE;

    return 0;
}




LRESULT
Tree_View::Wm_Mouse_Left_Button_Up(UINT, WPARAM, LPARAM, BOOL& handled)
{
    // If we're dragging, end the drag.

    if (My_Is_Dragging)
    {
        Tree_End_Drag();

        handled = TRUE;

        return 0;
    }




    // Not handled.

    handled = FALSE;
    
    return 0;
}




LRESULT
Tree_View::Wm_Mouse_Left_Button_Double_Click(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
    // Perform hit testing in the TreeView control to see where the user double-clicked.  Default doesn't do double-click on the right of the label.

    auto point = POINT { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    auto hit_test_info = TVHITTESTINFO {};

    hit_test_info.pt = point;

    auto item = TreeView_HitTest(m_hWnd, &hit_test_info);

    if (item != nullptr)
    {
        // Toggle the expanded/collapsed state of the item.

        auto state = TreeView_GetItemState(m_hWnd, item, TVIS_EXPANDED);

        if (state & TVIS_EXPANDED)
        {
            TreeView_Expand(m_hWnd, item, TVE_COLLAPSE);
        }
        else
        {
            TreeView_Expand(m_hWnd, item, TVE_EXPAND);
        }

        bHandled = TRUE;

        return 0;
    }

    bHandled = FALSE;

    return 0;
}




LRESULT
Tree_View::Wm_Tree_Begin_Drag(int /* ctrl */, LPNMHDR pNMHDR, BOOL& handled)
{
    // Configure default values.

    auto tree_view_info = reinterpret_cast<NM_TREEVIEW*>(pNMHDR);

    My_Drop_Item = nullptr;
    My_Drag_Over_Item = nullptr;
    My_Initial_Drag_Over_State = 0;




    // We're dragging the given item.

    My_Drag_Item = tree_view_info->itemNew.hItem;




    // Make sure we don't have a timer until we motion over something.

    KillTimer(EXPAND_TIMER_ID);
    KillTimer(SCROLL_TIMER_ID);




    // Get item info.

    auto item = TVITEM {};

    auto label = std::array<wchar_t, 256>();

    item.mask = TVIF_TEXT | TVIF_STATE;
    item.hItem = My_Drag_Item;
    item.pszText = label.data();
    item.cchTextMax = static_cast<int>(label.size());
    item.stateMask = TVIS_SELECTED;
    
    TreeView_GetItem(m_hWnd, &item);




    // Get item rectangles.

    auto area_item = RECT{};

    TreeView_GetItemRect(m_hWnd, My_Drag_Item, &area_item, TRUE);

    auto area_client = RECT {};

    GetClientRect(&area_client);




    // Create a compatible bitmap and device context
    
    HDC hdcScreen = ::GetDC(m_hWnd);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcScreen, area_client.right - area_client.left, area_item.bottom - area_item.top); // Adjust size as needed
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // Fill the background with a solid color
    HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    RECT fillRect = { 0, 0, area_client.right - area_client.left, area_item.bottom - area_item.top };
    FillRect(hdcMem, &fillRect, hBrush);
    DeleteObject(hBrush);

    // Set up custom draw parameters
    NMTVCUSTOMDRAW customDraw = {};
    customDraw.nmcd.hdr.hwndFrom = m_hWnd;
    customDraw.nmcd.hdr.idFrom = GetDlgCtrlID();
    customDraw.nmcd.hdr.code = NM_CUSTOMDRAW;
    customDraw.nmcd.hdc = hdcMem;
    customDraw.nmcd.dwItemSpec = (DWORD_PTR)My_Drag_Item;
    customDraw.nmcd.dwDrawStage = CDDS_ITEMPREPAINT;

    // Adjust the rc to fit the whole row
    customDraw.nmcd.rc.left = 0;
    customDraw.nmcd.rc.top = 0;
    customDraw.nmcd.rc.bottom = area_item.bottom - area_item.top;
    customDraw.nmcd.rc.right = area_client.right;

    // Call custom draw to draw the item into the bitmap
    BOOL custom_draw_handled = FALSE;
    Wm_Tree_Custom_Draw(0, (LPNMHDR)&customDraw, custom_draw_handled);

    // Clean up
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    ::ReleaseDC(m_hWnd, hdcScreen);

    // Create the image list and add the bitmap
    My_Drag_Image = ImageList_Create(area_client.right - area_client.left, area_item.bottom - area_item.top, ILC_COLOR32 | ILC_MASK, 1, 1); // Adjust size as needed
    ImageList_Add(My_Drag_Image, hbmMem, NULL);
    DeleteObject(hbmMem);




    // Calculate the initial offset.

    auto o_y = tree_view_info->ptDrag.y - area_item.top;




    // Begin the drag operation using the created image list with the correct offset

    My_Is_Dragging = true;

    ImageList_BeginDrag(My_Drag_Image, 0, 0, o_y);




    // Use client coordinates for ImageList_DragEnter.

    ImageList_DragEnter(m_hWnd, tree_view_info->ptDrag.x, tree_view_info->ptDrag.y);

    SetCapture();




    // Remember the expanded state of all nodes in the subtree.

    Tree_Remember_Expanded_State(My_Drag_Item);




    // Handled.

    handled = TRUE;

    return 0;
}




LRESULT
Tree_View::Wm_Timer(UINT /*message*/, WPARAM wParam, LPARAM, BOOL& handled)
{
    // Handled either way.

    handled = TRUE;

    
    
    
    // Scroll Timers.
    
    if (wParam == SCROLL_TIMER_ID)
    {
        // Get the current scroll position and range.

        auto scroll_info = SCROLLINFO { sizeof(SCROLLINFO) };

        scroll_info.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;

        GetScrollInfo(SB_VERT, &scroll_info);




        // Calculate the maximum scroll position so we don't attempt it when there's no further to scroll.

        auto max_scroll_position = scroll_info.nMax - (scroll_info.nPage - 1);

        if (My_Is_Scroll_Up)
        {
            if (scroll_info.nPos > scroll_info.nMin)
            {
                ImageList_DragShowNolock(FALSE);

                SendMessage(m_hWnd, WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);

                ImageList_DragShowNolock(TRUE);
            }
            else
            {
                KillTimer(SCROLL_TIMER_ID);
            }
        }
        else if (My_Is_Scroll_Down)
        {
            if (scroll_info.nPos < static_cast<int>(max_scroll_position))
            {
                ImageList_DragShowNolock(FALSE);

                SendMessage(m_hWnd, WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);

                ImageList_DragShowNolock(TRUE);
            }
            else
            {
                KillTimer(SCROLL_TIMER_ID);
            }
        }

        return 0;
    }




    // Expand/Collapse Timers.

    if (wParam == EXPAND_TIMER_ID)
    {
        if (My_Drag_Over_Item == nullptr)
        {
            return 0;
        }




        // Get the current item expanded state.

        auto state = TreeView_GetItemState(m_hWnd, My_Drag_Over_Item, TVIS_EXPANDED);

        
        
        
        // Expand only if it was initially collapsed and collapse only if it was initially expanded.

        if (My_Initial_Drag_Over_State == 0 && !(state & TVIS_EXPANDED))
        {
            TreeView_Expand(m_hWnd, My_Drag_Over_Item, TVE_EXPAND);
        }
        else if (My_Initial_Drag_Over_State == TVIS_EXPANDED && (state & TVIS_EXPANDED))
        {
            TreeView_Expand(m_hWnd, My_Drag_Over_Item, TVE_COLLAPSE);
        }

        
        
        
        // Once expanded or collapsed, stop the timer to prevent toggling.

        KillTimer(EXPAND_TIMER_ID);

        return 0;
    }

    return 0;
}




LRESULT
Tree_View::Wm_Dpi_Changed(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Set font and row height for new DPI.

    auto dpi = HIWORD(wParam);

    DPI_Set_Font_And_Row_Height(dpi);

    
    
    
    // And modify window size.

    auto area = reinterpret_cast<RECT*>(lParam);

    SetWindowPos(NULL, area->left, area->top, area->right - area->left, area->bottom - area->top, SWP_NOZORDER | SWP_NOACTIVATE);

    bHandled = TRUE;

    return 0;
}




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

void
Tree_View::Tree_Move_All_Children(HTREEITEM parent_old, HTREEITEM parent_new)
{
    auto child = TreeView_GetChild(m_hWnd, parent_old);

    while (child)
    {
        auto insert_struct = TVINSERTSTRUCT{};

        insert_struct.hParent = parent_new;
        insert_struct.hInsertAfter = TVI_LAST;
        insert_struct.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;

        auto label = std::array<wchar_t, 256>();
        
        insert_struct.item.pszText = label.data();
        insert_struct.item.cchTextMax = static_cast<int>(label.size());
        insert_struct.item.hItem = child;

        if (TreeView_GetItem(m_hWnd, &insert_struct.item))
        {
            auto new_child = reinterpret_cast<HTREEITEM>(SendMessage(m_hWnd, TVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&insert_struct)));

            Tree_Move_All_Children(child, new_child);

            auto it = My_Expanded_States.find(child);

            if (it != My_Expanded_States.end())
            {
                // We remembered the expand state when drag started, so reproduce it after move.

                TreeView_Expand(m_hWnd, new_child, it->second ? TVE_EXPAND : TVE_COLLAPSE);
            }
        }

        child = TreeView_GetNextSibling(m_hWnd, child);
    }
}




void
Tree_View::Tree_End_Drag()
{
    // Nothing to do if not dragging.

    if (!My_Is_Dragging)
    {
        return;
    }




    // End the drag on the image list.

    ImageList_DragLeave(m_hWnd);
    ImageList_EndDrag();

    if (My_Drag_Image)
    {
        ImageList_Destroy(My_Drag_Image);

        My_Drag_Image = nullptr;
    }

    ReleaseCapture();




    // Set dragging to false before moving any nodes, in case some messages get sent that use this variable.

    My_Is_Dragging = false;

    
    
    
    // No longer need the timer.

    KillTimer(EXPAND_TIMER_ID);
    KillTimer(SCROLL_TIMER_ID);

    
    
    
    // Do we have a drag and a drop item, and they're not equal to each other?

    if (My_Drop_Item && My_Drop_Item != My_Drag_Item && !Tree_Is_Descendant(My_Drag_Item, My_Drop_Item))
    {
        // Move the item.

        auto item = TVITEM{};
        auto label = std::array<wchar_t, 256>();

        item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        item.hItem = My_Drag_Item;
        item.pszText = label.data();
        item.cchTextMax = static_cast<int>(label.size());

        // Get the item's data, insert a new node, move all of its children as well, and then delete it.

        if (TreeView_GetItem(m_hWnd, &item))
        {
            auto insert_struct = TVINSERTSTRUCT{};

            insert_struct.hParent = My_Drop_Item;
            insert_struct.hInsertAfter = TVI_LAST;
            insert_struct.item = item;

            auto new_item = reinterpret_cast<HTREEITEM>(SendMessage(m_hWnd, TVM_INSERTITEM, 0, reinterpret_cast<LPARAM>(&insert_struct)));

            Tree_Move_All_Children(My_Drag_Item, new_item);

            // Restore the expanded state of the dragged item itself.

            auto it = My_Expanded_States.find(My_Drag_Item);

            if (it != My_Expanded_States.end())
            {
                TreeView_Expand(m_hWnd, new_item, it->second ? TVE_EXPAND : TVE_COLLAPSE);
            }

            TreeView_DeleteItem(m_hWnd, My_Drag_Item);
            TreeView_SelectItem(m_hWnd, new_item);
        }
    }




    // Clear the drop target.

    TreeView_SelectDropTarget(m_hWnd, nullptr);




    // Reset all drag-related variables.

    My_Drag_Item = nullptr;
    My_Drop_Item = nullptr;
    My_Drag_Over_Item = nullptr;
    My_Initial_Drag_Over_State = 0;
    My_Expanded_States.clear();
}




void
Tree_View::Tree_Remember_Expanded_State(HTREEITEM item)
{
    // Finished recursion.

    if (!item)
    {
        return;
    }

    
    
    
    // Fetch and store this node's expanded state,.

    auto state = TreeView_GetItemState(m_hWnd, item, TVIS_EXPANDED);
    
    My_Expanded_States[item] = (state & TVIS_EXPANDED) != 0;




    // Recursively remember the state for all child items.

    auto child = TreeView_GetChild(m_hWnd, item);

    while (child)
    {
        Tree_Remember_Expanded_State(child);

        child = TreeView_GetNextSibling(m_hWnd, child);
    }
}



void
Tree_View::Tree_Restore_Expanded_State(HTREEITEM item)
{
    // Finished recursion.

    if (!item)
    {
        return;
    }




    // Restore this node's expanded state.

    auto it = My_Expanded_States.find(item);

    if (it != My_Expanded_States.end() && it->second)
    {
        TreeView_Expand(m_hWnd, item, TVE_EXPAND);
    }
    else
    {
        TreeView_Expand(m_hWnd, item, TVE_COLLAPSE);
    }




    // Recursively restore the state for all child items.

    auto child = TreeView_GetChild(m_hWnd, item);

    while (child)
    {
        Tree_Restore_Expanded_State(child);

        child = TreeView_GetNextSibling(m_hWnd, child);
    }
}




bool
Tree_View::Tree_Is_Descendant(HTREEITEM parent, HTREEITEM child)
{
    auto current = child;

    while (current)
    {
        if (current == parent)
        {
            return true;
        }

        current = TreeView_GetParent(m_hWnd, current);
    }

    return false;
}




// ------------------------------------------------------------------------------------------------------------------
// 
//  <summary>	High DPI related methods. </summary>
// 
// ------------------------------------------------------------------------------------------------------------------

void
Tree_View::DPI_Initialize()
{
    auto dpi = GetDpiForWindow(m_hWnd);

    DPI_Set_Font_And_Row_Height(dpi);
}




UINT
Tree_View::DPI_Get(HWND)
{
    auto hdc = GetDC();
    auto dpi = GetDeviceCaps(hdc, LOGPIXELSX);

    ReleaseDC(hdc);

    return dpi;
}




void
Tree_View::DPI_Set_Font_And_Row_Height(UINT dpi)
{
    // Adjust the font size based on the DPI.

    auto log_font = My_Initial_Font;

    log_font.lfHeight = MulDiv(My_Initial_Font.lfHeight, dpi, My_Initial_DPI);

    
    
    
    // Create the new scaled font.

    auto new_font = CreateFontIndirect(&log_font);

    
    
    
    // Set the new font.

    SendMessage(m_hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(new_font), TRUE);

    
    
    
    // Set the row height based on the DPI.

    auto row_height = max(32, MulDiv(My_Initial_Row_Height, dpi, My_Initial_DPI));

    TreeView_SetItemHeight(m_hWnd, row_height);

    
    
    
    // Set the indent based on the new row height but we'll also add a little width for our arrow.

    auto size_icon = row_height;
    auto size_arrow = size_icon / 2;
    auto size_indent = size_icon + size_arrow;
    
    TreeView_SetIndent(m_hWnd, size_indent);

    
    
    
    // Scale the scroll zone height based on the DPI
    
    My_Scaled_Scroll_Zone_Height = MulDiv(Initial_Scroll_Zone_Height, dpi, 96);
}



