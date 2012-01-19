{{%AUTOESCAPE context="JSON"}}
    "total" : {{TOTAL}},
    "cursors": {    
                    "current"   : "{{CURRENT}}",
                    "first"     : "{{FIRST}}",
                    "last"      : "{{LAST}}",
                    "previous"  : "{{PREVIOUS}}",
                    "next"      : "{{NEXT}}"
    },
    {{>DOCUMENTS}}
