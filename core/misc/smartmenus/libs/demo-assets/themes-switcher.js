$(function() {

	var $menu = $('#main-menu');

	// add the HTML structure
	$('div.right-column').prepend('\
  <div id="themes">\
   <h2>Switch theme (class)</h2>\
   <p>\
    <select id="themes-classes">\
\
\
	<!-- include new themes by adding a new option below -->\
\
\
     <option value="sm-blue" data-page-bg="#fbf3e8" data-codepen-url="http://codepen.io/vadikom/pen/rVMmMm?editors=010" data-init-options="{\n\
			subMenusSubOffsetX: 1,\n\
			subMenusSubOffsetY: -8\n\
		}" data-init-options-vertical="{\n\
			mainMenuSubOffsetX: 1,\n\
			mainMenuSubOffsetY: -8,\n\
			subMenusSubOffsetX: 1,\n\
			subMenusSubOffsetY: -8\n\
		}">sm-blue</option>\
     <option value="sm-clean" data-page-bg="#fcfcfc" data-codepen-url="http://codepen.io/vadikom/pen/Mwjmbb?editors=010" data-init-options="{\n\
			mainMenuSubOffsetX: -1,\n\
			mainMenuSubOffsetY: 4,\n\
			subMenusSubOffsetX: 6,\n\
			subMenusSubOffsetY: -6\n\
		}" data-init-options-vertical="{\n\
			mainMenuSubOffsetX: 6,\n\
			mainMenuSubOffsetY: -6,\n\
			subMenusSubOffsetX: 6,\n\
			subMenusSubOffsetY: -6\n\
		}">sm-clean</option>\
     <option value="sm-mint" data-page-bg="#fff" data-codepen-url="http://codepen.io/vadikom/pen/LVRybm?editors=010" data-init-options="{\n\
			subMenusSubOffsetX: 6,\n\
			subMenusSubOffsetY: -8\n\
		}" data-init-options-vertical="{\n\
			mainMenuSubOffsetX: 6,\n\
			mainMenuSubOffsetY: -8,\n\
			subMenusSubOffsetX: 6,\n\
			subMenusSubOffsetY: -8\n\
		}">sm-mint</option>\
     <option value="sm-simple" data-page-bg="#f6f6f6" data-codepen-url="http://codepen.io/vadikom/pen/OVRmbe?editors=010" data-init-options="{\n\
			mainMenuSubOffsetX: -1,\n\
			subMenusSubOffsetX: 10,\n\
			subMenusSubOffsetY: 0\n\
		}" data-init-options-vertical="{\n\
			mainMenuSubOffsetX: 10,\n\
			mainMenuSubOffsetY: 0,\n\
			subMenusSubOffsetX: 10,\n\
			subMenusSubOffsetY: 0\n\
		}">sm-simple</option>\
    </select>\
    <span style="float:right;"><a id="themes-codepen-url" href="http://codepen.io/vadikom/pen/rVMmMm?editors=010">Customize "<span id="themes-codepen-theme-name">sm-blue</span>" on Codepen</a></span><br />\
    <!--[if lt IE 9]><strong>IE8 note: Changing the following options will not produce proper preview for you due to Respond.js related issues. However, these main menu configurations will work just fine on your live website.</strong><br /><![endif]-->\
    <input id="themes-horizontal-fullwidth" name="themes-orientation" value="horizontal-fullwidth" type="radio" checked="checked" /><label for="themes-horizontal-fullwidth">Horizontal full width main menu</label><br />\
    <span id="themes-horizontal-fullwidth-align-holder" style="display:block;padding-left:1.5em;">\
     <input id="themes-horizontal-fullwidth-align-justified" type="checkbox" /><label for="themes-horizontal-fullwidth-align-justified">justified<small style="display:none;"><br />Note: Some themes may need minor changes like tweaking the main menu sub indicators\' position, etc.</small></label><br />\
    </span>\
    <input id="themes-horizontal" name="themes-orientation" value="horizontal" type="radio" /><label for="themes-horizontal">Horizontal main menu</label><br />\
    <span id="themes-horizontal-align-holder" style="display:block;padding-left:1.5em;">\
     <input id="themes-horizontal-align-left" name="themes-horizontal-align" value="left" type="radio" checked="checked" /><label for="themes-horizontal-align-left">left</label>&nbsp;&nbsp;\
     <input id="themes-horizontal-align-center" name="themes-horizontal-align" value="center" type="radio" /><label for="themes-horizontal-align-center">center</label>&nbsp;&nbsp;\
     <input id="themes-horizontal-align-right" name="themes-horizontal-align" value="right" type="radio" /><label for="themes-horizontal-align-right">right</label><br />\
    </span>\
    <input id="themes-vertical" name="themes-orientation" value="vertical" type="radio" /><label for="themes-vertical">Vertical main menu</label><br />\
    <input id="themes-rtl" type="checkbox" /><label for="themes-rtl" title="Won\'t use real RTL text, just preview the theme">Right-to-left</label><br />\
   </p>\
   <h3>Source code</h3>\
   <h4>CSS:</h4>\
   <pre class="sh_html sh_sourceCode">&lt;!-- SmartMenus core CSS (required) --&gt;\n\
&lt;link href="../css/sm-core-css.css" rel="stylesheet" type="text/css" /&gt;\n\
\n\
&lt;!-- "<span class="themes-code-class">sm-blue</span>" menu theme (optional, you can use your own CSS, too) --&gt;\n\
&lt;link href="../css/<span class="themes-code-class">sm-blue</span>/<span class="themes-code-class">sm-blue</span>.css" rel="stylesheet" type="text/css" /&gt;\n' + (window.addonCSS ? window.addonCSS : '') + '\
\n\
<span class="themes-code-main-menu-css-holder" style="display:none;">&lt;!-- #main-menu config - instance specific stuff not covered in the theme --&gt;\n\
&lt;!-- Put this in an external stylesheet if you want the media query to work in IE8 (e.g. where the rest of your page styles are) --&gt;\n\
&lt;style type="text/css"&gt;\n' + (window.addonCSSBefore ? window.addonCSSBefore : '') + '<span class="themes-code-main-menu-css"></span>' + (window.addonCSSAfter ? window.addonCSSAfter : '') + '&lt;/style&gt;\n\
\n</span>\
&lt;!-- HTML5 shim and Respond.js IE8 support of HTML5 elements and media queries --&gt;\n\
&lt;!--[if lt IE 9]&gt;\n\
  &lt;script src="https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js"&gt;&lt;/script&gt;\n\
  &lt;script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"&gt;&lt;/script&gt;\n\
&lt;![endif]--&gt;</pre>\
   <h4>JavaScript:</h4>\
   <pre class="sh_html sh_sourceCode">&lt;!-- jQuery --&gt;\n\
&lt;script type="text/javascript" src="../libs/jquery/jquery.js"&gt;&lt;/script&gt;\n\
\n\
&lt;!-- SmartMenus jQuery plugin --&gt;\n\
&lt;script type="text/javascript" src="../jquery.smartmenus.js"&gt;&lt;/script&gt;\n' + (window.addonScriptSrc ? $.map(window.addonScriptSrc, function(arr) {
	return '\n&lt;!-- ' + arr[0] + ' --&gt;\n&lt;script type="text/javascript" src="' + arr[1] + '"&gt;&lt;/script&gt;\n';
}).join('') : '') + '\
\n\
&lt;!-- SmartMenus jQuery init --&gt;\n\
&lt;script type="text/javascript"&gt;\n\
	$(function() {\n\
		$(\'#main-menu\').smartmenus(<span class="themes-code-init-options">{\n\
			subMenusSubOffsetX: 1,\n\
			subMenusSubOffsetY: -8\n\
		}</span>);\n' + (window.addonScriptInit ? window.addonScriptInit : '') + '\
	});\n\
&lt;/script&gt;</pre>\
   <h4>HTML:</h4>\
   <pre class="sh_html sh_sourceCode">' + (window.addonHTMLBefore ? window.addonHTMLBefore : '') + '\&lt;nav id="main-nav" role="navigation">\n\
	&lt;ul id="main-menu" class="<span class="themes-code-main-class">' + $menu[0].className + '</span>"&gt;\n\
		...\n\
	&lt;/ul>\n\
&lt;/nav>' + (window.addonHTMLAfter ? window.addonHTMLAfter : '') + '</pre>\
  </div>\
');

	// hide sub options
	$('#themes-horizontal-align-holder').hide();

	// load additional themes
	$('#themes-classes option').not(':first').each(function() {
		var className = $(this).attr('value');
		$('<link href="../css/' + className + '/' + className + '.css" rel="stylesheet" type="text/css" />').appendTo('head');
	});

	// update Respond.js to parse all themes loaded dynamically
	if (window.respond) {
		respond.update();
	}

	// define the styles for the different main menu configurations
	var mainMenuConfigs = {
		horizontalLeft: '	@media (min-width: 768px) {\n\
		#main-nav {\n\
			line-height: 0;\n\
			text-align: left;\n\
		}\n\
		#main-menu {\n\
			display: inline-block;\n\
		}\n\
	}\n',
		horizontalCenter: '	@media (min-width: 768px) {\n\
		#main-nav {\n\
			line-height: 0;\n\
			text-align: center;\n\
		}\n\
		#main-menu {\n\
			display: inline-block;\n\
		}\n\
	}\n',
		horizontalRight: '	@media (min-width: 768px) {\n\
		#main-nav {\n\
			line-height: 0;\n\
			text-align: right;\n\
		}\n\
		#main-menu {\n\
			display: inline-block;\n\
		}\n\
	}\n',
		horizontalFullwidthLeft: '',
		horizontalFullwidthJustified: '	@media (min-width: 768px) {\n\
		#main-menu > li {\n\
			float: none;\n\
			display: table-cell;\n\
			width: 1%;\n\
			text-align: center;\n\
		}\n\
	}\n',
		vertical: '	@media (min-width: 768px) {\n\
		#main-menu {\n\
			float: left;\n\
			width: 12em;\n\
		}\n\
	}\n',
		verticalRTL: '	@media (min-width: 768px) {\n\
		#main-menu {\n\
			float: right;\n\
			width: 12em;\n\
		}\n\
	}\n'
	};

	// hook theme switcher
	$('#themes-classes, #themes-horizontal, #themes-horizontal-align-left, #themes-horizontal-align-center, #themes-horizontal-align-right, #themes-horizontal-fullwidth, #themes-horizontal-fullwidth-align-justified, #themes-vertical, #themes-rtl').change(function() {
		var $select = $('#themes-classes'),
			$mainMenuCSS = $('#main-menu-css'),
			mainMenuCSS,
			className = $select.val(),
			horizontal = $('#themes-horizontal')[0].checked,
			horizontalLeft = horizontal && $('#themes-horizontal-align-left')[0].checked,
			horizontalCenter = horizontal && $('#themes-horizontal-align-center')[0].checked,
			horizontalRight = horizontal && $('#themes-horizontal-align-right')[0].checked,
			horizontalFullwidth = $('#themes-horizontal-fullwidth')[0].checked,
			horizontalFullwidthLeft = horizontalFullwidth && !$('#themes-horizontal-fullwidth-align-justified')[0].checked,
			horizontalFullwidthJustified = horizontalFullwidth && $('#themes-horizontal-fullwidth-align-justified')[0].checked,
			vertical = $('#themes-vertical')[0].checked,
			rtl = $('#themes-rtl')[0].checked,
			$optionElm = $select.children().eq($select[0].selectedIndex),
			initOptions = $optionElm.data('init-options' + (vertical ? '-vertical' : '')),
			mainMenuClass = 'sm ' + (rtl ? 'sm-rtl ' : '') + (vertical ? 'sm-vertical ' : '') + className;

		if ($mainMenuCSS.length) {
			$mainMenuCSS.remove();
			$mainMenuCSS = null;
		} else {
			// remove the inline style on init
			$('style').eq(0).remove();
		}
		mainMenuCSS = (window.addonCSSBefore ? window.addonCSSBefore : '') + (
			horizontalLeft ? mainMenuConfigs['horizontalLeft'] :
			horizontalCenter ? mainMenuConfigs['horizontalCenter'] :
			horizontalRight ? mainMenuConfigs['horizontalRight'] :
			horizontalFullwidthLeft ? mainMenuConfigs['horizontalFullwidthLeft'] :
			horizontalFullwidthJustified ? mainMenuConfigs['horizontalFullwidthJustified'] :
			// vertical
			!rtl ? mainMenuConfigs['vertical'] : mainMenuConfigs['verticalRTL']
		) + (window.addonCSSAfter ? window.addonCSSAfter : '');
		$('<style id="main-menu-css">' + mainMenuCSS + '</style>').appendTo('head');

		// show/hide sub options
		$('#themes-horizontal-align-holder')[horizontal ? 'slideDown' : 'slideUp'](250);
		$('#themes-horizontal-fullwidth-align-holder')[horizontalFullwidth ? 'slideDown' : 'slideUp'](250);

		// switch #main-menu theme
		$menu.smartmenus('destroy')[0].className = mainMenuClass;
		$menu.smartmenus(eval('(' + initOptions + ')'));
		$('html, body').css('background', $optionElm.data('page-bg'));

		// update code samples
		$('span.themes-code-class span, #themes-codepen-theme-name').text(className);
		$('#themes-codepen-url').attr('href', $optionElm.data('codepen-url'));
		$('span.themes-code-main-class span').text(mainMenuClass);
		$('span.themes-code-main-menu-css').text(mainMenuCSS);
		$('span.themes-code-main-menu-css-holder')[mainMenuCSS || window.addonCSSBefore || window.addonCSSAfter ? 'show' : 'hide']();
		$('span.themes-code-init-options').text(initOptions);

		// display horizontal justified note if needed
		if ($(this).is('#themes-horizontal-fullwidth-align-justified')) {
			$('label[for="themes-horizontal-fullwidth-align-justified"] small')[this.checked ? 'show' : 'hide']();
		}

		// call any addon init code
		if (window.addonScriptInit) {
			try { eval(window.addonScriptInit); } catch(e) {};
		}
	});

	// init SHJS syntax highlighter
	$('<link href="../libs/demo-assets/shjs/shjs.css" rel="stylesheet" type="text/css" />').appendTo('head');
	sh_highlightDocument();

});

// load SHJS syntax highlighter synchronously
document.write('<scr' + 'ipt type="text/javascript" src="../libs/demo-assets/shjs/shjs.js"></scr' + 'ipt>');