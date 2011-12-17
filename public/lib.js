RegExp.escape = function(str)
{
  var specials = new RegExp("[.*+?|()\\[\\]{}\\\\]", "g"); // .*+?|()[]{}\
  return str.replace(specials, "\\$&");
};

// From http://www.sencha.com/forum/showthread.php?68599-Ultra-Simple-text-highlighting-method
function highlightText(node, regex, cls, deep) {
    if (typeof(regex) == 'string') {
        regex = new RegExp(regex, "g");
    } else if (!regex.global) {
        throw "RegExp to highlight must use the global qualifier";
    }

    var value, df, m, l, start = 0, highlightSpan;
//  Note: You must add the trim function to the String's prototype
    if ((node.nodeType == 3) && (value = node.data.trim())) {

//      Loop through creating a document DocumentFragment containing text nodes interspersed with
//      <span class={cls}> elements wrapping the matched text.
        while (m = regex.exec(value)) {
            if (!df) {
                df = document.createDocumentFragment();
            }
            if (l = m.index - start) {
                df.appendChild(document.createTextNode(value.substr(start, l)));
            }
            highlightSpan = document.createElement('span');
            highlightSpan.className = cls;
            highlightSpan.appendChild(document.createTextNode(m[0]));
            df.appendChild(highlightSpan);
            start = m.index + m[0].length;
        }
        
//      If there is a resulting DocumentFragment, replace the original text node with the fragment
        if (df) {
            if (l = value.length - start) {
                df.appendChild(document.createTextNode(value.substr(start, l)));
            }
            node.parentNode.replaceChild(df, node);
        }
    }else{
        if(deep){
            Ext.each(node.childNodes, function(child){
                highlightText(child, regex, cls, deep);
            }); 
        }
    }
}

function removeHighlighting(highlightClass, node) {
    var h = Ext.DomQuery.select("span." + highlightClass, node);
    for (var i = 0; i < h.length; i++) {
        var s = h[i], sp = s.parentNode;
        sp.replaceChild(document.createTextNode(s.firstChild.data), s);
        sp.normalize();
    }
}