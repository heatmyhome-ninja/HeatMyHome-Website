var $jscomp=$jscomp||{};$jscomp.scope={};$jscomp.arrayIteratorImpl=function(a){var b=0;return function(){return b<a.length?{done:!1,value:a[b++]}:{done:!0}}};$jscomp.arrayIterator=function(a){return{next:$jscomp.arrayIteratorImpl(a)}};$jscomp.makeIterator=function(a){var b="undefined"!=typeof Symbol&&Symbol.iterator&&a[Symbol.iterator];return b?b.call(a):$jscomp.arrayIterator(a)};
var INITIAL_WINDOW_WIDTH=window.innerWidth,INITIAL_WINDOW_HEIGHT=window.innerHeight,DEFAULT_XSTEP_LENGTH=16+7/1508*(INITIAL_WINDOW_WIDTH-412),DEFAULT_YSTEP_LENGTH=Math.max(1,Math.abs(38-20/140*(INITIAL_WINDOW_HEIGHT-789))),DEFAULT_MIN_ANIMATION_FRAMES=INITIAL_WINDOW_HEIGHT/DEFAULT_YSTEP_LENGTH,DEFAULT_SCROLL_CALCULATOR_TEST_VALUE=100,DEFAULT_PAGE_SCROLLER=window,DEFAULT_ERROR_LOGGER=function(a,b,c){if(!/disabled/i.test(uss._debugMode)){var d="string"===typeof c;c=null===c?"null":void 0===c?"undefined":
c.name||c.toString().replaceAll("\n"," ");30<c.length&&(c=c.slice(0,30)+" ...");d&&(c='"'+c+'"');/legacy/i.test(uss._debugMode)?(console.log("UniversalSmoothScroll API (documentation at: https://github.com/CristianDavideConte/universalSmoothScroll)\n"),console.error("USS ERROR\n ",a,"was expecting",b+", but it received",c+".")):(console.group("UniversalSmoothScroll API (documentation at: https://github.com/CristianDavideConte/universalSmoothScroll)"),console.log("%cUSS ERROR","font-family: system-ui; font-weight: 800; font-size: 40px;  background: #eb445a; color:black; border-radius: 5px 5px 5px 5px; padding:0.4vh 0.5vw; margin: 1vh 0"),
console.log("  %c"+a+"%cwas expecting "+b,"font-style: italic; font-family: system-ui; font-weight: 700; font-size: 17px; background: #2dd36f; color: black; border-radius: 5px 0px 0px 5px; padding:0.4vh 0.5vw","font-family: system-ui; font-weight: 600; font-size: 17px; background: #2dd36f; color:black; border-radius: 0px 5px 5px 0px; padding:0.4vh 0.5vw"),console.log("  %cBut it received%c"+c,"font-family: system-ui; font-weight: 600; font-size: 17px; background: #eb445a; color:black; border-radius: 5px 0px 0px 5px; padding:0.4vh 0.5vw",
"font-style: italic; font-family: system-ui; font-weight: 700; font-size: 17px; background: #eb445a; color: black; border-radius: 0px 5px 5px 0px; padding:0.4vh 0.5vw"),console.trace("%cStack Trace","font-family: system-ui; font-weight: 500; font-size: 17px; background: #3171e0; color: #f5f6f9; border-radius: 5px; padding:0.3vh 0.5vw; margin-left: 2px; margin-top: 1vh"),console.groupEnd("UniversalSmoothScroll API (documentation at: https://github.com/CristianDavideConte/universalSmoothScroll)"))}},
DEFAULT_WARNING_LOGGER=function(a,b){if(!/disabled/i.test(uss._debugMode)){var c="string"===typeof a;a=null===a?"null":void 0===a?"undefined":a.name||a.toString().replaceAll("\n"," ");30<a.length&&(a=a.slice(0,30)+" ...");c&&(a='"'+a+'"');/legacy/i.test(uss._debugMode)?(console.log("UniversalSmoothScroll API (documentation at: https://github.com/CristianDavideConte/universalSmoothScroll)\n"),console.warn("USS WARNING\n ",a,b+".")):(console.groupCollapsed("UniversalSmoothScroll API (documentation at: https://github.com/CristianDavideConte/universalSmoothScroll)"),
console.log("%cUSS WARNING:","font-family: system-ui; font-weight: 800; font-size: 40px;  background: #fcca03; color:black; border-radius: 5px 5px 5px 5px; padding:0.4vh 0.5vw; margin: 1vh 0"),console.log("  %c"+a+"%c"+b,"font-style: italic; font-family: system-ui; font-weight: 700; font-size: 17px; background: #fcca03; color: black; border-radius: 5px 0px 0px 5px; padding:0.4vh 0.5vw","font-family: system-ui; font-weight: 600; font-size: 17px; background: #fcca03; color:black; border-radius: 0px 5px 5px 0px; padding:0.4vh 0.5vw"),
console.trace("%cStack Trace","font-family: system-ui; font-weight: 500; font-size: 17px; background: #3171e0; color: #f5f6f9; border-radius: 5px; padding:0.3vh 0.5vw; margin-left: 2px; margin-top: 1vh"),console.groupEnd("UniversalSmoothScroll API (documentation at: https://github.com/CristianDavideConte/universalSmoothScroll)"))}},uss={_containersData:new Map,_xStepLength:DEFAULT_XSTEP_LENGTH,_yStepLength:DEFAULT_YSTEP_LENGTH,_minAnimationFrame:DEFAULT_MIN_ANIMATION_FRAMES,_windowHeight:INITIAL_WINDOW_HEIGHT,
_windowWidth:INITIAL_WINDOW_WIDTH,_scrollbarsMaxDimension:0,_pageScroller:DEFAULT_PAGE_SCROLLER,_reducedMotion:"matchMedia"in window&&window.matchMedia("(prefers-reduced-motion)").matches,_debugMode:"",isXscrolling:function(a){a=void 0===a?uss._pageScroller:a;if(a===window||a instanceof HTMLElement)return"number"===typeof(uss._containersData.get(a)||[])[0];DEFAULT_ERROR_LOGGER("isXscrolling","an HTMLElement or the Window",a)},isYscrolling:function(a){a=void 0===a?uss._pageScroller:a;if(a===window||
a instanceof HTMLElement)return"number"===typeof(uss._containersData.get(a)||[])[1];DEFAULT_ERROR_LOGGER("isYscrolling","an HTMLElement or the Window",a)},isScrolling:function(a){a=void 0===a?uss._pageScroller:a;if(a===window||a instanceof HTMLElement)return a=uss._containersData.get(a)||[],"number"===typeof a[0]||"number"===typeof a[1];DEFAULT_ERROR_LOGGER("isScrolling","an HTMLElement or the Window",a)},getFinalXPosition:function(a){a=void 0===a?uss._pageScroller:a;if(a===window||a instanceof HTMLElement){var b=
uss._containersData.get(a)||[];return"number"===typeof b[0]?b[2]:uss.getScrollXCalculator(a)()}DEFAULT_ERROR_LOGGER("getFinalXPosition","an HTMLElement or the Window",a)},getFinalYPosition:function(a){a=void 0===a?uss._pageScroller:a;if(a===window||a instanceof HTMLElement){var b=uss._containersData.get(a)||[];return"number"===typeof b[1]?b[3]:uss.getScrollYCalculator(a)()}DEFAULT_ERROR_LOGGER("getFinalYPosition","an HTMLElement or the Window",a)},getXStepLengthCalculator:function(a,b){a=void 0===
a?uss._pageScroller:a;b=void 0===b?!1:b;if(a===window||a instanceof HTMLElement){var c=uss._containersData.get(a)||[];return b?c[14]:c[12]}DEFAULT_ERROR_LOGGER("getXStepLengthCalculator","an HTMLElement or the Window",a)},getYStepLengthCalculator:function(a,b){a=void 0===a?uss._pageScroller:a;b=void 0===b?!1:b;if(a===window||a instanceof HTMLElement){var c=uss._containersData.get(a)||[];return b?c[15]:c[13]}DEFAULT_ERROR_LOGGER("getYStepLengthCalculator","an HTMLElement or the Window",a)},getXStepLength:function(){return uss._xStepLength},
getYStepLength:function(){return uss._yStepLength},getMinAnimationFrame:function(){return uss._minAnimationFrame},getWindowHeight:function(){return uss._windowHeight},getWindowWidth:function(){return uss._windowWidth},getScrollbarsMaxDimension:function(){return uss._scrollbarsMaxDimension},getPageScroller:function(){return uss._pageScroller},getReducedMotionState:function(){return uss._reducedMotion},getDebugMode:function(){return uss._debugMode},setXStepLengthCalculator:function(a,b,c){b=void 0===
b?uss._pageScroller:b;c=void 0===c?!1:c;if("function"!==typeof a)DEFAULT_ERROR_LOGGER("setXStepLengthCalculator","a function",a);else if(b===window||b instanceof HTMLElement){var d=a(DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,0,0,DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,0,DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,b);Number.isFinite(d)?(d=uss._containersData.get(b)||[],c?d[14]=a:(d[12]=a,d[14]=null),uss._containersData.set(b,d)):DEFAULT_ERROR_LOGGER("setXStepLengthCalculator","a function which returns a valid step value",
a.name||"Anonymous function")}else DEFAULT_ERROR_LOGGER("setXStepLengthCalculator","an HTMLElement or the Window",b)},setYStepLengthCalculator:function(a,b,c){b=void 0===b?uss._pageScroller:b;c=void 0===c?!1:c;if("function"!==typeof a)DEFAULT_ERROR_LOGGER("setYStepLengthCalculator","a function",a);else if(b===window||b instanceof HTMLElement){var d=a(DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,0,0,DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,0,DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,b);Number.isFinite(d)?(d=uss._containersData.get(b)||
[],c?d[15]=a:(d[13]=a,d[15]=null),uss._containersData.set(b,d)):DEFAULT_ERROR_LOGGER("setYStepLengthCalculator","a function which returns a valid step value",a.name||"Anonymous function")}else DEFAULT_ERROR_LOGGER("setYStepLengthCalculator","an HTMLElement or the Window",b)},setStepLengthCalculator:function(a,b,c){b=void 0===b?uss._pageScroller:b;c=void 0===c?!1:c;if("function"!==typeof a)DEFAULT_ERROR_LOGGER("setStepLengthCalculator","a function",a);else if(b===window||b instanceof HTMLElement){var d=
a(DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,0,0,DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,0,DEFAULT_SCROLL_CALCULATOR_TEST_VALUE,b);Number.isFinite(d)?(d=uss._containersData.get(b)||[],c?(d[14]=a,d[15]=a):(d[12]=a,d[13]=a,d[14]=null,d[15]=null),uss._containersData.set(b,d)):DEFAULT_ERROR_LOGGER("setStepLengthCalculator","a function which returns a valid step value",a.name||"Anonymous function")}else DEFAULT_ERROR_LOGGER("setStepLengthCalculator","an HTMLElement or the Window",b)},setXStepLength:function(a){!Number.isFinite(a)||
0>=a?DEFAULT_ERROR_LOGGER("setXStepLength","a positive number",a):uss._xStepLength=a},setYStepLength:function(a){!Number.isFinite(a)||0>=a?DEFAULT_ERROR_LOGGER("setYStepLength","a positive number",a):uss._yStepLength=a},setStepLength:function(a){!Number.isFinite(a)||0>=a?DEFAULT_ERROR_LOGGER("setStepLength","a positive number",a):(uss._xStepLength=a,uss._yStepLength=a)},setMinAnimationFrame:function(a){!Number.isFinite(a)||0>=a?DEFAULT_ERROR_LOGGER("setMinAnimationFrame","a positive number",a):uss._minAnimationFrame=
a},setPageScroller:function(a){a===window||a instanceof HTMLElement?uss._pageScroller=a:DEFAULT_ERROR_LOGGER("setPageScroller","an HTMLElement or the Window",a)},setDebugMode:function(a){a=void 0===a?"":a;if("string"!==typeof a){var b=null;/disabled/i.test(uss._debugMode)&&(b=uss._debugMode,uss._debugMode="legacy");DEFAULT_ERROR_LOGGER("setDebugMode",'"disabled", "legacy" or any other string',a);b&&(uss._debugMode=b)}else uss._debugMode=a},calcXStepLength:function(a){if(!Number.isFinite(a)||0>a)throw DEFAULT_ERROR_LOGGER("calcXStepLength",
"a positive number",a),"USS fatal error (execution stopped)";return a>=uss._minAnimationFrame*uss._xStepLength?uss._xStepLength:Math.ceil(a/uss._minAnimationFrame)},calcYStepLength:function(a){if(!Number.isFinite(a)||0>a)throw DEFAULT_ERROR_LOGGER("calcYStepLength","a positive number",a),"USS fatal error (execution stopped)";return a>=uss._minAnimationFrame*uss._yStepLength?uss._yStepLength:Math.ceil(a/uss._minAnimationFrame)},calcScrollbarsDimensions:function(a){if(a===window)return[0,0];if(!(a instanceof
HTMLElement))throw DEFAULT_ERROR_LOGGER("calcScrollbarsDimensions","an HTMLElement or the Window",a),"USS fatal error (execution stopped)";if(0===uss._scrollbarsMaxDimension)return[0,0];var b=[],c=window.getComputedStyle(a),d=Number.parseInt(c.width),g=Number.parseInt(c.height),e=a.clientWidth,l=a.clientHeight,k=a.style.overflowX,f=a.style.overflowY,m=a.scrollLeft,h=a.scrollTop;a.style.overflowX="hidden";a.style.overflowY="hidden";b[0]=Number.parseInt(c.width)-d;b[1]=Number.parseInt(c.height)-g;0===
b[0]?b[0]=a.clientWidth-e:0>b[0]&&(b[0]=0);0===b[1]?b[1]=a.clientHeight-l:0>b[1]&&(b[1]=0);a.style.overflowX=k;a.style.overflowY=f;a.scrollLeft=m;a.scrollTop=h;return b},calcBordersDimensions:function(a){if(a===window)return[0,0,0,0];if(!(a instanceof HTMLElement))throw DEFAULT_ERROR_LOGGER("calcBordersDimensions","an HTMLElement or the Window",a),"USS fatal error (execution stopped)";a=window.getComputedStyle(a);return[Number.parseInt(a.borderTopWidth),Number.parseInt(a.borderRightWidth),Number.parseInt(a.borderBottomWidth),
Number.parseInt(a.borderLeftWidth)]},getScrollXCalculator:function(a){a=void 0===a?uss._pageScroller:a;return a===window?function(){return window.scrollX}:a instanceof HTMLElement?function(){return a.scrollLeft}:function(){DEFAULT_ERROR_LOGGER("getScrollXCalculator","an HTMLElement or the Window",a);throw"USS fatal error (execution stopped)";}},getScrollYCalculator:function(a){a=void 0===a?uss._pageScroller:a;return a===window?function(){return window.scrollY}:a instanceof HTMLElement?function(){return a.scrollTop}:
function(){DEFAULT_ERROR_LOGGER("getScrollYCalculator","an HTMLElement or the Window",a);throw"USS fatal error (execution stopped)";}},getMaxScrollX:function(a){a=void 0===a?uss._pageScroller:a;if(a===window){var b=window.scrollX;a.scroll(1073741824,window.scrollY);var c=window.scrollX;a.scroll(b,window.scrollY);return c}if(a===document.documentElement||a===document.body)return b=a.scrollLeft,a.scrollLeft=1073741824,c=a.scrollLeft,a.scrollLeft=b,c;if(a instanceof HTMLElement)return a.scrollWidth-
a.clientWidth;DEFAULT_ERROR_LOGGER("getMaxScrollX","an HTMLElement or the Window",a)},getMaxScrollY:function(a){a=void 0===a?uss._pageScroller:a;if(a===window){var b=window.scrollY;a.scroll(window.scrollX,1073741824);var c=window.scrollY;a.scroll(window.scrollX,b);return c}if(a===document.documentElement||a===document.body)return b=a.scrollTop,a.scrollTop=1073741824,c=a.scrollTop,a.scrollTop=b,c;if(a instanceof HTMLElement)return a.scrollHeight-a.clientHeight;DEFAULT_ERROR_LOGGER("getMaxScrollY",
"an HTMLElement or the Window",a)},getXScrollableParent:function(a,b){b=void 0===b?!1:b;if(a===window)return null;try{var c=window.getComputedStyle(a);if("fixed"===c.position)return null;for(var d="absolute"!==c.position,g=b?/(auto|scroll|hidden)/:/(auto|scroll)/,e=a.parentElement;e;){c=window.getComputedStyle(e);if((d||"static"!==c.position)&&g.test(c.overflowX)&&e.scrollWidth>e.clientWidth)return e;if("fixed"===c.position)return null;e=e.parentElement}}catch(l){DEFAULT_ERROR_LOGGER("getXScrollableParent",
"an HTMLElement or the Window",a)}return window},getYScrollableParent:function(a,b){b=void 0===b?!1:b;if(a===window)return null;try{var c=window.getComputedStyle(a);if("fixed"===c.position)return null;for(var d="absolute"!==c.position,g=b?/(auto|scroll|hidden)/:/(auto|scroll)/,e=a.parentElement;e;){c=window.getComputedStyle(e);if((d||"static"!==c.position)&&g.test(c.overflowY)&&e.scrollHeight>e.clientHeight)return e;if("fixed"===c.position)return null;e=e.parentElement}}catch(l){DEFAULT_ERROR_LOGGER("getYScrollableParent",
"an HTMLElement or the Window",a)}return window},getScrollableParent:function(a,b){b=void 0===b?!1:b;if(a===window)return null;try{var c=window.getComputedStyle(a);if("fixed"===c.position)return null;for(var d="absolute"!==c.position,g=b?/(auto|scroll|hidden)/:/(auto|scroll)/,e=a.parentElement;e;){c=window.getComputedStyle(e);if((d||"static"!==c.position)&&g.test(c.overflow)&&(e.scrollWidth>e.clientWidth||e.scrollHeight>e.clientHeight))return e;if("fixed"===c.position)return null;e=e.parentElement}}catch(l){DEFAULT_ERROR_LOGGER("getScrollableParent",
"an HTMLElement or the Window",a)}return window},getAllScrollableParents:function(a,b,c){b=void 0===b?!1:b;var d=[];if(a===window)return d;try{var g=window.getComputedStyle(a);if("fixed"===g.position)return d;c="function"===typeof c?c:function(){};var e="absolute"!==g.position;b=b?/(auto|scroll|hidden)/:/(auto|scroll)/;for(var l=!0,k=a.parentElement;k;){g=window.getComputedStyle(k);if((e||"static"!==g.position)&&b.test(g.overflow)&&(k.scrollWidth>k.clientWidth||k.scrollHeight>k.clientHeight)){if(k===
document.body||k===document.documentElement)l=!1;d.push(k);c(k)}if("fixed"===g.position)return d;k=k.parentElement}l&&(d.push(window),c(window))}catch(f){DEFAULT_ERROR_LOGGER("getAllScrollableParents","an HTMLElement or the Window",a)}return d},scrollXTo:function(a,b,c){function d(h){var p=f[2],r=f[4],n=g(),q=(p-n)*r;if(0>=q)f[0]=null,f[14]=null,"function"===typeof f[10]&&window.requestAnimationFrame(f[10]);else{try{var v=f[0];m=f["function"===typeof f[14]?14:12](q,f[8],h,f[6],n,p,b);if(v!==f[0])return;
if(p!==f[2]){f[0]=window.requestAnimationFrame(d);return}Number.isFinite(m)||(DEFAULT_WARNING_LOGGER(m,"is not a valid step length"),m=uss.calcXStepLength(e))}catch(t){m=uss.calcXStepLength(e)}q<=m?(f[0]=null,f[14]=null,k(p),"function"===typeof f[10]&&window.requestAnimationFrame(f[10])):(k(n+m*r),0!==m&&n===g()?(f[0]=null,f[14]=null,"function"===typeof f[10]&&window.requestAnimationFrame(f[10])):f[0]=window.requestAnimationFrame(d))}}b=void 0===b?uss._pageScroller:b;if(Number.isFinite(a))if(b===
window||b instanceof HTMLElement)if(1>uss.getMaxScrollX(b))uss.stopScrollingX(b,c);else{var g=uss.getScrollXCalculator(b),e=a-g(),l=0<e?1:-1;e*=l;if(1>e)uss.stopScrollingX(b,c);else{var k=b!==window?function(h){return b.scrollLeft=h}:function(h){return b.scroll(h,window.scrollY)};if(uss._reducedMotion)k(a),uss.stopScrollingX(b,c);else{var f=uss._containersData.get(b)||[];f[2]=a;f[4]=l;f[6]=e;f[8]=performance.now();f[10]=c;if("number"!==typeof f[0]){f[0]=window.requestAnimationFrame(d);uss._containersData.set(b,
f);var m}}}}else DEFAULT_ERROR_LOGGER("scrollXTo","an HTMLElement or the Window",b);else DEFAULT_ERROR_LOGGER("scrollXTo","a number as the finalXPosition",a)},scrollYTo:function(a,b,c){function d(h){var p=f[3],r=f[5],n=g(),q=(p-n)*r;if(0>=q)f[1]=null,f[15]=null,"function"===typeof f[11]&&window.requestAnimationFrame(f[11]);else{try{var v=f[1];m=f["function"===typeof f[15]?15:13](q,f[9],h,f[7],n,p,b);if(v!==f[1])return;if(p!==f[3]){f[1]=window.requestAnimationFrame(d);return}Number.isFinite(m)||(DEFAULT_WARNING_LOGGER(m,
"is not a valid step length"),m=uss.calcYStepLength(e))}catch(t){m=uss.calcYStepLength(e)}q<=m?(f[1]=null,f[15]=null,k(p),"function"===typeof f[11]&&window.requestAnimationFrame(f[11])):(k(n+m*r),0!==m&&n===g()?(f[1]=null,f[15]=null,"function"===typeof f[11]&&window.requestAnimationFrame(f[11])):f[1]=window.requestAnimationFrame(d))}}b=void 0===b?uss._pageScroller:b;if(Number.isFinite(a))if(b===window||b instanceof HTMLElement)if(1>uss.getMaxScrollY(b))uss.stopScrollingY(b,c);else{var g=uss.getScrollYCalculator(b),
e=a-g(),l=0<e?1:-1;e*=l;if(1>e)uss.stopScrollingY(b,c);else{var k=b!==window?function(h){return b.scrollTop=h}:function(h){return b.scroll(window.scrollX,h)};if(uss._reducedMotion)k(a),uss.stopScrollingY(b,c);else{var f=uss._containersData.get(b)||[];f[3]=a;f[5]=l;f[7]=e;f[9]=performance.now();f[11]=c;if("number"!==typeof f[1]){f[1]=window.requestAnimationFrame(d);uss._containersData.set(b,f);var m}}}}else DEFAULT_ERROR_LOGGER("scrollYTo","an HTMLElement or the Window",b);else DEFAULT_ERROR_LOGGER("scrollYTo",
"a number as the finalYPosition",a)},scrollXBy:function(a,b,c,d){b=void 0===b?uss._pageScroller:b;d=void 0===d?!0:d;if(Number.isFinite(a))if(b===window||b instanceof HTMLElement){if(!d&&(d=uss._containersData.get(b)||[],"number"===typeof d[0])){d[10]=c;if(0===a)return;d[2]+=a;a=d[2]-uss.getScrollXCalculator(b)();d[4]=0<a?1:-1;d[6]=a*d[4];d[8]=performance.now();return}uss.scrollXTo(uss.getScrollXCalculator(b)()+a,b,c)}else DEFAULT_ERROR_LOGGER("scrollXBy","an HTMLElement or the Window",b);else DEFAULT_ERROR_LOGGER("scrollXBy",
"a number as the deltaX",a)},scrollYBy:function(a,b,c,d){b=void 0===b?uss._pageScroller:b;d=void 0===d?!0:d;if(Number.isFinite(a))if(b===window||b instanceof HTMLElement){if(!d&&(d=uss._containersData.get(b)||[],"number"===typeof d[1])){d[11]=c;if(0===a)return;d[3]+=a;a=d[3]-uss.getScrollYCalculator(b)();d[5]=0<a?1:-1;d[7]=a*d[5];d[9]=performance.now();return}uss.scrollYTo(uss.getScrollYCalculator(b)()+a,b,c)}else DEFAULT_ERROR_LOGGER("scrollYBy","an HTMLElement or the Window",b);else DEFAULT_ERROR_LOGGER("scrollYBy",
"a number as the deltaY",a)},scrollTo:function(a,b,c,d){c=void 0===c?uss._pageScroller:c;if(Number.isFinite(a))if(Number.isFinite(b))if(c===window||c instanceof HTMLElement){var g={__requiredSteps:1,__currentSteps:0,__function:"function"===typeof d?function(){g.__currentSteps<g.__requiredSteps?g.__currentSteps++:d()}:null};uss.scrollXTo(a,c,g.__function);uss.scrollYTo(b,c,g.__function)}else DEFAULT_ERROR_LOGGER("scrollTo","an HTMLElement or the Window",c);else DEFAULT_ERROR_LOGGER("scrollTo","a number as the finalYPosition",
b);else DEFAULT_ERROR_LOGGER("scrollTo","a number as the finalXPosition",a)},scrollBy:function(a,b,c,d,g){c=void 0===c?uss._pageScroller:c;g=void 0===g?!0:g;if(Number.isFinite(a))if(Number.isFinite(b))if(c===window||c instanceof HTMLElement){if(g){g=uss.getScrollXCalculator(c)();var e=uss.getScrollYCalculator(c)()}else e=uss._containersData.get(c)||[],g="number"===typeof e[0]?e[2]:uss.getScrollXCalculator(c)(),e="number"===typeof e[1]?e[3]:uss.getScrollYCalculator(c)();uss.scrollTo(g+a,e+b,c,d)}else DEFAULT_ERROR_LOGGER("scrollBy",
"an HTMLElement or the Window",c);else DEFAULT_ERROR_LOGGER("scrollBy","a number as the deltaY",b);else DEFAULT_ERROR_LOGGER("scrollBy","a number as the deltaX",a)},scrollIntoView:function(a,b,c,d,g){function e(){var r=uss.calcScrollbarsDimensions(h),n=uss.calcBordersDimensions(h),q=h!==window?h.getBoundingClientRect():{left:0,top:0,width:uss._windowWidth,height:uss._windowHeight},v=q.width,t=q.height,w=p.getBoundingClientRect(),z=w.width,u=w.height,y=w.left-q.left;q=w.top-q.top;if("nearest"===b){w=
Math.abs(v-y-z);var x=Math.abs(.5*(v-z)-y);m=(0<y?y:-y)<x?!0:w<x?!1:null}"nearest"===c&&(w=Math.abs(t-q-u),x=Math.abs(.5*(t-u)-q),f=(0<q?q:-q)<x?!0:w<x?!1:null);uss.scrollBy(y-(!0===m?n[3]:!1===m?v-z-r[0]-n[1]:.5*(v-z-r[0]-n[1]+n[3])),q-(!0===f?n[0]:!1===f?t-u-r[1]-n[2]:.5*(t-u-r[1]-n[2]+n[0])),h,function(){p===a?"function"===typeof d&&window.requestAnimationFrame(d):(l--,h=k[l],p=1>l?a:k[l-1],e())})}b=void 0===b?!0:b;c=void 0===c?!0:c;g=void 0===g?!1:g;if(a===window)"function"===typeof d&&window.requestAnimationFrame(d);
else if(a instanceof HTMLElement){var l=-1,k=uss.getAllScrollableParents(a,g,function(){return l++});if(0>l)"function"===typeof d&&window.requestAnimationFrame(d);else{var f=c,m=b;var h=k[l];var p=1>l?a:k[l-1];e()}}else DEFAULT_ERROR_LOGGER("scrollIntoView","an HTMLElement or the Window",a)},scrollIntoViewIfNeeded:function(a,b,c,d){function g(){var p=uss.calcScrollbarsDimensions(m),r=uss.calcBordersDimensions(m),n=m!==window?m.getBoundingClientRect():{left:0,top:0,width:uss._windowWidth,height:uss._windowHeight},
q=n.width,v=n.height,t=h.getBoundingClientRect(),w=t.width,z=t.height,u=t.left-n.left;n=t.top-n.top;t=0>=u&&0<=u+w-q+p[0];var y=0>=n&&0<=n+z-v+p[1],x=h===a;t=-1<u&&1>u+w-q+p[0]||x&&t;y=-1<n&&1>n+z-v+p[1]||x&&y;if(t&&y)x?"function"===typeof c&&window.requestAnimationFrame(c):(e--,m=l[e],h=1>e?a:l[e-1],g());else{if(x&&!0===b)y=t=!1;else{if(!t){x=Math.abs(q-u-w);var A=Math.abs(.5*(q-w)-u);f=(0<u?u:-u)<A?!0:x<A?!1:null}y||(x=Math.abs(v-n-z),A=Math.abs(.5*(v-z)-n),k=(0<n?n:-n)<A?!0:x<A?!1:null)}uss.scrollBy(u-
(t?u:!0===f?r[3]:!1===f?q-w-p[0]-r[1]:.5*(q-w-p[0]-r[1]+r[3])),n-(y?n:!0===k?r[0]:!1===k?v-z-p[1]-r[2]:.5*(v-z-p[1]-r[2]+r[0])),m,function(){h===a?"function"===typeof c&&window.requestAnimationFrame(c):(e--,m=l[e],h=1>e?a:l[e-1],g())})}}b=void 0===b?!0:b;d=void 0===d?!1:d;if(a===window)"function"===typeof c&&window.requestAnimationFrame(c);else if(a instanceof HTMLElement){var e=-1,l=uss.getAllScrollableParents(a,d,function(){return e++});if(0>e)"function"===typeof c&&window.requestAnimationFrame(c);
else{var k=null,f=null;var m=l[e];var h=1>e?a:l[e-1];g()}}else DEFAULT_ERROR_LOGGER("scrollIntoView","an HTMLElement or the Window",a)},stopScrollingX:function(a,b){a=void 0===a?uss._pageScroller:a;if(a===window||a instanceof HTMLElement){var c=uss._containersData.get(a)||[];window.cancelAnimationFrame(c[0]);c[0]=null;c[14]=null;"function"===typeof b&&window.requestAnimationFrame(b)}else DEFAULT_ERROR_LOGGER("stopScrollingX","an HTMLElement or the Window",a)},stopScrollingY:function(a,b){a=void 0===
a?uss._pageScroller:a;if(a===window||a instanceof HTMLElement){var c=uss._containersData.get(a)||[];window.cancelAnimationFrame(c[1]);c[1]=null;c[15]=null;"function"===typeof b&&window.requestAnimationFrame(b)}else DEFAULT_ERROR_LOGGER("stopScrollingY","an HTMLElement or the Window",a)},stopScrolling:function(a,b){a=void 0===a?uss._pageScroller:a;if(a===window||a instanceof HTMLElement){var c=uss._containersData.get(a)||[];window.cancelAnimationFrame(c[0]);window.cancelAnimationFrame(c[1]);c[0]=null;
c[1]=null;c[14]=null;c[15]=null;"function"===typeof b&&window.requestAnimationFrame(b)}else DEFAULT_ERROR_LOGGER("stopScrolling","an HTMLElement or the Window",a)},hrefSetup:function(a,b,c,d,g,e){a=void 0===a?!0:a;b=void 0===b?!0:b;g=void 0===g?!1:g;e=void 0===e?!1:e;var l="function"===typeof c?c:function(){},k=document.links;c=document.URL.split("#")[0];var f=e&&!!(window.history&&window.history.pushState&&window.history.scrollRestoration);f&&(window.history.scrollRestoration="manual",window.addEventListener("popstate",
function(){var h=document.URL.split("#")[1];h?(h=document.getElementById(h)||document.querySelector("a[name='"+h+"']"),null!==h&&!1!==l(window,h)&&uss.scrollIntoView(h,a,b,d,g)):!1!==l(window,uss._pageScroller)&&uss.scrollTo(0,0,uss._pageScroller,d)},{passive:!0}),window.addEventListener("unload",function(h){h.preventDefault()},{passive:!1}));e={};k=$jscomp.makeIterator(k);for(var m=k.next();!m.done;e={$jscomp$loop$prop$_pageLink$11:e.$jscomp$loop$prop$_pageLink$11,$jscomp$loop$prop$_elementToReach$12:e.$jscomp$loop$prop$_elementToReach$12,
$jscomp$loop$prop$_pageLinkParts$13:e.$jscomp$loop$prop$_pageLinkParts$13},m=k.next())e.$jscomp$loop$prop$_pageLink$11=m.value,e.$jscomp$loop$prop$_pageLinkParts$13=e.$jscomp$loop$prop$_pageLink$11.href.split("#"),e.$jscomp$loop$prop$_pageLinkParts$13[0]===c&&(""===e.$jscomp$loop$prop$_pageLinkParts$13[1]?e.$jscomp$loop$prop$_pageLink$11.addEventListener("click",function(h){return function(p){p.preventDefault();p.stopPropagation();!1!==l(h.$jscomp$loop$prop$_pageLink$11,uss._pageScroller)&&(f&&"#"!==
window.history.state&&window.history.pushState("#","","#"),uss.scrollTo(0,0,uss._pageScroller,d))}}(e),{passive:!1}):(e.$jscomp$loop$prop$_elementToReach$12=document.getElementById(e.$jscomp$loop$prop$_pageLinkParts$13[1])||document.querySelector("a[name='"+e.$jscomp$loop$prop$_pageLinkParts$13[1]+"']"),null===e.$jscomp$loop$prop$_elementToReach$12?DEFAULT_WARNING_LOGGER(e.$jscomp$loop$prop$_pageLinkParts$13[1],"is not a valid anchor's destination"):e.$jscomp$loop$prop$_pageLink$11.addEventListener("click",
function(h){return function(p){p.preventDefault();p.stopPropagation();!1!==l(h.$jscomp$loop$prop$_pageLink$11,h.$jscomp$loop$prop$_elementToReach$12)&&(f&&window.history.state!==h.$jscomp$loop$prop$_pageLinkParts$13[1]&&window.history.pushState(h.$jscomp$loop$prop$_pageLinkParts$13[1],"","#"+h.$jscomp$loop$prop$_pageLinkParts$13[1]),uss.scrollIntoView(h.$jscomp$loop$prop$_elementToReach$12,a,b,d,g))}}(e),{passive:!1})))}};
window.addEventListener("resize",function(){uss._windowHeight=window.innerHeight;uss._windowWidth=window.innerWidth},{passive:!0});window.addEventListener("load",function calcMaxScrollbarsDimensions(){var b=document.createElement("div");b.style.overflowX="scroll";document.body.appendChild(b);uss._scrollbarsMaxDimension=b.offsetHeight-b.clientHeight;document.body.removeChild(b);window.removeEventListener("load",calcMaxScrollbarsDimensions)},{passive:!0});
try{window.matchMedia("(prefers-reduced-motion)").addEventListener("change",function(){uss._reducedMotion=!uss._reducedMotion;var a=uss._containersData.keys();a=$jscomp.makeIterator(a);for(var b=a.next();!b.done;b=a.next())uss.stopScrolling(b.value)},{passive:!0})}catch(a){window.matchMedia("(prefers-reduced-motion)").addListener(function(){uss._reducedMotion=!uss._reducedMotion;var b=uss._containersData.keys();b=$jscomp.makeIterator(b);for(var c=b.next();!c.done;c=b.next())uss.stopScrolling(c.value)},
{passive:!0})};