var bnd = function(c, b, a) {
    c.addEventListener(b, a, false)
};
var ubnd = function(c, b, a) {
    c.removeEventListener(b, a, false)
};
var m = function(f, d, g) {
    d = document;
    g = d.createElement("p");
    g.innerHTML = f;
    f = d.createDocumentFragment();
    while (d = g.firstChild) {
        f.appendChild(d)
    }
    return f
};
var $ = function(d, c) {
    d = d.match(/^(\W)?(.*)/);
    return (c || document)["getElement" + (d[1] ? d[1] == "#" ? "ById" : "sByClassName" : "sByTagName")](d[2])
};
var j = function(b) {
    for (b = 0; b < 4; b++) {
        try {
            return b ? new ActiveXObject([, "Msxml2", "Msxml3", "Microsoft"][b] + ".XMLHTTP") : new XMLHttpRequest
        } catch (c) {}
    }
};

function domForEach(b, a) {
    return Array.prototype.forEach.call(b, a)
}
e = function(b) {
    return document.createElement(b)
};

function onLoad(b) {
    var a = window.onload;
    if (typeof a != "function") {
        window.onload = b
    } else {
        window.onload = function() {
            a();
            b()
        }
    }
}

function addClass(b, a) {
    b.className += " " + a
}

function removeClass(f, c) {
    var b = f.className.split(/\s+/),
        a = b.length;
    for (var d = 0; d < a; d++) {
        if (b[d] === c) {
            b.splice(d, 1)
        }
    }
    f.className = b.join(" ");
    return b.length != a
}

function toggleClass(b, a) {
    if (!removeClass(b, a)) {
        addClass(b, a)
    }
}

function ajaxReq(h, a, g, c) {
    var f = j();
    f.open(h, a, true);
    var d = setTimeout(function() {
        f.abort();
        console.log("XHR abort:", h, a);
        f.status = 599;
        f.responseText = "request time-out"
    }, 9000);
    f.onreadystatechange = function() {
        if (f.readyState != 4) {
            return
        }
        clearTimeout(d);
        if (f.status >= 200 && f.status < 300) {
            g(f.responseText)
        } else {
            console.log("XHR ERR :", h, a, "->", f.status, f.responseText, f);
            c(f.status, f.responseText)
        }
    };
    try {
        f.send()
    } catch (b) {
        console.log("XHR EXC :", h, a, "->", b);
        c(599, b)
    }
}

function dispatchJson(f, d, c) {
    var a;
    try {
        a = JSON.parse(f)
    } catch (b) {
        console.log("JSON parse error: " + b + ". In: " + f);
        c(500, "JSON parse error: " + b);
        return
    }
    d(a)
}

function ajaxJson(d, a, c, b) {
    ajaxReq(d, a, function(f) {
        dispatchJson(f, c, b)
    }, b)
}

function ajaxSpin(d, a, c, b) {
    $("#spinner").removeAttribute("hidden");
    ajaxReq(d, a, function(f) {
        $("#spinner").setAttribute("hidden", "");
        c(f)
    }, function(f, g) {
        $("#spinner").setAttribute("hidden", "");
        b(f, g)
    })
}

function ajaxJsonSpin(d, a, c, b) {
    ajaxSpin(d, a, function(f) {
        dispatchJson(f, c, b)
    }, b)
}

function hidePopup(a) {
    addClass(a, "popup-hidden");
    addClass(a.parentNode, "popup-target")
}
onLoad(function() {
    var b = $("#layout");
    var f = b.childNodes[0];
    b.insertBefore(m('<div id="spinner" class="spinner" hidden></div>'), f);
    b.insertBefore(m('<div id="messages"><div id="warning" hidden></div><div id="notification" hidden></div></div>'), f);
    b.insertBefore(m('<a href="#menu" id="menuLink" class="menu-link"><span></span></a>'), f);
    var d = m('<div id="menu">      <div class="pure-menu">        <a class="pure-menu-heading" href=".">        <img src="/logo.ico" height="128"></a>        <ul id="menu-list" class="pure-menu-list"></ul>        <p class="sub-menu-header">settings</p>        <ul id="menu-list-settings" class="pure-menu-list"></ul>      </div>      <h3 id="version"></h3>    </div>    ');
    b.insertBefore(d, f);
    var g = $("#menuLink"),
        d = $("#menu");
    bnd(g, "click", function(i) {
        var h = "active";
        i.preventDefault();
        toggleClass(b, h);
        toggleClass(d, h);
        toggleClass(g, h);
        setTimeout(a, 10000)
    });
    domForEach($(".popup"), function(h) {
        hidePopup(h)
    });
    var c = function() {
        //ajaxJson("GET", "/menu", function(o) {
        //    var n = "",
        //        q = "",
        //        p = window.location.pathname;
        //    for (var l = 0; l < o.menu.length; l += 2) {
        //        var k = o.menu[l + 1];
        //        if (l < 4) {
        //            n = n.concat(' <li class="pure-menu-item' + (p === k ? " pure-menu-selected" : "") + '"><a href="' + k + '" class="pure-menu-link">' + o.menu[l] + "</a></li>")
        //        } else {
        //            q = q.concat(' <li class="pure-menu-item' + (p === k ? " pure-menu-selected" : "") + '"><a href="' + k + '" class="pure-menu-link">' + o.menu[l] + "</a></li>")
        //        }
        //    }
        //    $("#menu-list").innerHTML = n;
        //    $("#menu-list-settings").innerHTML = q;
        //    var h = $("#version");
        //    if (h != null) {
        //        h.innerHTML = o.version
        //    }
        //}, function() {
        //    setTimeout(c, 1000)
        //})
    };
    c();
    bnd($("#main"), "click", function() {
        a()
    });
    var a = function() {
        removeClass(b, "active");
        removeClass(d, "active");
        removeClass(g, "active")
    }
});

function showWifiInfo(f) {
    Object.keys(f).forEach(function(h) {
        el = $("#wifi-" + h);
        if (el != null) {
            if (el.nodeName === "INPUT") {
                el.value = f[h]
            } else {
                el.innerHTML = f[h]
            }
        }
    });
    var d = $("#dhcp-r" + f.dhcp);
    if (d) {
        d.click()
    }
    var b = $("#wifi-spinner");
    if (b != null) {
        b.setAttribute("hidden", "")
    }
    $("#wifi-table").removeAttribute("hidden");
    var c = $("#change-hostname-input");
    if (c != null) {
        c.value = f.hostname
    }
    currAp = f.ssid;
    var g = $("#wifi-warn");
    if (g != null) {
        var a = g.children[0];
        if (currAp == "" || currAp == null) {
            a.onclick = "";
            bnd(a, "click", function(h) {
                showConfigWiFiMessage()
            })
        }
    }
}

function getWifiInfo() {
    ajaxJson("GET", "/wifi/info", showWifiInfo, function(b, a) {
        window.setTimeout(getWifiInfo, 1000)
    })
}

function setEditToClick(a, b) {
    domForEach($("." + a), function(c) {
        if (c.children.length > 0) {
            domForEach(c.children, function(d) {
                if (d.nodeName === "INPUT") {
                    d.value = b
                } else {
                    if (d.nodeName !== "DIV") {
                        d.innerHTML = b
                    }
                }
            })
        } else {
            c.innerHTML = b
        }
    })
}

function showSystemInfo(a) {
    Object.keys(a).forEach(function(b) {
        setEditToClick("system-" + b, a[b])
    });
    currAp = a.ssid
}

function getSystemInfo() {
    ajaxJson("GET", "/system/info", showSystemInfo, function(b, a) {
        window.setTimeout(getSystemInfo, 1000)
    })
}

function makeAjaxInput(a, b) {
    domForEach($("." + a + "-" + b), function(i) {
        var f = $(".edit-on", i);
        var d = $(".edit-off", i)[0];
        var c = "/" + a + "/update?" + b;
        if (d === undefined || f == undefined) {
            return
        }
        var h = function() {
            d.setAttribute("hidden", "");
            domForEach(f, function(k) {
                k.removeAttribute("hidden")
            });
            f[0].select();
            return false
        };
        var g = function(k) {
            ajaxSpin("POST", c + "=" + k, function() {
                domForEach(f, function(l) {
                    l.setAttribute("hidden", "")
                });
                d.removeAttribute("hidden");
                setEditToClick(a + "-" + b, k);
                showNotification(b + " changed to " + k)
            }, function() {
                showWarning(b + " change failed")
            });
            return false
        };
        bnd(d, "click", function() {
            return h()
        });
        bnd(f[0], "blur", function() {
            return g(f[0].value)
        });
        bnd(f[0], "keyup", function(k) {
            if ((k || window.event).keyCode == 13) {
                return g(f[0].value)
            }
        })
    })
}

function showWarning(b) {
    var a = $("#warning");
    a.innerHTML = b;
    a.removeAttribute("hidden");
    window.scrollTo(0, 0)
}

function hideWarning() {
    el = $("#warning").setAttribute("hidden", "")
}
var notifTimeout = null;

function showNotification(b) {
    var a = $("#notification");
    a.innerHTML = b;
    a.removeAttribute("hidden");
    if (notifTimeout != null) {
        clearTimeout(notifTimeout)
    }
    notifTimout = setTimeout(function() {
        a.setAttribute("hidden", "");
        notifTimout = null
    }, 4000)
}
var pinPresets = {
    "esp-01": [0, -1, 2, -1, 0, 1],
    "esp-12": [12, 14, 0, 2, 0, 1],
    "esp-12 swap": [1, 3, 0, 2, 1, 1],
    "esp-bridge": [12, 13, 0, 14, 0, 0],
    "wifi-link-12": [1, 3, 0, 2, 1, 0]
};

function createPresets(c) {
    for (var d in pinPresets) {
        var a = m('<option value="' + d + '">' + d + "</option>");
        c.appendChild(a)
    }

    function b(h) {
        var f = pinPresets[h];
        if (f === undefined) {
            return f
        }

        function g(l, i) {
            $("#pin-" + l).value = i
        }
        g("reset", f[0]);
        g("isp", f[1]);
        g("conn", f[2]);
        g("ser", f[3]);
        g("swap", f[4]);
        $("#pin-rxpup").checked = !!f[5];
        c.value = 0
    }
    bnd(c, "change", function(f) {
        f.preventDefault();
        b(c.value)
    })
}

function displayPins(b) {
    function a(f, c) {
        var g = $("#pin-" + f);
        addClass(g, "pure-button");
        g.innerHTML = "";
        [-1, 0, 1, 2, 3, 4, 5, 12, 13, 14, 15].forEach(function(k) {
            var h = document.createElement("option");
            h.value = k;
            if (k >= 0) {
                h.innerHTML = "gpio" + k
            } else {
                h.innerHTML = "disabled"
            }
            if (k === 1) {
                h.innerHTML += "/TX0"
            }
            if (k === 2) {
                h.innerHTML += "/TX1"
            }
            if (k === 3) {
                h.innerHTML += "/RX0"
            }
            if (k == c) {
                h.selected = true
            }
            g.appendChild(h)
        });
        var d = $(".popup", g.parentNode);
        if (d !== undefined) {
            hidePopup(d[0])
        }
    }
    a("reset", b.reset);
    a("isp", b.isp);
    a("conn", b.conn);
    a("ser", b.ser);
    $("#pin-swap").value = b.swap;
    $("#pin-rxpup").checked = !!b.rxpup;
    createPresets($("#pin-preset"));
    $("#pin-spinner").setAttribute("hidden", "");
    $("#pin-table").removeAttribute("hidden")
}

function fetchPins() {
    ajaxJson("GET", "/pins", displayPins, function() {
        window.setTimeout(fetchPins, 1000)
    })
}

function setPins(c) {
    c.preventDefault();
    var b = "/pins";
    var a = "?";
    ["reset", "isp", "conn", "ser", "swap"].forEach(function(d) {
        b += a + d + "=" + $("#pin-" + d).value;
        a = "&"
    });
    b += "&rxpup=" + ($("#pin-rxpup").checked ? "1" : "0");
    ajaxSpin("POST", b, function() {
        showNotification("Pin assignment changed")
    }, function(d, f) {
        showWarning(f);
        window.setTimeout(fetchPins, 100)
    })
}

function changeToWifiPage(a) {
    window.location.href = "wifi.html"
}

function showConfigWiFiMessage() {
    alert("Please connect to an existing wifi to enable this function.")
}
ajaxJson("GET", "/wifi/info", function(a) {
    document.title += " - " + a.hostname
});
