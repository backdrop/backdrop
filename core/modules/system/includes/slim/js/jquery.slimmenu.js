/**
 * jquery.slimmenu.js
 * http://adnantopal.github.io/slimmenu/
 * Author: @adnantopal
 * Copyright 2013-2015, Adnan Topal (adnan.co)
 * Licensed under the MIT license.
 */
(function ($, window, document, undefined) {
    "use strict";

    var pluginName = 'slimmenu',
        oldWindowWidth = 0,
        defaults = {
            resizeWidth: '767',
            initiallyVisible: false,
            collapserTitle: 'Main Menu',
            animSpeed: 'medium',
            easingEffect: null,
            indentChildren: false,
            childrenIndenter: '&nbsp;&nbsp;',
            expandIcon: '<i>&#9660;</i>',
            collapseIcon: '<i>&#9650;</i>'
        };

    function Plugin(element, options) {
        this.element = element;
        this.$elem = $(this.element);
        this.options = $.extend(defaults, options);
        this.init();
    }

    Plugin.prototype = {

        init: function () {
            var $window = $(window),
                options = this.options,
                $menu = this.$elem,
                $collapser = '<div class="menu-collapser">' + options.collapserTitle + '<div class="collapse-button"><span class="icon-bar"></span><span class="icon-bar"></span><span class="icon-bar"></span></div></div>',
                $menuCollapser;

            $menu.before($collapser);
            $menuCollapser = $menu.prev('.menu-collapser');

            $menu.on('click', '.sub-toggle', function (e) {
                e.preventDefault();
                e.stopPropagation();

                var $parentLi = $(this).closest('li');

                if ($(this).hasClass('expanded')) {
                    $(this).removeClass('expanded').html(options.expandIcon);
                    $parentLi.find('>ul').slideUp(options.animSpeed);
                } else {
                    $(this).addClass('expanded').html(options.collapseIcon);
                    $parentLi.find('>ul').slideDown(options.animSpeed);
                }
            });

            $menuCollapser.on('click', '.collapse-button', function (e) {
                e.preventDefault();
                $menu.slideToggle(options.animSpeed);
            });

            this.resizeMenu();
            $window.on('resize', this.resizeMenu.bind(this));
            $window.trigger('resize');
        },

        resizeMenu: function () {
            var self = this,
                $window = $(window),
                windowWidth = $window.width(),
                $options = this.options,
                $menu = $(this.element),
                $menuCollapser = $('body').find('.menu-collapser');

            if (window['innerWidth'] !== undefined) {
                if (window['innerWidth'] > windowWidth) {
                    windowWidth = window['innerWidth'];
                }
            }

            if (windowWidth != oldWindowWidth) {
                oldWindowWidth = windowWidth;

                $menu.find('li').each(function () {
                    if ($(this).has('ul').length) {
                        if ($(this).addClass('has-submenu').has('.sub-toggle').length) {
                            $(this).children('.sub-toggle').html($options.expandIcon);
                        } else {
                            $(this).addClass('has-submenu').append('<span class="sub-toggle">' + $options.expandIcon + '</span>');
                        }
                    }

                    $(this).children('ul').hide().end().find('.sub-toggle').removeClass('expanded').html($options.expandIcon);
                });

                if ($options.resizeWidth >= windowWidth) {
                    if ($options.indentChildren) {
                        $menu.find('ul').each(function () {
                            var $depth = $(this).parents('ul').length;
                            if (!$(this).children('li').children('a').has('i').length) {
                                $(this).children('li').children('a').prepend(self.indent($depth, $options));
                            }
                        });
                    }

                    $menu.addClass('collapsed').find('li').has('ul').off('mouseenter mouseleave');
                    $menuCollapser.show();

                    if (!$options.initiallyVisible) {
                        $menu.hide();
                    }
                } else {
                    $menu.find('li').has('ul')
                        .on('mouseenter', function () {
                            $(this).find('>ul').stop().slideDown($options.animSpeed);
                        })
                        .on('mouseleave', function () {
                            $(this).find('>ul').stop().slideUp($options.animSpeed);
                        });

                    $menu.find('li > a > i').remove();
                    $menu.removeClass('collapsed').show();
                    $menuCollapser.hide();
                    console.log($menuCollapser);
                }
            }
        },

        indent: function (num, options) {
            var i = 0,
                $indent = '';
            for (; i < num; i++) {
                $indent += options.childrenIndenter;
            }
            return '<i>' + $indent + '</i> ';
        }
    };

    $.fn[pluginName] = function (options) {
        return this.each(function () {
            if (!$.data(this, 'plugin_' + pluginName)) {
                $.data(this, 'plugin_' + pluginName,
                    new Plugin(this, options));
            }
        });
    };

}(jQuery, window, document));