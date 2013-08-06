	/*
		Copyright (c) 2013 Sean Mitchell (sean@ideafablabs.com)

		Permission is hereby granted, free of charge, to any person obtaining
		a copy of this software and associated documentation files (the
		"Software"), to deal in the Software without restriction, including
		without limitation the rights to use, copy, modify, merge, publish,
		distribute, sublicense, and/or sell copies of the Software, and to
		permit persons to whom the Software is furnished to do so, subject to
		the following conditions:

		The above copyright notice and this permission notice shall be
		included in all copies or substantial portions of the Software.

		THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
		EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
		MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
		NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
		LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
		OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
		WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	*/
	
	$(function(){
		$('#master').pyromote();
	});

(function($){
	
    // if(!$.Pyro){
    //         $.Pyro = new Object();
    //     }
    // 
    // 		if(!$.Pyro.Grid){
    //         $.Pyro.Grid = new Object();
    //     }
    // 
    // 		
    // 
    //     $.Pyro.UI.Grid = function(el, options){
    // 			var base = this;
    // 			
    // 			base.$el = $(el);
    // 			
    // 			base.$el.data("Pyro.Grid", base);
    // 			
    // 			var gridStartX = $('.x.axis');
    // 			var gridFinishX = $(document).height();
    // 			
    // 			var gridStartY = $('.y.axis');
    // 			var gridFinishY = $(document).width();
    // 			
    // 			
    // 			
    // 		}
    // 		
    // 		$.Pyro.UI.Grid.defaultOptions = {
    // 			
    // 		}
    // 		
    // 		$.fn.pyroUIGRid = function(options){
    //         return this.each(function(){
    //             new $.Pyro.UI.Grid(this, options);
    //         });
    //     };
		
		
		
});
	
(function($){
	
    if(!$.Pyro){
        $.Pyro = new Object();
    }

		if(!$.Pyro.UI){
			$.Pyro.UI = new Object();
		}

    $.Pyro.UI.Master = function(el, options){
        var base = this;

        base.$el = $(el);
        base.el = el;

				base.patterns = '#patterns';
				base.$patterns = base.$el.find(base.patterns);

				//This contains selector the values.
				base.grid = '#grid';
				base.$grid = base.$el.find(base.grid);
				
				//The X/Y location of this determines the value of frameDuration and frameInterval
				base.pointer = '.pointer';
				base.$pointer = base.$grid.find(base.pointer).filter(':first');
				
				base.lastUpdate = new Date().getTime();
				
				base.request = "";
				
				base.status = new Object();
				base.status.idle = true;
				base.status.active = false;
				
				base.$el.data("Pyro.Master", base);

				//Load data into object.
				
				//Updates Parameters when user interacts with the grid.
				base.gridInteraction = function(touch){
					
					var now = new Date().getTime();
					
					base.lastInteraction = now;
					
					base.current = {};
					//
					base.current.dim = {};
					base.current.dim.width = base.$pointer.width();
					base.current.dim.height = base.$pointer.height();
					
					base.current.css = { left : touch.x - (base.current.dim.width/2), top : touch.y - (base.current.dim.height/2)}
					
					//Raw touch values, not normalized to ranges yet. Considering user preference.
					base.current.oriented = {}
					base.current.oriented.duration = !base.options.invertAxis ? touch.x : touch.y;
					base.current.oriented.interval = !base.options.invertAxis ? touch.y : touch.x;
					
					//Interfacing limits. Considering axis inversion, max width or height.
					base.current.limits = {}
					base.current.limits.duration = !base.options.invertAxis ? $(document).width() : $(document).height();
					base.current.limits.interval = !base.options.invertAxis ? $(document).height() : $(document).width();
					
					//Max range value
					base.current.normalized = {}
					base.current.normalized.duration = Math.round(normalize(base.current.oriented.duration, 0, base.current.limits.duration, $.Pyro.Config.Limits.Duration.Min, $.Pyro.Config.Limits.Duration.Max)); //no less than 30 MS, no longer than 750 ms. (flameDuration)
					base.current.normalized.interval = Math.round(normalize(base.current.oriented.interval, 0, base.current.limits.interval, $.Pyro.Config.Limits.Interval.Min, $.Pyro.Config.Limits.Interval.Max)); //no less than 30 MS, no longer than 750 ms. (flameDuration)
				
					//Change the options.
					base.updateOptions('frameDuration', base.current.normalized.duration );
					base.updateOptions('frameInterval', base.current.normalized.interval );
					
					base.updateStats();
					
					base.$pointer.css(base.current.css);
					
					base.updateFollowerLines(touch.x, touch.y);
					
					base.pointerAlpha();
					base.pointerDims();
					base.valueFontSize();
					
					if(now - base.lastUpdate < base.options.refreshRate) return;
					base.$el.data("Pyro.Master", base);
					
					// $.Pyro. 	.send($.Pyro.request);
					// $.Pyro.requestReset();
					// $.Pyro.debug();
				}
				
				base.uiReset = function(){
					base.$pointer.fadeIn();
					
				}

				base.updateOptions = function(key, value){
					if(typeof base.options == 'undefined') return;
					if(typeof base.options[key] == 'undefined') return;
					base.options[key] = value;
					base.addToRequest(key, value);
				}
				
				base.updateStats = function(){
					var $dur = $('.duration .value');
					var $intval = $('.interval .value');
					
					$dur.html(base.options.frameDuration);
					$intval.html(base.options.frameInterval);
				}
				
				base.setupFollowerLines = function(){
					var $x = $('<div>');
					var $y = $('<div>');
				
					$x.appendTo('body');
					$y.appendTo('body');
					
					$x.css({ left: '0px', right  : '0px' }).addClass('follower x');
					$y.css({ top : '0px', bottom : '0px' }).addClass('follower y');
				}
				
				base.updateFollowerLines = function(x, y) {
					$('.follower').show();
					$('.follower.x').css('top', y);
					$('.follower.y').css('left', x);
				}
				
				base.addToRequest = function(key, value){	
					// if(!value.length || typeof value == 'undefined' || typeof key == 'undefined') return;
					// base.$el.data("Pyro.Master");
					var prefix = '';
					
					//Tell teh pyrosphere to start opening valves.
					if(base.options.active == false) {
						base.request += '*1.';
						base.options.active = true;
					}
					
					//What to do with them.
					if(key == 'pattern') prefix += '!';
					if(key == 'frameDuration') prefix += '@';
					if(key == 'frameInterval') prefix += '#';
					
 					base.request += prefix+(value)+'.';
				}
				
				base.pointerAlpha = function(){
					var average = (base.options.frameDuration + base.options.frameInterval) / 2;
					var max = $(document).width() > $(document).height() ? $(document).width()  : $(document).height(); //Bigger of the two
					// console.log('Max' + max);
					var alpha = normalize(average, 0, max, 0.10, 0.60); //Normalize average between
					base.$pointer.find('.gradient').css('opacity', alpha);					
					// console.log('alpha: '+alpha);
				}
				
				base.valueFontSize = function(){
					var size = {};
							size.duration = normalize(base.options.frameDuration, 20, 750, 40, 200 );
							size.interval = normalize(base.options.frameInterval, 20, 750, 40, 200 );
							
					$('header .duration .value').css('font-size', size.duration+'pt');
					$('header .interval .value').css('font-size', size.interval+'pt');
				}
				
				base.setupPointerTransitions = function(){
					base.$pointer.transitionDirs = [
						{ left : base.$pointer.width()*-1 }, //left
						{ left : $(document).width() }, //right
						{ top : base.$pointer.height()*-1  }, //up
						{ top : $(document).height() }, //down
						{ left : base.$pointer.width()*-1, top : base.$pointer.height()*-1 }, //left up
						{ left : base.$pointer.width()*-1, top : $(document).height() }, //left down
						{ left : $(document).width(), top : base.$pointer.height()*-1 }, //right up
						{ left : $(document).width(), top : $(document).height() } //right down
					];
					base.$pointer.transitionDirs = shuffle(base.$pointer.transitionDirs);
				}
				
				base.pointerDims = function(){
					var raw = {}
							raw.width = (!base.options.invertAxis) ? base.options.frameInterval : base.options.frameDuration;
							raw.height = (!base.options.invertAxis) ? base.options.frameDuration : base.options.frameInterval;
							
					var normal = {};
							normal.width = normalize(raw.width, 0, 750, 30, $(document).width()*2);
							normal.height = normalize(raw.height, 0, 750, 30, $(document).height()*2);
							
					base.$pointer.css('width', normal.width+'px').css('height', normal.height+'px')
				}
				
				base.activated = function(){
				}
				
				base.idleCheck = function(){
					var now = new Date().getTime();
					var diff = now - base.lastInteraction;
					var isIdle = diff > $.Pyro.Config.Limits.IdleThreshold;
					if(isIdle) 
						{ base.status.idle = true; base.status.active = true; }
					else 
						{ base.status.idle = false; }
						
					if(diff > $.Pyro.Config.Limits.IdleThreshold*1.5) { base.status.active = false; }
				}

        base.init = function(){
					
          base.options = $.extend({},$.Pyro.UI.Master.defaultOptions, options);

					base.setupFollowerLines();
					
					base.setupPointerTransitions();

					// base.updateOptions('frameDuration', (!base.options.invertAxis ? touch.x : touch.y) );

					//Bind Interactions to Grid
					base.$grid
						.bind('touchstart', function(event){
							
							// base.uiReset();
							
							var touch = event.originalEvent.touches[0];
									touch.y = touch.pageY;
									touch.x = touch.pageX		
									
							base.gridInteraction(touch);
							
						})
						.bind('touchmove', function(event){
							
							var touch = event.originalEvent.touches[0] || event.originalEvent.changedTouches[0];
									touch.y = touch.pageY;
									touch.x = touch.pageX		
							
							base.gridInteraction(touch);
							
						})
						.bind('touchend', function(event){
						
							//Will tell the pyrosphere to turn completely off.
							if(!base.options.persistence) {
								base.$pointer.fadeOut(fast);
								base.$pointer.css({ left : 0, top : 0 });
								base.options.active = false;
								// $.Pyro.socket.send($.Pyro.request);
								// $.Pyro.requestReset();
							}
						});
						
					base.mouseDown = false;
					
						
					base.$grid
						.bind('mousedown', function(event){

							// base.uiReset();
							
							base.mouseDown = true;
							
							if(base.status.idle) { $.playSound('sounds/positive.wav'); base.status.idle = false; }
							else { $.playSound('sounds/welcome.wav'); }
							if(!base.status.active) { base.status.active = true; }
								
								if(typeof event.pageY != 'integer' ||typeof event.pagex != 'integer') return;
							
								base.$pointer.stop()
								.css({ top: 0, left : 0});
								// .css({ top: event.pageY, left: event.pageX, opacity : 1 });
								
							setTimeout(function(){
								base.$pointer.css({ top: event.pageY, left: event.pageX, opacity : 1 });
							}, 3);

							var touch = new Object();
									touch.y = event.pageY;
									touch.x = event.pageX;

							base.gridInteraction(touch);
							// 	
							// base.pointerRotate = typeof base.pointerRotate != 'undefined' ? base.pointerRotate+10 : 0;
							// 					
							// base.$pointer.find('.zero').css({
							//                   transform: 'rotate(' + base.pointerRotate + 'deg)'
							//               });

						})
						
						.bind('mousemove', function(event){
							
							if(!base.mouseDown) return;

							base.uiReset();

							var touch = new Object();
									touch.y = event.pageY;
									touch.x = event.pageX;

							base.gridInteraction(touch);
							
							base.$pointer.find('.zero').css({
                  transform: 'rotate(' + base.pointerRotate + 'deg)'
              });

						})
						
						.bind('mouseup', function(event){
							
							base.mouseDown = false;
							
							var rand = Math.round(Math.random()*7);
							// console.log('Rand: '+rand);

							//Will tell the pyrosphere to turn completely off.
							if(!base.options.persistence) {
								
								var trans = base.$pointer.transitionDirs[rand];
										trans.easing = 'easeInExpo';
										trans.opacity = 0;
								
								base.$pointer.stop().animate(trans, 250, function(){
									// base.$pointer.css({ left : base.$pointer.width()*-1, top : base.$pointer.width()*-1 });
								});
								$('.follower').fadeOut(100);
								base.request += '*0.';
								base.options.active = false;
								// base.updateOptions('active', false);

								// $.Pyro.socket.emit('masterController', '*0.');
								// $.Pyro.requestReset();
							}
							
						});
						
					setTimeout(function(){
						$.Pyro.watcher = setInterval(function(){
							base.$el.data("Pyro.Master");
							if(base.request.length) {
								// $.Pyro.socket.send($.Pyro.request);
								// $.Pyro.requestReset();	
				
								// console.log(base.options);
								console.log('Request: '+base.request);
								base.request = '';
								base.$el.data("Pyro.Master", base);
								base.idleCheck();
								// $.Pyro.requestReset();	
							}
						}, base.options.refreshRate);
				}, 1000);
			
			}

				
				//Initilize;
        base.init();
				//
				
    };

		$.Pyro.UI.Master.userPrefs = {
			scaleAxis : {
				x : {
					min : 40,
					max : 750
				},
				y : {
					min : 40,
					max : 750
				}
			},
			brightness : {
				pointer : 0.85,
				grid : 0.2,
				stats : 0.1
			},
			display : {
				grid : true,
				stats : true,
				pointer : true,
				menu : false
			}
		}

    $.Pyro.UI.Master.defaultOptions = {
				//Parameters
				active: false,					// Opposite of Idle. Reason: shouldn't test false(idle) for true, thus active.
				
				// Animation Parameters
        frameDuration: 300,			// Length of a frame
        frameInterval: 100,			// Time between frames
        patternInterval: 100,		//
        pattern: "00",					// This is the pattern ID.
				persistence:true, 			// When the user let's go, it won't turn off.
				refreshRate: 20,					// This is how fast it will send the server a request.
				
				//Interface Preferences
				invertAxis: false,        //By default, x is frameDuration and y is frameInterval
				
				callbacks: {
					load : function(ui, sphere){ },
					change : function(value, ui, sphere){ },
					onIdle : function(event, ui, sphere) { },
					onActive : function(event, ui, sphere) { },
					toolbarShow : function(toolbar, ui, sphere) { }
				}
				
    };

		/******************************************************************
		/*** jQuery Methods
		/******************************************************************/
		
		//$('.element').pyromote(400, 1000, 400, "00", {});
    $.fn.pyromote = function(options){
        return this.each(function(){
            new $.Pyro.UI.Master(this, options);
        });
    };

    $.fn.getMixerValues = function(){
			this.data("Pyro.Master");
    };

		$.fn.animateRotate = function(angle, duration, easing, complete) {
		    return this.each(function() {
		        var $elem = $(this);

		        $({deg: 0}).animate({deg: angle}, {
		            duration: duration,
		            easing: easing,
		            step: function(now) {
		                $elem.css({
		                    transform: 'rotate(' + now + 'deg)'
		                });
		            },
		            complete: complete || $.noop
		        });
		    });
		};
		
		
})(jQuery);
