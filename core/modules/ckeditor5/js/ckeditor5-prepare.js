/**
 * @file
 * Backwards compatibility of UMD build to integration via DLL builds.
 */
(function (CKEDITOR) {
  "use strict";

  /**
   * Prepare CKEditor 5 namespace variables for backwards-compatibility.
   *
   * CKEditor 5 versions 45 and higher stopped supporting the loading mechanism
   * Backdrop used previously (DLL builds) and changed the namespace used by
   * the editor from "CKEditor5" to "CKEDITOR".
   *
   * This mimics some of the DLL structure by adding aliases for possibly
   * extended classes and many utility functions.
   * This does not mimic the entire DLL structure, only CKEDITOR.core and
   * CKEDITOR.ui. This must happen before any custom plugin JS files load.
   */
  function prepareCKEditorNamespaces() {
    /**
     * @deprecated "CKEDITOR.core" does not exist in UMD builds.
     */
    CKEDITOR.core = {};
    CKEDITOR.core.Command = CKEDITOR.Command;
    CKEDITOR.core.Context = CKEDITOR.Context;
    CKEDITOR.core.ContextPlugin = CKEDITOR.ContextPlugin;
    CKEDITOR.core.DataApiMixin = CKEDITOR.DataApiMixin;
    CKEDITOR.core.Editor = CKEDITOR.Editor;
    CKEDITOR.core.ElementApiMixin = CKEDITOR.ElementApiMixin;
    CKEDITOR.core.MultiCommand = CKEDITOR.MultiCommand;
    CKEDITOR.core.PendingActions = CKEDITOR.PendingActions;
    CKEDITOR.core.Plugin = CKEDITOR.Plugin;
    CKEDITOR.core.attachToForm = CKEDITOR.attachToForm;
    // Location and names changed for all icons, so we don't map. An empty
    // object prevents fatal errors.
    CKEDITOR.core.icons = {};
    CKEDITOR.core.secureSourceElement = CKEDITOR.secureSourceElement;

    /**
     * @deprecated "CKEDITOR.ui" does not exist in UMD builds.
     */
    CKEDITOR.ui = {};
    CKEDITOR.ui.AccessibilityHelp = CKEDITOR.AccessibilityHelp;
    CKEDITOR.ui.AutocompleteView = CKEDITOR.AutocompleteView;
    CKEDITOR.ui.BalloonPanelView = CKEDITOR.BalloonPanelView;
    CKEDITOR.ui.BalloonToolbar = CKEDITOR.BalloonToolbar;
    CKEDITOR.ui.BlockToolbar = CKEDITOR.BlockToolbar;
    CKEDITOR.ui.BodyCollection = CKEDITOR.BodyCollection;
    CKEDITOR.ui.BoxedEditorUIView = CKEDITOR.BoxedEditorUIView;
    CKEDITOR.ui.ButtonLabelView = CKEDITOR.ButtonLabelView;
    CKEDITOR.ui.ButtonLabelWithHighlightView = CKEDITOR.ButtonLabelWithHighlightView;
    CKEDITOR.ui.ButtonView = CKEDITOR.ButtonView;
    CKEDITOR.ui.CollapsibleView = CKEDITOR.CollapsibleView;
    CKEDITOR.ui.ColorGridView = CKEDITOR.ColorGridView;
    CKEDITOR.ui.ColorPickerView = CKEDITOR.ColorPickerView;
    CKEDITOR.ui.ColorSelectorView = CKEDITOR.ColorSelectorView;
    CKEDITOR.ui.ColorTileView = CKEDITOR.ColorTileView;
    CKEDITOR.ui.ComponentFactory = CKEDITOR.ComponentFactory;
    CKEDITOR.ui.ContextualBalloon = CKEDITOR.ContextualBalloon;
    CKEDITOR.ui.CssTransitionDisablerMixin = CKEDITOR.CssTransitionDisablerMixin;
    CKEDITOR.ui.DefaultMenuBarItems = CKEDITOR.DefaultMenuBarItems;
    CKEDITOR.ui.Dialog = CKEDITOR.Dialog;
    CKEDITOR.ui.DialogView = CKEDITOR.DialogView;
    CKEDITOR.ui.DialogViewPosition = CKEDITOR.DialogViewPosition;
    CKEDITOR.ui.DocumentColorCollection = CKEDITOR.DocumentColorCollection;
    CKEDITOR.ui.DropdownButtonView = CKEDITOR.DropdownButtonView;
    CKEDITOR.ui.DropdownMenuListItemButtonView = CKEDITOR.DropdownMenuListItemButtonView;
    CKEDITOR.ui.DropdownMenuListItemView = CKEDITOR.DropdownMenuListItemView;
    CKEDITOR.ui.DropdownMenuListView = CKEDITOR.DropdownMenuListView;
    CKEDITOR.ui.DropdownMenuNestedMenuView = CKEDITOR.DropdownMenuNestedMenuView;
    CKEDITOR.ui.DropdownMenuPanelPositioningFunctions = CKEDITOR.DropdownMenuPanelPositioningFunctions;
    CKEDITOR.ui.DropdownMenuRootListView = CKEDITOR.DropdownMenuRootListView;
    CKEDITOR.ui.DropdownPanelView = CKEDITOR.DropdownPanelView;
    CKEDITOR.ui.DropdownView = CKEDITOR.DropdownView;
    CKEDITOR.ui.EditorUI = CKEDITOR.EditorUI;
    CKEDITOR.ui.EditorUIView = CKEDITOR.EditorUIView;
    CKEDITOR.ui.FileDialogButtonView = CKEDITOR.FileDialogButtonView;
    CKEDITOR.ui.FileDialogListItemButtonView = CKEDITOR.FileDialogListItemButtonView;
    CKEDITOR.ui.FocusCycler = CKEDITOR.FocusCycler;
    CKEDITOR.ui.FormHeaderView = CKEDITOR.FormHeaderView;
    CKEDITOR.ui.HighlightedTextView = CKEDITOR.HighlightedTextView;
    CKEDITOR.ui.IconView = CKEDITOR.IconView;
    CKEDITOR.ui.IframeView = CKEDITOR.IframeView;
    CKEDITOR.ui.InlineEditableUIView = CKEDITOR.InlineEditableUIView;
    CKEDITOR.ui.InputNumberView = CKEDITOR.InputNumberView;
    CKEDITOR.ui.InputTextView = CKEDITOR.InputTextView;
    CKEDITOR.ui.InputView = CKEDITOR.InputView;
    CKEDITOR.ui.LabelView = CKEDITOR.LabelView;
    CKEDITOR.ui.LabelWithHighlightView = CKEDITOR.LabelWithHighlightView;
    CKEDITOR.ui.LabeledFieldView = CKEDITOR.LabeledFieldView;
    CKEDITOR.ui.ListItemButtonView = CKEDITOR.ListItemButtonView;
    CKEDITOR.ui.ListItemGroupView = CKEDITOR.ListItemGroupView;
    CKEDITOR.ui.ListItemView = CKEDITOR.ListItemView;
    CKEDITOR.ui.ListSeparatorView = CKEDITOR.ListSeparatorView;
    CKEDITOR.ui.ListView = CKEDITOR.ListView;
    CKEDITOR.ui.MenuBarMenuListItemButtonView = CKEDITOR.MenuBarMenuListItemButtonView;
    CKEDITOR.ui.MenuBarMenuListItemFileDialogButtonView = CKEDITOR.MenuBarMenuListItemFileDialogButtonView;
    CKEDITOR.ui.MenuBarMenuListItemView = CKEDITOR.MenuBarMenuListItemView;
    CKEDITOR.ui.MenuBarMenuListView = CKEDITOR.MenuBarMenuListView;
    CKEDITOR.ui.MenuBarMenuView = CKEDITOR.MenuBarMenuView;
    CKEDITOR.ui.MenuBarView = CKEDITOR.MenuBarView;
    CKEDITOR.ui.Notification = CKEDITOR.Notification;
    CKEDITOR.ui.SearchInfoView = CKEDITOR.SearchInfoView;
    CKEDITOR.ui.SearchTextView = CKEDITOR.SearchTextView;
    CKEDITOR.ui.SpinnerView = CKEDITOR.SpinnerView;
    CKEDITOR.ui.SplitButtonView = CKEDITOR.SplitButtonView;
    CKEDITOR.ui.StickyPanelView = CKEDITOR.StickyPanelView;
    CKEDITOR.ui.SwitchButtonView = CKEDITOR.SwitchButtonView;
    CKEDITOR.ui.Template = CKEDITOR.Template;
    CKEDITOR.ui.TextareaView = CKEDITOR.TextareaView;
    CKEDITOR.ui.ToolbarLineBreakView = CKEDITOR.ToolbarLineBreakView;
    CKEDITOR.ui.ToolbarSeparatorView = CKEDITOR.ToolbarSeparatorView;
    CKEDITOR.ui.ToolbarView = CKEDITOR.ToolbarView;
    CKEDITOR.ui.TooltipManager = CKEDITOR.TooltipManager;
    CKEDITOR.ui.View = CKEDITOR.View;
    CKEDITOR.ui.ViewCollection = CKEDITOR.ViewCollection;
    CKEDITOR.ui.ViewModel = CKEDITOR.ViewModel;
    CKEDITOR.ui.addKeyboardHandlingForGrid = CKEDITOR.addKeyboardHandlingForGrid;
    CKEDITOR.ui.addListToDropdown = CKEDITOR.addListToDropdown;
    CKEDITOR.ui.addMenuToDropdown = CKEDITOR.addMenuToDropdown;
    CKEDITOR.ui.addToolbarToDropdown = CKEDITOR.addToolbarToDropdown;
    CKEDITOR.ui.clickOutsideHandler = CKEDITOR.clickOutsideHandler;
    CKEDITOR.ui.createDropdown = CKEDITOR.createDropdown;
    CKEDITOR.ui.createLabeledDropdown = CKEDITOR.createLabeledDropdown;
    CKEDITOR.ui.createLabeledInputNumber = CKEDITOR.createLabeledInputNumber;
    CKEDITOR.ui.createLabeledInputText = CKEDITOR.createLabeledInputText;
    CKEDITOR.ui.createLabeledTextarea = CKEDITOR.createLabeledTextarea;
    CKEDITOR.ui.filterGroupAndItemNames = CKEDITOR.filterGroupAndItemNames;
    CKEDITOR.ui.focusChildOnDropdownOpen = CKEDITOR.focusChildOnDropdownOpen;
    CKEDITOR.ui.getLocalizedColorOptions = CKEDITOR.getLocalizedColorOptions;
    CKEDITOR.ui.injectCssTransitionDisabler = CKEDITOR.injectCssTransitionDisabler;
    CKEDITOR.ui.isFocusable = CKEDITOR.isFocusable;
    CKEDITOR.ui.isViewWithFocusCycler = CKEDITOR.isViewWithFocusCycler;
    CKEDITOR.ui.normalizeColorOptions = CKEDITOR.normalizeColorOptions;
    CKEDITOR.ui.normalizeMenuBarConfig = CKEDITOR.normalizeMenuBarConfig;
    CKEDITOR.ui.normalizeSingleColorDefinition = CKEDITOR.normalizeSingleColorDefinition;
    CKEDITOR.ui.normalizeToolbarConfig = CKEDITOR.normalizeToolbarConfig;
    CKEDITOR.ui.submitHandler = CKEDITOR.submitHandler;

    // Use a different name to expose to global space - the one we used before.
    window.CKEditor5 = CKEDITOR;

    // Prevent conflict with CKEditor 4. Note this renaming depends on the
    // loading order, v4 has to run after this file.
    delete window.CKEDITOR;
  }

  prepareCKEditorNamespaces();

})(window.CKEDITOR);
